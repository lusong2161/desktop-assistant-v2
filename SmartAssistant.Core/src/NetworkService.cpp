#include "NetworkService.h"
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>
#include <nlohmann/json.hpp>
#include <mutex>
#include <queue>
#include <thread>

namespace SmartAssistant {
namespace Core {

using json = nlohmann::json;

class NetworkService::Impl {
public:
    using Client = websocketpp::client<websocketpp::config::asio_tls_client>;
    using MessagePtr = websocketpp::config::asio_client::message_type::ptr;
    using WebsocketPtr = websocketpp::client<websocketpp::config::asio_client>::connection_ptr;

    Impl() : status_(ConnectionStatus::Disconnected) {
        InitializeWebSocket();
        cryptoService_ = std::make_shared<CryptoService>();
        fileTransferManager_ = std::make_shared<FileTransferManager>();
    }

    ~Impl() {
        Disconnect();
    }

    Result<bool> SendEncryptedMessage(const Message& message, const std::string& publicKey) {
        try {
            auto encryptedContent = cryptoService_->EncryptMessage(message.content, publicKey);
            if (encryptedContent.empty()) {
                return Result<bool>(Error(static_cast<int>(StatusCode::EncryptionError), 
                    "Failed to encrypt message"));
            }

            Message encryptedMsg = message;
            encryptedMsg.content = "";
            encryptedMsg.data = encryptedContent;
            encryptedMsg.type = MessageType::Encrypted;

            return SendMessage(encryptedMsg);
        }
        catch (const std::exception& e) {
            return Result<bool>(Error(static_cast<int>(StatusCode::EncryptionError), e.what()));
        }
    }

    Result<std::string> InitiateFileTransfer(const std::string& receiverId, 
                                           const std::string& filePath) {
        try {
            auto result = fileTransferManager_->InitiateTransfer(filePath, receiverId);
            if (!result.IsSuccess()) {
                return result;
            }

            auto transferId = result.GetValue();
            
            // Notify receiver about incoming transfer
            Message msg;
            msg.id = GenerateUUID();
            msg.type = MessageType::FileTransfer;
            msg.senderId = currentUserId_;
            msg.receiverId = receiverId;
            msg.content = transferId;
            
            auto sendResult = SendMessage(msg);
            if (!sendResult.IsSuccess()) {
                fileTransferManager_->CancelTransfer(transferId);
                return Result<std::string>(sendResult.GetError());
            }

            return Result<std::string>(transferId);
        }
        catch (const std::exception& e) {
            return Result<std::string>(Error(static_cast<int>(StatusCode::FileError), e.what()));
        }
    }

    Result<bool> ResumeFileTransfer(const std::string& transferId) {
        return fileTransferManager_->ResumeTransfer(transferId);
    }

    Result<bool> PauseFileTransfer(const std::string& transferId) {
        return fileTransferManager_->PauseTransfer(transferId);
    }

    Result<bool> CancelFileTransfer(const std::string& transferId) {
        return fileTransferManager_->CancelTransfer(transferId);
    }

    Result<bool> AddFriend(const std::string& userId) {
        try {
            FriendRequest request;
            request.requestId = GenerateUUID();
            request.senderId = currentUserId_;
            request.receiverId = userId;
            request.status = FriendRequestStatus::Pending;
            request.timestamp = GetCurrentTimestamp();

            json j = {
                {"type", "friend_request"},
                {"requestId", request.requestId},
                {"senderId", request.senderId},
                {"receiverId", request.receiverId},
                {"status", static_cast<int>(request.status)},
                {"timestamp", request.timestamp}
            };

            client_.send(connection_, j.dump(), websocketpp::frame::opcode::text);
            pendingFriendRequests_[request.requestId] = request;
            return Result<bool>(true);
        }
        catch (const std::exception& e) {
            return Result<bool>(Error(static_cast<int>(StatusCode::NetworkError), e.what()));
        }
    }

    Result<bool> AcceptFriendRequest(const std::string& requestId) {
        try {
            auto it = pendingFriendRequests_.find(requestId);
            if (it == pendingFriendRequests_.end()) {
                return Result<bool>(Error(static_cast<int>(StatusCode::NetworkError), 
                    "Friend request not found"));
            }


            auto& request = it->second;
            request.status = FriendRequestStatus::Accepted;

            json j = {
                {"type", "friend_request_response"},
                {"requestId", request.requestId},
                {"status", static_cast<int>(request.status)}
            };

            client_.send(connection_, j.dump(), websocketpp::frame::opcode::text);
            friendList_.push_back(request.senderId);
            pendingFriendRequests_.erase(it);
            return Result<bool>(true);
        }
        catch (const std::exception& e) {
            return Result<bool>(Error(static_cast<int>(StatusCode::NetworkError), e.what()));
        }
    }

    Result<bool> RejectFriendRequest(const std::string& requestId) {
        try {
            auto it = pendingFriendRequests_.find(requestId);
            if (it == pendingFriendRequests_.end()) {
                return Result<bool>(Error(static_cast<int>(StatusCode::NetworkError), 
                    "Friend request not found"));
            }

            auto& request = it->second;
            request.status = FriendRequestStatus::Rejected;

            json j = {
                {"type", "friend_request_response"},
                {"requestId", request.requestId},
                {"status", static_cast<int>(request.status)}
            };

            client_.send(connection_, j.dump(), websocketpp::frame::opcode::text);
            pendingFriendRequests_.erase(it);
            return Result<bool>(true);
        }
        catch (const std::exception& e) {
            return Result<bool>(Error(static_cast<int>(StatusCode::NetworkError), e.what()));
        }
    }

    void InitializeWebSocket() {
        client_.clear_access_channels(websocketpp::log::alevel::all);
        client_.set_access_channels(websocketpp::log::alevel::connect);
        client_.set_access_channels(websocketpp::log::alevel::disconnect);
        client_.set_access_channels(websocketpp::log::alevel::app);

        client_.init_asio();

        client_.set_message_handler([this](websocketpp::connection_hdl hdl, MessagePtr msg) {
            OnWebSocketMessage(msg->get_payload());
        });

        client_.set_open_handler([this](websocketpp::connection_hdl hdl) {
            SetStatus(ConnectionStatus::Connected);
        });

        client_.set_close_handler([this](websocketpp::connection_hdl hdl) {
            SetStatus(ConnectionStatus::Disconnected);
        });

        client_.set_fail_handler([this](websocketpp::connection_hdl hdl) {
            SetStatus(ConnectionStatus::Error);
        });
    }

    bool Connect(const std::string& url, const std::string& userId, const std::string& authToken) {
        if (status_ == ConnectionStatus::Connected) {
            return true;
        }

        try {
            websocketpp::lib::error_code ec;
            connection_ = client_.get_connection(url, ec);
            
            if (ec) {
                return false;
            }

            // Add authentication headers
            connection_->append_header("User-Id", userId);
            connection_->append_header("Auth-Token", authToken);

            currentUserId_ = userId;

            client_.connect(connection_);
            
            // Start the ASIO io_service run loop
            if (!clientThread_.joinable()) {
                clientThread_ = std::thread([this]() { client_.run(); });
            }

            // Set initial online status
            if (!ec) {
                UpdateUserStatus(userId, true, "在线");
            }

            SetStatus(ConnectionStatus::Connecting);
            return true;
        }
        catch (const std::exception&) {
            return false;
        }
    }

    void Disconnect() {
        if (connection_) {
            try {
                client_.close(connection_, websocketpp::close::status::normal, "Closing connection");
            }
            catch (const std::exception&) {}
        }

        if (clientThread_.joinable()) {
            client_.stop();
            clientThread_.join();
        }

        SetStatus(ConnectionStatus::Disconnected);
    }

    void SetStatus(ConnectionStatus status) {
        std::lock_guard<std::mutex> lock(mutex_);
        status_ = status;
        NotifyStatusChange();
    }

    void NotifyStatusChange() {
        for (auto& callback : callbacks_) {
            if (auto cb = callback.lock()) {
                cb->OnConnectionStatusChanged(status_);
            }
        }
    }

    Result<bool> UpdateStatus(const std::string& statusMessage) {
        try {
            if (status_ != ConnectionStatus::Connected) {
                return Result<bool>(Error(static_cast<int>(StatusCode::NetworkError), 
                    "未连接到服务器"));
            }

            UserStatus status;
            status.userId = currentUserId_;
            status.isOnline = true;
            status.statusMessage = statusMessage;
            status.lastSeen = GetCurrentTimestamp();

            json j = {
                {"type", "status_update"},
                {"userId", status.userId},
                {"isOnline", status.isOnline},
                {"statusMessage", status.statusMessage},
                {"lastSeen", status.lastSeen}
            };

            client_.send(connection_, j.dump(), websocketpp::frame::opcode::text);

            // Update local status
            userStatusMap_[currentUserId_] = status;

            // Notify callbacks
            for (auto& callback : callbacks_) {
                if (auto cb = callback.lock()) {
                    cb->OnStatusChanged(status);
                }
            }

            return Result<bool>(true);
        }
        catch (const std::exception& e) {
            return Result<bool>(Error(static_cast<int>(StatusCode::NetworkError), 
                std::string("更新状态失败: ") + e.what()));
        }
    }

    Result<std::vector<UserStatus>> GetOnlineUsers() {
        std::vector<UserStatus> onlineUsers;
        for (const auto& [userId, status] : userStatusMap_) {
            if (status.isOnline) {
                onlineUsers.push_back(status);
            }
        }
        return Result<std::vector<UserStatus>>(onlineUsers);
    }

    void UpdateUserStatus(const std::string& userId, bool isOnline, 
                         const std::string& statusMessage = "") {
        UserStatus status;
        status.userId = userId;
        status.isOnline = isOnline;
        status.statusMessage = statusMessage;
        status.lastSeen = GetCurrentTimestamp();

        userStatusMap_[userId] = status;

        // Notify callbacks about status change
        for (auto& callback : callbacks_) {
            if (auto cb = callback.lock()) {
                cb->OnStatusChanged(status);
            }
        }
    }

    void OnWebSocketMessage(const std::string& payload) {
        try {
            auto j = json::parse(payload);
            std::string type = j["type"].get<std::string>();

            if (type == "status_update") {
                // Handle status update message
                std::string userId = j["userId"].get<std::string>();
                bool isOnline = j["isOnline"].get<bool>();
                std::string statusMessage = j["statusMessage"].get<std::string>();
                UpdateUserStatus(userId, isOnline, statusMessage);
            }
            else {
                // Handle regular message
                Message msg;
                msg.id = j["id"].get<std::string>();
                msg.senderId = j["senderId"].get<std::string>();
                msg.receiverId = j["receiverId"].get<std::string>();
                msg.type = static_cast<MessageType>(j["type"].get<int>());
                msg.content = j["content"].get<std::string>();
                msg.timestamp = j["timestamp"].get<int64_t>();

                if (j.contains("data")) {
                    msg.data = j["data"].get<std::vector<uint8_t>>();
                }

                for (auto& callback : callbacks_) {
                    if (auto cb = callback.lock()) {
                        cb->OnMessageReceived(msg);
                    }
                }
            }
        }
        catch (const std::exception&) {
            // Log error
        }
    }

    bool SendMessage(const Message& message) {
        if (status_ != ConnectionStatus::Connected) {
            return false;
        }

        try {
            json j;
            j["id"] = message.id;
            j["senderId"] = message.senderId;
            j["receiverId"] = message.receiverId;
            j["type"] = static_cast<int>(message.type);
            j["content"] = message.content;
            j["timestamp"] = message.timestamp;
            
            if (!message.data.empty()) {
                j["data"] = message.data;
            }

            client_.send(connection_, j.dump(), websocketpp::frame::opcode::text);
            return true;
        }
        catch (const std::exception&) {
            return false;
        }
    }

    void RegisterCallback(std::shared_ptr<INetworkCallback> callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        callbacks_.push_back(callback);
    }

    void UnregisterCallback(std::shared_ptr<INetworkCallback> callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        callbacks_.erase(
            std::remove_if(callbacks_.begin(), callbacks_.end(),
                [&](const std::weak_ptr<INetworkCallback>& weak) {
                    auto ptr = weak.lock();
                    return !ptr || ptr == callback;
                }
            ),
            callbacks_.end()
        );
    }

private:
    Client client_;
    WebsocketPtr connection_;
    std::thread clientThread_;
    ConnectionStatus status_;
    std::mutex mutex_;
    std::vector<std::weak_ptr<INetworkCallback>> callbacks_;
    std::shared_ptr<CryptoService> cryptoService_;
    std::shared_ptr<FileTransferManager> fileTransferManager_;
    std::string currentUserId_;
    std::vector<std::string> friendList_;
    std::map<std::string, FriendRequest> pendingFriendRequests_;
};

// NetworkService implementation
NetworkService::NetworkService() : pImpl(std::make_unique<Impl>()) {}
NetworkService::~NetworkService() = default;

Result<bool> NetworkService::Initialize(const std::string& serverUrl) {
    try {
        if (!pImpl) {
            return Result<bool>(Error(static_cast<int>(StatusCode::UnknownError), 
                "Implementation not initialized"));
        }
        return Result<bool>(true);
    }
    catch (const std::exception& e) {
        return Result<bool>(Error(static_cast<int>(StatusCode::UnknownError), e.what()));
    }
}

Result<bool> NetworkService::Connect(const std::string& userId, const std::string& authToken) {
    try {
        if (!pImpl) {
            return Result<bool>(Error(static_cast<int>(StatusCode::UnknownError), 
                "Implementation not initialized"));
        }
        return Result<bool>(pImpl->Connect(serverUrl_, userId, authToken));
    }
    catch (const std::exception& e) {
        return Result<bool>(Error(static_cast<int>(StatusCode::NetworkError), e.what()));
    }
}

void NetworkService::Disconnect() {
    if (pImpl) {
        pImpl->Disconnect();
    }
}

ConnectionStatus NetworkService::GetConnectionStatus() const {
    if (!pImpl) {
        return ConnectionStatus::Error;
    }
    return pImpl->GetStatus();
}

Result<bool> NetworkService::SendMessage(const Message& message) {
    try {
        if (!pImpl) {
            return Result<bool>(Error(static_cast<int>(StatusCode::UnknownError), 
                "Implementation not initialized"));
        }
        return Result<bool>(pImpl->SendMessage(message));
    }
    catch (const std::exception& e) {
        return Result<bool>(Error(static_cast<int>(StatusCode::NetworkError), e.what()));
    }
}

Result<bool> NetworkService::SendEncryptedMessage(const Message& message, 
                                                const std::string& publicKey) {
    try {
        if (!pImpl) {
            return Result<bool>(Error(static_cast<int>(StatusCode::UnknownError), 
                "Implementation not initialized"));
        }
        return pImpl->SendEncryptedMessage(message, publicKey);
    }
    catch (const std::exception& e) {
        return Result<bool>(Error(static_cast<int>(StatusCode::EncryptionError), e.what()));
    }
}

Result<std::string> NetworkService::InitiateFileTransfer(const std::string& receiverId, 
                                                        const std::string& filePath) {
    try {
        if (!pImpl) {
            return Result<std::string>(Error(static_cast<int>(StatusCode::UnknownError), 
                "Implementation not initialized"));
        }
        return pImpl->InitiateFileTransfer(receiverId, filePath);
    }
    catch (const std::exception& e) {
        return Result<std::string>(Error(static_cast<int>(StatusCode::FileError), e.what()));
    }
}

Result<bool> NetworkService::ResumeFileTransfer(const std::string& transferId) {
    try {
        if (!pImpl) {
            return Result<bool>(Error(static_cast<int>(StatusCode::UnknownError), 
                "Implementation not initialized"));
        }
        return pImpl->ResumeFileTransfer(transferId);
    }
    catch (const std::exception& e) {
        return Result<bool>(Error(static_cast<int>(StatusCode::FileError), e.what()));
    }
}

Result<bool> NetworkService::PauseFileTransfer(const std::string& transferId) {
    try {
        if (!pImpl) {
            return Result<bool>(Error(static_cast<int>(StatusCode::UnknownError), 
                "Implementation not initialized"));
        }
        return pImpl->PauseFileTransfer(transferId);
    }
    catch (const std::exception& e) {
        return Result<bool>(Error(static_cast<int>(StatusCode::FileError), e.what()));
    }
}

Result<bool> NetworkService::CancelFileTransfer(const std::string& transferId) {
    try {
        if (!pImpl) {
            return Result<bool>(Error(static_cast<int>(StatusCode::UnknownError), 
                "Implementation not initialized"));
        }
        return pImpl->CancelFileTransfer(transferId);
    }
    catch (const std::exception& e) {
        return Result<bool>(Error(static_cast<int>(StatusCode::FileError), e.what()));
    }
}

void NetworkService::RegisterCallback(std::shared_ptr<INetworkCallback> callback) {
    if (pImpl) {
        pImpl->RegisterCallback(callback);
    }
}

void NetworkService::UnregisterCallback(std::shared_ptr<INetworkCallback> callback) {
    if (pImpl) {
        pImpl->UnregisterCallback(callback);
    }
}

} // namespace Core
} // namespace SmartAssistant

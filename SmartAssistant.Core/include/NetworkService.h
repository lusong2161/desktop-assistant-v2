#pragma once
#include "Common.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <chrono>

namespace SmartAssistant {
namespace Core {

// Forward declarations
class CryptoService;
class FileTransferManager;

// Friend request status
enum class FriendRequestStatus {
    Pending,
    Accepted,
    Rejected,
    Blocked
};

// Friend request
struct CORE_API FriendRequest {
    std::string requestId;
    std::string senderId;
    std::string receiverId;
    FriendRequestStatus status;
    std::string message;
    int64_t timestamp;
};

enum class ConnectionStatus {
    Disconnected,
    Connecting,
    Connected,
    Error
};

enum class MessageType {
    Text,
    File,
    Status,
    System
};

struct Message {
    std::string id;
    std::string senderId;
    std::string receiverId;
    MessageType type;
    std::string content;
    std::vector<uint8_t> data;
    int64_t timestamp;
};

struct UserStatus {
    std::string userId;
    bool isOnline;
    std::string statusMessage;
    int64_t lastSeen;
};

class INetworkCallback {
public:
    virtual ~INetworkCallback() = default;
    virtual void OnMessageReceived(const Message& message) = 0;
    virtual void OnStatusChanged(const UserStatus& status) = 0;
    virtual void OnConnectionStatusChanged(ConnectionStatus status) = 0;
    virtual void OnTransferProgress(const std::string& transferId, size_t bytesTransferred, size_t totalBytes) = 0;
};

class CORE_API NetworkService {
public:
    NetworkService();
    ~NetworkService();

    // Connection management
    Result<bool> Initialize(const std::string& serverUrl);
    Result<bool> Connect(const std::string& userId, const std::string& authToken);
    void Disconnect();
    ConnectionStatus GetConnectionStatus() const;

    // Message handling
    Result<bool> SendMessage(const Message& message);
    Result<bool> SendEncryptedMessage(const Message& message, const std::string& publicKey);
    
    // File transfer
    Result<std::string> InitiateFileTransfer(const std::string& receiverId, const std::string& filePath);
    Result<bool> ResumeFileTransfer(const std::string& transferId);
    Result<bool> PauseFileTransfer(const std::string& transferId);
    Result<bool> CancelFileTransfer(const std::string& transferId);
    
    // Friend management
    Result<bool> AddFriend(const std::string& userId);
    Result<bool> RemoveFriend(const std::string& userId);
    Result<std::vector<UserStatus>> GetFriendList();
    Result<bool> AcceptFriendRequest(const std::string& userId);
    Result<bool> RejectFriendRequest(const std::string& userId);
    
    // User management
    Result<std::vector<UserStatus>> GetOnlineUsers();
    Result<bool> UpdateStatus(const std::string& statusMessage);
    
    // Callback registration
    void RegisterCallback(std::shared_ptr<INetworkCallback> callback);
    void UnregisterCallback(std::shared_ptr<INetworkCallback> callback);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
    std::shared_ptr<CryptoService> cryptoService_;
    std::shared_ptr<FileTransferManager> fileTransferManager_;
    
    // Internal methods
    bool EncryptAndSendMessage(const Message& message, const std::string& publicKey);
    bool HandleIncomingMessage(const Message& message);
    void UpdateUserStatus(const std::string& userId, bool isOnline);
    
    // Friend management
    std::map<std::string, UserStatus> userStatusMap_;
    std::vector<std::string> friendList_;
    
    // File transfer tracking
    struct TransferInfo {
        std::string transferId;
        std::string filePath;
        size_t totalBytes;
        size_t transferredBytes;
        bool isPaused;
        std::chrono::system_clock::time_point lastUpdate;
    };
    std::map<std::string, TransferInfo> activeTransfers_;
};

} // namespace Core
} // namespace SmartAssistant

#include "FileTransferManager.h"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <fstream>
#include <filesystem>
#include <sqlite3.h>

namespace SmartAssistant {
namespace Core {

namespace {
    namespace asio = boost::asio;
    namespace beast = boost::beast;
    using tcp = asio::ip::tcp;
}

class FileTransferManager::Impl {
public:
    Impl() : ioContext_(), acceptor_(ioContext_) {
        InitializeDatabase();
    }

    void InitializeDatabase() {
        sqlite3* db;
        sqlite3_open("filetransfer.db", &db);
        
        // Create permissions table
        const char* permissionsSql = 
            "CREATE TABLE IF NOT EXISTS permissions ("
            "file_id TEXT NOT NULL,"
            "user_id TEXT NOT NULL,"
            "can_read INTEGER NOT NULL,"
            "can_write INTEGER NOT NULL,"
            "can_share INTEGER NOT NULL,"
            "expiry_time INTEGER,"
            "PRIMARY KEY (file_id, user_id));";
        
        // Create checkpoints table for resume support
        const char* checkpointsSql =
            "CREATE TABLE IF NOT EXISTS checkpoints ("
            "transfer_id TEXT PRIMARY KEY,"
            "position INTEGER NOT NULL,"
            "timestamp INTEGER NOT NULL DEFAULT (strftime('%s', 'now')));";
        
        char* errMsg = nullptr;
        sqlite3_exec(db, permissionsSql, nullptr, nullptr, &errMsg);
        sqlite3_exec(db, checkpointsSql, nullptr, nullptr, &errMsg);
        sqlite3_close(db);
    }

    Result<std::string> InitiateTransfer(const std::string& filePath, const std::string& receiverId) {
        try {
            if (!std::filesystem::exists(filePath)) {
                return Result<std::string>(Error(static_cast<int>(StatusCode::FileError), "File not found"));
            }

            std::string transferId = GenerateUUID();
            TransferInfo info;
            info.filePath = filePath;
            info.totalSize = std::filesystem::file_size(filePath);
            info.receiverId = receiverId;
            info.status = TransferStatus::Pending;
            
            activeTransfers_[transferId] = info;
            return Result<std::string>(transferId);
        }
        catch (const std::exception& e) {
            return Result<std::string>(Error(static_cast<int>(StatusCode::FileError), e.what()));
        }
    }

    Result<bool> PauseTransfer(const std::string& transferId) {
        auto it = activeTransfers_.find(transferId);
        if (it == activeTransfers_.end()) {
            return Result<bool>(Error(static_cast<int>(StatusCode::FileError), "传输未找到"));
        }

        auto& transfer = it->second;
        if (transfer.status == TransferStatus::Transferring) {
            transfer.status = TransferStatus::Paused;
            transfer.pausePosition = transfer.bytesTransferred;
            
            // 保存断点信息到数据库
            SaveCheckpoint(transferId, transfer.pausePosition);
            return Result<bool>(true);
        }
        return Result<bool>(Error(static_cast<int>(StatusCode::InvalidOperation), "传输无法暂停"));
    }

    Result<bool> ResumeTransfer(const std::string& transferId) {
        auto it = activeTransfers_.find(transferId);
        if (it == activeTransfers_.end()) {
            return Result<bool>(Error(static_cast<int>(StatusCode::FileError), "传输未找到"));
        }

        auto& transfer = it->second;
        if (transfer.status == TransferStatus::Paused) {
            // 从数据库加载断点信息
            auto checkpoint = LoadCheckpoint(transferId);
            if (checkpoint > 0) {
                transfer.bytesTransferred = checkpoint;
            }
            
            transfer.status = TransferStatus::Transferring;
            StartTransferFromCheckpoint(transferId, transfer.pausePosition);
            return Result<bool>(true);
        }
        return Result<bool>(Error(static_cast<int>(StatusCode::InvalidOperation), "传输无法恢复"));
    }

    void SaveCheckpoint(const std::string& transferId, uint64_t position) {
        sqlite3* db;
        if (sqlite3_open("filetransfer.db", &db) == SQLITE_OK) {
            const char* sql = 
                "INSERT OR REPLACE INTO checkpoints (transfer_id, position) VALUES (?, ?);";
            
            sqlite3_stmt* stmt;
            if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, transferId.c_str(), -1, SQLITE_STATIC);
                sqlite3_bind_int64(stmt, 2, position);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
            sqlite3_close(db);
        }
    }

    uint64_t LoadCheckpoint(const std::string& transferId) {
        sqlite3* db;
        uint64_t position = 0;
        
        if (sqlite3_open("filetransfer.db", &db) == SQLITE_OK) {
            const char* sql = 
                "SELECT position FROM checkpoints WHERE transfer_id = ?;";
            
            sqlite3_stmt* stmt;
            if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, transferId.c_str(), -1, SQLITE_STATIC);
                
                if (sqlite3_step(stmt) == SQLITE_ROW) {
                    position = sqlite3_column_int64(stmt, 0);
                }
                sqlite3_finalize(stmt);
            }
            sqlite3_close(db);
        }
        return position;
    }

    void StartTransferFromCheckpoint(const std::string& transferId, uint64_t startPosition) {
        auto& transfer = activeTransfers_[transferId];
        if (!transfer.fileStream) {
            transfer.fileStream = std::make_unique<std::fstream>(
                transfer.filePath, 
                std::ios::binary | std::ios::in
            );
        }
        
        transfer.fileStream->seekg(startPosition);
        ContinueTransfer(transferId, transfer.currentSocket, transfer.buffer, *transfer.fileStream);
    }

    Result<bool> InitiateP2PTransfer(const std::string& transferId, const std::string& peerAddress) {
        try {
            auto it = activeTransfers_.find(transferId);
            if (it == activeTransfers_.end()) {
                return Result<bool>(Error(static_cast<int>(StatusCode::FileError), "传输未找到"));
            }


            auto& transfer = it->second;
            transfer.status = TransferStatus::Connecting;

            // Parse peer address
            auto colonPos = peerAddress.find(':');
            if (colonPos == std::string::npos) {
                return Result<bool>(Error(static_cast<int>(StatusCode::NetworkError), "Invalid peer address"));
            }

            std::string host = peerAddress.substr(0, colonPos);
            unsigned short port = std::stoi(peerAddress.substr(colonPos + 1));

            // Start connection
            tcp::endpoint endpoint(asio::ip::make_address(host), port);
            auto socket = std::make_shared<tcp::socket>(ioContext_);
            
            asio::async_connect(*socket, &endpoint, &endpoint + 1,
                [this, transferId, socket](const boost::system::error_code& ec, const tcp::endpoint&) {
                    if (!ec) {
                        StartP2PTransfer(transferId, socket);
                    }
                });

            return Result<bool>(true);
        }
        catch (const std::exception& e) {
            return Result<bool>(Error(static_cast<int>(StatusCode::NetworkError), e.what()));
        }
    }

    void StartP2PTransfer(const std::string& transferId, std::shared_ptr<tcp::socket> socket) {
        auto& transfer = activeTransfers_[transferId];
        transfer.status = TransferStatus::Transferring;

        std::ifstream file(transfer.filePath, std::ios::binary);
        if (!file) {
            NotifyError(transferId, "Failed to open file");
            return;
        }

        auto buffer = std::make_shared<std::vector<char>>(8192);
        
        auto readCallback = [this, transferId, socket, buffer, &file]
            (const boost::system::error_code& ec, std::size_t bytesRead) {
            if (!ec && bytesRead > 0) {
                asio::async_write(*socket, asio::buffer(*buffer, bytesRead),
                    [this, transferId, socket, buffer, &file]
                    (const boost::system::error_code& ec, std::size_t /*bytesSent*/) {
                        if (!ec) {
                            ContinueTransfer(transferId, socket, buffer, file);
                        }
                        else {
                            NotifyError(transferId, "Write error");
                        }
                    });
            }
            else {
                if (ec) {
                    NotifyError(transferId, "Read error");
                }
                else {
                    CompleteTransfer(transferId);
                }
            }
        };

        ContinueTransfer(transferId, socket, buffer, file);
    }

    void ContinueTransfer(const std::string& transferId, 
                         std::shared_ptr<tcp::socket> socket,
                         std::shared_ptr<std::vector<char>> buffer,
                         std::ifstream& file) {
        file.read(buffer->data(), buffer->size());
        std::streamsize bytesRead = file.gcount();
        
        if (bytesRead > 0) {
            auto& transfer = activeTransfers_[transferId];
            transfer.bytesTransferred += bytesRead;
            
            if (transfer.progressCallback) {
                transfer.progressCallback(transfer.bytesTransferred, transfer.totalSize);
            }

            asio::async_write(*socket, asio::buffer(*buffer, static_cast<size_t>(bytesRead)),
                [this, transferId, socket, buffer, &file]
                (const boost::system::error_code& ec, std::size_t /*bytesSent*/) {
                    if (!ec) {
                        ContinueTransfer(transferId, socket, buffer, file);
                    }
                    else {
                        NotifyError(transferId, "Write error");
                    }
                });
        }
        else {
            CompleteTransfer(transferId);
        }
    }

    Result<bool> CancelTransfer(const std::string& transferId) {
        try {
            auto it = activeTransfers_.find(transferId);
            if (it == activeTransfers_.end()) {
                return Result<bool>(Error(static_cast<int>(StatusCode::FileError), "传输未找到"));
            }

            auto& transfer = it->second;
            
            // 通知对端取消传输
            if (transfer.currentSocket && transfer.currentSocket->is_open()) {
                try {
                    // 发送取消信号
                    std::string cancelMsg = "CANCEL:" + transferId;
                    asio::async_write(*transfer.currentSocket, 
                        asio::buffer(cancelMsg),
                        [](const boost::system::error_code&, std::size_t) {});
                    
                    // 关闭套接字
                    transfer.currentSocket->shutdown(tcp::socket::shutdown_both);
                    transfer.currentSocket->close();
                }
                catch (const std::exception&) {
                    // 忽略关闭时的错误
                }
            }

            // 清理资源
            transfer.status = TransferStatus::Cancelled;
            transfer.fileStream.reset();
            transfer.currentSocket.reset();
            transfer.buffer.reset();

            // 从数据库中删除断点信息
            sqlite3* db;
            if (sqlite3_open("filetransfer.db", &db) == SQLITE_OK) {
                const char* sql = "DELETE FROM checkpoints WHERE transfer_id = ?;";
                sqlite3_stmt* stmt;
                if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                    sqlite3_bind_text(stmt, 1, transferId.c_str(), -1, SQLITE_STATIC);
                    sqlite3_step(stmt);
                    sqlite3_finalize(stmt);
                }
                sqlite3_close(db);
            }

            // 通知进度回调
            if (transfer.progressCallback) {
                transfer.progressCallback(transfer.bytesTransferred, transfer.totalSize);
            }

            // 从活动传输列表中移除
            activeTransfers_.erase(it);
            return Result<bool>(true);
        }
        catch (const std::exception& ex) {
            return Result<bool>(Error(static_cast<int>(StatusCode::FileError), 
                std::string("取消传输失败: ") + ex.what()));
        }
    }

    void CompleteTransfer(const std::string& transferId) {
        auto& transfer = activeTransfers_[transferId];
        transfer.status = TransferStatus::Completed;
        
        if (transfer.progressCallback) {
            transfer.progressCallback(transfer.totalSize, transfer.totalSize);
        }
    }

    void NotifyError(const std::string& transferId, const std::string& error) {
        auto& transfer = activeTransfers_[transferId];
        transfer.status = TransferStatus::Error;
        transfer.errorMessage = error;
    }

    Result<bool> SetPermissions(const std::string& fileId, 
                              const std::vector<TransferPermission>& permissions) {
        sqlite3* db;
        if (sqlite3_open("filetransfer.db", &db) != SQLITE_OK) {
            return Result<bool>(Error(static_cast<int>(StatusCode::FileError), 
                                    "Failed to open database"));
        }

        const char* sql = 
            "INSERT OR REPLACE INTO permissions "
            "(file_id, user_id, can_read, can_write, can_share, expiry_time) "
            "VALUES (?, ?, ?, ?, ?, ?);";

        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

        for (const auto& perm : permissions) {
            sqlite3_bind_text(stmt, 1, fileId.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, perm.userId.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_int(stmt, 3, perm.canRead ? 1 : 0);
            sqlite3_bind_int(stmt, 4, perm.canWrite ? 1 : 0);
            sqlite3_bind_int(stmt, 5, perm.canShare ? 1 : 0);
            sqlite3_bind_int64(stmt, 6, 
                std::chrono::system_clock::to_time_t(perm.expiryTime));

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                return Result<bool>(Error(static_cast<int>(StatusCode::FileError), 
                                        "Failed to set permissions"));
            }
            sqlite3_reset(stmt);
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return Result<bool>(true);
    }

private:
    enum class TransferStatus {
        Pending,
        Connecting,
        Transferring,
        Paused,
        Completed,
        Cancelled,
        Error
    };

    struct TransferInfo {
        std::string filePath;
        std::string receiverId;
        std::uintmax_t totalSize;
        std::uintmax_t bytesTransferred = 0;
        std::uintmax_t pausePosition = 0;
        TransferStatus status = TransferStatus::Pending;
        std::string errorMessage;
        ProgressCallback progressCallback;
        std::shared_ptr<tcp::socket> currentSocket;
        std::shared_ptr<std::vector<char>> buffer;
        std::unique_ptr<std::fstream> fileStream;
    };

    asio::io_context ioContext_;
    tcp::acceptor acceptor_;
    std::map<std::string, TransferInfo> activeTransfers_;
};

// FileTransferManager implementation
FileTransferManager::FileTransferManager() : pImpl(std::make_unique<Impl>()) {}
FileTransferManager::~FileTransferManager() {
    pImpl->CleanupAllTransfers();
}

Result<std::string> FileTransferManager::InitiateTransfer(const std::string& filePath, 
                                                        const std::string& receiverId) {
    return pImpl->InitiateTransfer(filePath, receiverId);
}

Result<bool> FileTransferManager::InitiateP2PTransfer(const std::string& transferId, 
                                                     const std::string& peerAddress) {
    return pImpl->InitiateP2PTransfer(transferId, peerAddress);
}

Result<bool> FileTransferManager::SetPermissions(const std::string& fileId,
                                               const std::vector<TransferPermission>& permissions) {
    return pImpl->SetPermissions(fileId, permissions);
}

} // namespace Core
Result<bool> FileTransferManager::PauseTransfer(const std::string& transferId) {
    return pImpl->PauseTransfer(transferId);
}

Result<bool> FileTransferManager::ResumeTransfer(const std::string& transferId) {
    return pImpl->ResumeTransfer(transferId);
}

Result<bool> FileTransferManager::CancelTransfer(const std::string& transferId) {
    return pImpl->CancelTransfer(transferId);
}

Result<bool> FileTransferManager::AcceptTransfer(const std::string& transferId, const std::string& savePath) {
    return pImpl->AcceptTransfer(transferId, savePath);
}

double FileTransferManager::GetTransferProgress(const std::string& transferId) const {
    auto progress = pImpl->GetTransferProgress(transferId);
    return progress.getProgressPercentage();
}

TransferStatus FileTransferManager::GetTransferStatus(const std::string& transferId) const {
    return pImpl->GetTransferStatus(transferId);
}

void FileTransferManager::RegisterProgressCallback(
    const std::string& transferId,
    std::function<void(uint64_t, uint64_t)> callback) {
    auto it = pImpl->activeTransfers_.find(transferId);
    if (it != pImpl->activeTransfers_.end()) {
        it->second.progressCallback = callback;
    }
}

void FileTransferManager::CleanupTransfer(const std::string& transferId) {
    auto it = pImpl->activeTransfers_.find(transferId);
    if (it != pImpl->activeTransfers_.end()) {
        it->second.fileStream.reset();
        it->second.currentSocket.reset();
        it->second.buffer.reset();
        pImpl->activeTransfers_.erase(it);
    }
}

void FileTransferManager::CleanupAllTransfers() {
    for (auto& transfer : pImpl->activeTransfers_) {
        CleanupTransfer(transfer.first);
    }
    pImpl->activeTransfers_.clear();
}

} // namespace Core
} // namespace SmartAssistant

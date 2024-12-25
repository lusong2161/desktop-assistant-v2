#pragma once
#include "Common.h"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>

namespace SmartAssistant {
namespace Core {

struct CORE_API TransferPermission {
    std::string userId;
    bool canRead;
    bool canWrite;
    bool canShare;
    std::chrono::system_clock::time_point expiryTime;
};

class CORE_API FileTransferManager {
public:
    FileTransferManager();
    ~FileTransferManager();

    // File transfer operations
    Result<std::string> InitiateTransfer(const std::string& filePath, const std::string& receiverId);
    Result<bool> PauseTransfer(const std::string& transferId);
    Result<bool> ResumeTransfer(const std::string& transferId);
    Result<bool> CancelTransfer(const std::string& transferId);
    Result<bool> AcceptTransfer(const std::string& transferId, const std::string& savePath);
    
    // Progress tracking
    double GetTransferProgress(const std::string& transferId) const;
    TransferStatus GetTransferStatus(const std::string& transferId) const;
    void RegisterProgressCallback(const std::string& transferId, 
                                std::function<void(uint64_t, uint64_t)> callback);
    
    // Cleanup
    void CleanupTransfer(const std::string& transferId);
    void CleanupAllTransfers();
    
    // Permission management
    Result<bool> SetPermissions(const std::string& fileId, const std::vector<TransferPermission>& permissions);
    Result<std::vector<TransferPermission>> GetPermissions(const std::string& fileId);
    Result<bool> HasPermission(const std::string& userId, const std::string& fileId, const std::string& permission);
    
    // Progress tracking
    void RegisterProgressCallback(const std::string& transferId, ProgressCallback callback);
    double GetTransferProgress(const std::string& transferId) const;
    
    // P2P transfer
    Result<bool> InitiateP2PTransfer(const std::string& transferId, const std::string& peerAddress);
    Result<bool> AcceptP2PTransfer(const std::string& transferId, const std::string& peerAddress);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace Core
} // namespace SmartAssistant

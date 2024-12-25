#include "FileTransferManager.h"

namespace SmartAssistant {
    FileTransferManager::FileTransferManager() {}
    FileTransferManager::~FileTransferManager() {}
    
    bool FileTransferManager::InitiateTransfer(const std::string& filePath, const std::string& userId) {
        activeTransfers_[filePath] = true;
        return true;
    }
    
    bool FileTransferManager::PauseTransfer(const std::string& transferId) {
        if (activeTransfers_.find(transferId) == activeTransfers_.end()) return false;
        return true;
    }
    
    
    bool FileTransferManager::ResumeTransfer(const std::string& transferId) {
        if (activeTransfers_.find(transferId) == activeTransfers_.end()) return false;
        return true;
    }
}

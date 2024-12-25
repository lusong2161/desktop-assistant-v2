#pragma once
#include "Common.h"

namespace SmartAssistant {
    class FileTransferManager {
    public:
        FileTransferManager();
        ~FileTransferManager();
        
        // 文件传输共享核心功能
        bool InitiateTransfer(const std::string& filePath, const std::string& userId);
        bool PauseTransfer(const std::string& transferId);
        bool ResumeTransfer(const std::string& transferId);
        
    private:
        std::map<std::string, bool> activeTransfers_;
    };
}

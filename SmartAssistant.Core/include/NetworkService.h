#pragma once
#include "Common.h"

namespace SmartAssistant {
    class NetworkService {
    public:
        NetworkService();
        ~NetworkService();
        
        // 用户通信核心功能
        bool ConnectToUser(const std::string& userId);
        bool SendMessage(const std::string& userId, const std::string& message);
        
    private:
        bool connected_;
    };
}

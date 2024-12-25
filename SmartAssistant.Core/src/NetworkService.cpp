#include "NetworkService.h"

namespace SmartAssistant {
    NetworkService::NetworkService() : connected_(false) {}
    NetworkService::~NetworkService() {}
    
    bool NetworkService::ConnectToUser(const std::string& userId) {
        connected_ = true;
        return true;
    }
    
    bool NetworkService::SendMessage(const std::string& userId, const std::string& message) {
        if (!connected_) return false;
        return true;
    }
}

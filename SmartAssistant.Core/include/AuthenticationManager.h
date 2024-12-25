#pragma once

#include <string>
#include <memory>
#include "Common.h"
#include "CryptoService.h"

namespace SmartAssistant {
namespace Core {

class AuthenticationManager {
public:
    static AuthenticationManager& getInstance();
    
    bool initialize(const std::string& jwtSecret);
    bool login(const std::string& username, const std::string& password);
    void logout();
    
    bool isAuthenticated() const { return !m_currentToken.empty(); }
    const std::string& getCurrentUser() const { return m_currentUser; }
    const std::string& getAuthToken() const { return m_currentToken; }
    
protected:
    bool validateToken(const std::string& token);
    std::string encryptCredentials(const std::string& username, const std::string& password);
    
private:
    AuthenticationManager() = default;
    ~AuthenticationManager() = default;
    AuthenticationManager(const AuthenticationManager&) = delete;
    AuthenticationManager& operator=(const AuthenticationManager&) = delete;
    
    std::string m_jwtSecret;
    std::string m_currentToken;
    std::string m_currentUser;
    std::unique_ptr<CryptoService> m_cryptoService;
};

} // namespace Core
} // namespace SmartAssistant

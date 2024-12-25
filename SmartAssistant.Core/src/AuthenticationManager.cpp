#include "AuthenticationManager.h"
#include <jwt-cpp/jwt.h>
#include <ctime>
#include <iostream>

namespace SmartAssistant {
namespace Core {

AuthenticationManager& AuthenticationManager::getInstance()
{
    static AuthenticationManager instance;
    return instance;
}

bool AuthenticationManager::initialize(const std::string& jwtSecret)
{
    m_jwtSecret = jwtSecret;
    m_cryptoService = std::make_unique<CryptoService>();
    return true;
}

bool AuthenticationManager::login(const std::string& username, const std::string& password)
{
    try {
        // 加密凭据
        std::string encryptedCreds = encryptCredentials(username, password);
        
        
        // 通过API客户端发送认证请求
        auto& apiClient = ApiClient::getInstance();
        std::string response;
        if (apiClient.sendRequest("/auth/login", "POST", encryptedCreds, response)) {
            if (!response.empty() && validateToken(response)) {
                m_currentToken = response;
                m_currentUser = username;
                return true;
            }
        }
        return false;
    }
    catch (const std::exception& e) {
        std::cerr << "登录失败: " << e.what() << std::endl;
        return false;
    }
}

void AuthenticationManager::logout()
{
    m_currentToken.clear();
    m_currentUser.clear();
}

bool AuthenticationManager::validateToken(const std::string& token)
{
    try {
        auto decoded = jwt::decode(token);
        
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{m_jwtSecret})
            .with_issuer("smart_assistant");
            
        verifier.verify(decoded);
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Token验证失败: " << e.what() << std::endl;
        return false;
    }
}

std::string AuthenticationManager::encryptCredentials(
    const std::string& username, const std::string& password)
{
    // 使用CryptoService加密凭据
    std::string credentials = username + ":" + password;
    return m_cryptoService->encrypt(credentials);
}

} // namespace Core
} // namespace SmartAssistant

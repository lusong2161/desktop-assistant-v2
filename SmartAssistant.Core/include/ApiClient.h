#pragma once

#include <string>
#include <memory>
#include <functional>
#include "Common.h"

namespace SmartAssistant {
namespace Core {

class ApiClient {
public:
    static ApiClient& getInstance();
    
    // 初始化API客户端
    bool initialize(const std::string& baseUrl);
    
    // 文件传输相关
    bool uploadFile(const std::string& filePath, const std::string& recipientId, 
                   std::function<void(int)> progressCallback);
    bool downloadFile(const std::string& fileId, const std::string& savePath,
                     std::function<void(int)> progressCallback);
    
    // 系统控制相关
    bool executeCommand(const std::string& command, std::string& response);
    
    // 文档协作相关
    bool syncDocument(const std::string& documentId, const std::string& content);
    bool getDocumentHistory(const std::string& documentId, std::vector<std::string>& history);

private:
    ApiClient() = default;
    ~ApiClient() = default;
    ApiClient(const ApiClient&) = delete;
    ApiClient& operator=(const ApiClient&) = delete;

    std::string m_baseUrl;
    std::string m_authToken;
    
    // 内部辅助方法
    bool sendRequest(const std::string& endpoint, const std::string& method,
                    const std::string& data, std::string& response);
    bool handleResponse(const std::string& response);
};

} // namespace Core
} // namespace SmartAssistant

#include "ApiClient.h"
#include <curl/curl.h>
#include <sstream>
#include <iostream>

namespace SmartAssistant {
namespace Core {

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

ApiClient& ApiClient::getInstance()
{
    static ApiClient instance;
    return instance;
}

bool ApiClient::initialize(const std::string& baseUrl)
{
    m_baseUrl = baseUrl;
    curl_global_init(CURL_GLOBAL_DEFAULT);
    return true;
}

bool ApiClient::uploadFile(const std::string& filePath, const std::string& recipientId,
                         std::function<void(int)> progressCallback)
{
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "CURL初始化失败" << std::endl;
        return false;
    }

    struct curl_httppost* formpost = nullptr;
    struct curl_httppost* lastptr = nullptr;

    // 添加文件
    curl_formadd(&formpost, &lastptr,
                 CURLFORM_COPYNAME, "file",
                 CURLFORM_FILE, filePath.c_str(),
                 CURLFORM_END);

    // 添加接收者ID
    curl_formadd(&formpost, &lastptr,
                 CURLFORM_COPYNAME, "recipientId",
                 CURLFORM_COPYCONTENTS, recipientId.c_str(),
                 CURLFORM_END);

    std::string url = m_baseUrl + "/files/transfer";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

    // 设置进度回调
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, 
        [](void* clientp, double dltotal, double dlnow, double ultotal, double ulnow) -> int {
            if (ultotal > 0) {
                auto callback = reinterpret_cast<std::function<void(int)>*>(clientp);
                (*callback)(static_cast<int>((ulnow / ultotal) * 100));
            }
            return 0;
        });
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &progressCallback);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);

    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    
    curl_formfree(formpost);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}

bool ApiClient::executeCommand(const std::string& command, std::string& response)
{
    return sendRequest("/command", "POST", command, response);
}

bool ApiClient::syncDocument(const std::string& documentId, const std::string& content)
{
    std::string response;
    std::string data = "{\"documentId\":\"" + documentId + "\",\"content\":\"" + content + "\"}";
    return sendRequest("/documents/sync", "POST", data, response);
}

bool ApiClient::getDocumentHistory(const std::string& documentId, std::vector<std::string>& history)
{
    std::string response;
    if (sendRequest("/documents/" + documentId + "/history", "GET", "", response)) {
        // TODO: Parse JSON response into history vector
        return true;
    }
    return false;
}

bool ApiClient::sendRequest(const std::string& endpoint, const std::string& method,
                          const std::string& data, std::string& response)
{
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }

    std::string url = m_baseUrl + endpoint;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!m_authToken.empty()) {
        headers = curl_slist_append(headers, ("Authorization: Bearer " + m_authToken).c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    }

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}

bool ApiClient::handleResponse(const std::string& response)
{
    // TODO: Implement response handling
    return true;
}

} // namespace Core
} // namespace SmartAssistant

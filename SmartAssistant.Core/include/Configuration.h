#pragma once

#include <string>
#include <memory>
#include "Common.h"

namespace SmartAssistant {
namespace Core {

class Configuration {
public:
    static Configuration& getInstance();
    
    bool initialize(const std::string& configPath = "config.json");
    bool save();
    
    std::string getApiBaseUrl() const { return m_apiBaseUrl; }
    std::string getWebSocketUrl() const { return m_wsUrl; }
    
    void setApiBaseUrl(const std::string& url) { m_apiBaseUrl = url; }
    void setWebSocketUrl(const std::string& url) { m_wsUrl = url; }

private:
    Configuration() = default;
    ~Configuration() = default;
    Configuration(const Configuration&) = delete;
    Configuration& operator=(const Configuration&) = delete;

    std::string m_configPath;
    std::string m_apiBaseUrl = "https://localhost/api";
    std::string m_wsUrl = "wss://localhost/ws";
    
    bool loadFromFile();
    bool saveToFile();
};

} // namespace Core
} // namespace SmartAssistant

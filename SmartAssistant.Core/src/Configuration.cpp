#include "Configuration.h"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace SmartAssistant {
namespace Core {

using json = nlohmann::json;

Configuration& Configuration::getInstance()
{
    static Configuration instance;
    return instance;
}

bool Configuration::initialize(const std::string& configPath)
{
    m_configPath = configPath;
    return loadFromFile();
}

bool Configuration::loadFromFile()
{
    try {
        std::ifstream file(m_configPath);
        if (file.is_open()) {
            json j;
            file >> j;
            
            if (j.contains("apiBaseUrl")) {
                m_apiBaseUrl = j["apiBaseUrl"].get<std::string>();
            }
            if (j.contains("webSocketUrl")) {
                m_wsUrl = j["webSocketUrl"].get<std::string>();
            }
            
            return true;
        }
        else {
            // 如果配置文件不存在，创建默认配置
            return save();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "配置加载失败: " << e.what() << std::endl;
        return false;
    }
}

bool Configuration::save()
{
    try {
        json j;
        j["apiBaseUrl"] = m_apiBaseUrl;
        j["webSocketUrl"] = m_wsUrl;
        
        std::ofstream file(m_configPath);
        if (file.is_open()) {
            file << j.dump(4);
            return true;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "配置保存失败: " << e.what() << std::endl;
    }
    return false;
}

} // namespace Core
} // namespace SmartAssistant

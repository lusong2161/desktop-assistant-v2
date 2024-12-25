#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>

namespace SmartAssistant {
    // 通用错误类型
    class SmartAssistantException : public std::runtime_error {
    public:
        explicit SmartAssistantException(const std::string& message)
            : std::runtime_error(message) {}
    };
}

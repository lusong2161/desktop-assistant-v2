#pragma once

#ifdef SMARTASSISTANT_EXPORTS
#define SMARTASSISTANT_API __declspec(dllexport)
#else
#define SMARTASSISTANT_API __declspec(dllimport)
#endif

#include <string>
#include <Windows.h>
#include <memory>
#include <vector>
#include <chrono>

namespace SmartAssistant {
namespace Core {

// Common error handling
struct SMARTASSISTANT_API Error {
    int code;
    std::wstring message;
};

// Common result template
template<typename T>
struct SMARTASSISTANT_API Result {
    bool success;
    T value;
    Error error;
};

// Common initialization options
struct SMARTASSISTANT_API InitOptions {
    std::wstring configPath;
    std::wstring logPath;
    bool debug;
};

} // namespace Core
} // namespace SmartAssistant

#pragma once
#include "Common.h"

#include <string>

namespace SmartAssistant {
namespace Core {

class SMARTASSISTANT_API CoreAPI {
public:
    // Initialization
    static bool Initialize();
    static void Shutdown();
    
    // AI Integration
    static bool InitializeAI(const std::wstring& apiKey);
    static bool ProcessCommand(const std::wstring& command, std::wstring& response);
    static bool ProcessVoiceCommand(const std::wstring& audioPath, std::wstring& response);
    
    // System Integration
    static bool SetWindowsStartup(bool enable);
    static bool RegisterGlobalHotkey(int id, int modifiers, int key);
    static bool UnregisterGlobalHotkey(int id);
    
    // Notification System
    static bool ShowNotification(const std::wstring& title, const std::wstring& message);
    static bool ScheduleReminder(const std::wstring& title, const std::wstring& message, 
                               long long timestamp);
    
    // Weather Integration
    static bool GetWeatherInfo(const std::wstring& location, std::wstring& weatherInfo);
};

} // namespace Core
} // namespace SmartAssistant

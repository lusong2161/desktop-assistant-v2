#pragma once
#include "Common.h"

#include <string>
#include <chrono>
#include <vector>

namespace SmartAssistant {
namespace Core {

enum class SMARTASSISTANT_API NotificationType {
    Information,
    Warning,
    Error,
    Reminder
};

struct SMARTASSISTANT_API Reminder {
    std::wstring title;
    std::wstring message;
    std::chrono::system_clock::time_point timestamp;
    bool recurring;
    int recurringIntervalMinutes;
};

class SMARTASSISTANT_API NotificationService {
public:
    static bool Initialize();
    static void Shutdown();
    
    // Notifications
    static bool ShowNotification(const std::wstring& title, 
                               const std::wstring& message,
                               NotificationType type = NotificationType::Information);
    
    // Reminders
    static bool AddReminder(const Reminder& reminder);
    static bool RemoveReminder(const std::wstring& title);
    static bool GetReminders(std::vector<Reminder>& reminders);
    
    // Weather
    static bool UpdateWeather(const std::wstring& location);
    static bool GetWeatherInfo(std::wstring& weatherInfo);
};

} // namespace Core
} // namespace SmartAssistant

#include "../include/CoreAPI.h"
#include <windows.h>

namespace SmartAssistant {
namespace Core {

bool CoreAPI::Initialize() {
    // Initialize COM for the current thread
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    return SUCCEEDED(hr);
}

void CoreAPI::Shutdown() {
    CoUninitialize();
}

bool CoreAPI::InitializeAI(const std::wstring& apiKey) {
    // TODO: Initialize OpenAI API client
    return true;
}

bool CoreAPI::ProcessCommand(const std::wstring& command, std::wstring& response) {
    // TODO: Implement command processing using OpenAI API
    return true;
}

bool CoreAPI::ProcessVoiceCommand(const std::wstring& audioPath, std::wstring& response) {
    // TODO: Implement voice command processing
    return true;
}

bool CoreAPI::SetWindowsStartup(bool enable) {
    HKEY hKey;
    const wchar_t* keyPath = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run";
    
    if (RegOpenKeyExW(HKEY_CURRENT_USER, keyPath, 0, KEY_SET_VALUE, &hKey) != ERROR_SUCCESS) {
        return false;
    }
    
    if (enable) {
        wchar_t exePath[MAX_PATH];
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        RegSetValueExW(hKey, L"SmartAssistant", 0, REG_SZ, 
                      (BYTE*)exePath, (wcslen(exePath) + 1) * sizeof(wchar_t));
    } else {
        RegDeleteValueW(hKey, L"SmartAssistant");
    }
    
    RegCloseKey(hKey);
    return true;
}

bool CoreAPI::RegisterGlobalHotkey(int id, int modifiers, int key) {
    return RegisterHotKey(nullptr, id, modifiers, key) != 0;
}

bool CoreAPI::UnregisterGlobalHotkey(int id) {
    return UnregisterHotKey(nullptr, id) != 0;
}

bool CoreAPI::ShowNotification(const std::wstring& title, const std::wstring& message) {
    // TODO: Implement Windows notification
    return true;
}

bool CoreAPI::ScheduleReminder(const std::wstring& title, const std::wstring& message, 
                             long long timestamp) {
    // TODO: Implement reminder scheduling
    return true;
}


bool CoreAPI::GetWeatherInfo(const std::wstring& location, std::wstring& weatherInfo) {
    // TODO: Implement weather API integration
    return true;
}

} // namespace Core
} // namespace SmartAssistant

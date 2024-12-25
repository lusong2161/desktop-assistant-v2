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

namespace SmartAssistant {
namespace Core {

struct SMARTASSISTANT_API SystemMetrics {
    double cpuUsage;
    size_t totalMemory;
    size_t availableMemory;
    size_t totalDiskSpace;
    size_t freeDiskSpace;
};

class SMARTASSISTANT_API SystemControl {
public:
    static bool Initialize();
    static void Shutdown();

    // Window Management
    static bool SetWindowTopMost(HWND hwnd, bool enable);
    static bool SetWindowTransparency(HWND hwnd, BYTE alpha);
    static bool SetWindowPosition(HWND hwnd, int x, int y);
    
    // System Operations
    static bool SetSystemVolume(float volume); // 0.0f to 1.0f
    static float GetSystemVolume();
    static bool SetScreenBrightness(int brightness); // 0 to 100
    static int GetScreenBrightness();
    
    // Process Management
    static bool StartProcess(const std::wstring& path);
    static bool StopProcess(const std::wstring& processName);
    static bool IsProcessRunning(const std::wstring& processName);
    
    // System Information
    static bool GetCPUUsage(double& usage);
    static bool GetMemoryStatus(size_t& totalPhys, size_t& availPhys);
    static bool GetDiskSpace(const std::wstring& path, size_t& totalSpace, size_t& freeSpace);

    // Hotkey Management
    static bool RegisterGlobalHotkey(int id, int modifiers, int key);
    static bool UnregisterGlobalHotkey(int id);
};

} // namespace Core
} // namespace SmartAssistant

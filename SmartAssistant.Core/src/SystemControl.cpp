#include "../include/SystemControl.h"
#include <psapi.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <powrprof.h>
#include <highlevelmonitorconfigurationapi.h>
#include <physicalmonitorenumerationapi.h>
#include <tlhelp32.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "psapi.lib")

namespace SmartAssistant {
namespace Core {

bool SystemControl::Initialize() {
    return true;
}


void SystemControl::Shutdown() {
}

bool SystemControl::SetWindowTopMost(HWND hwnd, bool enable) {
    return SetWindowPos(hwnd, enable ? HWND_TOPMOST : HWND_NOTOPMOST,
                       0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
}

bool SystemControl::SetWindowTransparency(HWND hwnd, BYTE alpha) {
    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    if (!(exStyle & WS_EX_LAYERED)) {
        SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED);
    }
    return SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
}

bool SystemControl::SetWindowPosition(HWND hwnd, int x, int y) {
    return SetWindowPos(hwnd, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

bool SystemControl::SetSystemVolume(float volume) {
    // TODO: Implement using IMMDeviceEnumerator and IAudioEndpointVolume
    return true;
}

float SystemControl::GetSystemVolume() {
    // TODO: Implement using IMMDeviceEnumerator and IAudioEndpointVolume
    return 0.0f;
}

bool SystemControl::SetScreenBrightness(int brightness) {
    // TODO: Implement using SetMonitorBrightness
    return true;
}

int SystemControl::GetScreenBrightness() {
    // TODO: Implement using GetMonitorBrightness
    return 0;
}

bool SystemControl::StartProcess(const std::wstring& path) {
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    
    return CreateProcessW(nullptr, const_cast<LPWSTR>(path.c_str()),
                         nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
}

bool SystemControl::StopProcess(const std::wstring& processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(pe32);

    if (!Process32FirstW(snapshot, &pe32)) {
        CloseHandle(snapshot);
        return false;
    }

    bool result = false;
    do {
        if (processName == pe32.szExeFile) {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
            if (hProcess != nullptr) {
                result = TerminateProcess(hProcess, 0);
                CloseHandle(hProcess);
            }
            break;
        }
    } while (Process32NextW(snapshot, &pe32));

    CloseHandle(snapshot);
    return result;
}

bool SystemControl::IsProcessRunning(const std::wstring& processName) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(pe32);

    if (!Process32FirstW(snapshot, &pe32)) {
        CloseHandle(snapshot);
        return false;
    }

    bool found = false;
    do {
        if (processName == pe32.szExeFile) {
            found = true;
            break;
        }
    } while (Process32NextW(snapshot, &pe32));

    CloseHandle(snapshot);
    return found;
}

bool SystemControl::GetCPUUsage(double& usage) {
    // TODO: Implement using PDH or GetSystemTimes
    return true;
}

bool SystemControl::GetMemoryStatus(size_t& totalPhys, size_t& availPhys) {
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    
    if (GlobalMemoryStatusEx(&memInfo)) {
        totalPhys = static_cast<size_t>(memInfo.ullTotalPhys);
        availPhys = static_cast<size_t>(memInfo.ullAvailPhys);
        return true;
    }
    return false;
}

bool SystemControl::GetDiskSpace(const std::wstring& path, size_t& totalSpace, size_t& freeSpace) {
    ULARGE_INTEGER free, total;
    if (GetDiskFreeSpaceExW(path.c_str(), nullptr, &total, &free)) {
        totalSpace = static_cast<size_t>(total.QuadPart);
        freeSpace = static_cast<size_t>(free.QuadPart);
        return true;
    }
    return false;
}

bool SystemControl::RegisterGlobalHotkey(int id, int modifiers, int key) {
    return RegisterHotKey(nullptr, id, modifiers, key) != 0;
}

bool SystemControl::UnregisterGlobalHotkey(int id) {
    return UnregisterHotKey(nullptr, id) != 0;
}

} // namespace Core
} // namespace SmartAssistant

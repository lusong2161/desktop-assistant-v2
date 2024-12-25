# Windows Development Environment Setup Guide

## Required Software
1. Visual Studio 2022 Community Edition or Professional/Enterprise
   - Download from: https://visualstudio.microsoft.com/downloads/

## Required Workloads
During Visual Studio installation, select the following workloads:
1. ".NET Desktop Development"
   - Includes WPF development tools
   - .NET SDK
   - .NET Runtime
2. "Desktop Development with C++"
   - Windows SDK
   - C++ build tools
   - Windows 11 SDK
   - C++ CMake tools

## Individual Components
Ensure these individual components are selected:
1. Windows SDK (latest version)
2. C++ ATL for latest build tools
3. C++ MFC for latest build tools
4. C++/CLI support
5. .NET Framework 4.8 SDK
6. .NET Framework 4.8 targeting pack

## Post-Installation Steps
1. Start Visual Studio 2022
2. Sign in with Microsoft Account (if needed)
3. Open SmartAssistant.sln
4. Restore NuGet packages
5. Build solution to verify setup

## Project Configuration
The solution contains two projects:
1. SmartAssistant.UI (C# WPF Project)
   - Target Framework: .NET 6.0
   - Output Type: Windows Application
2. SmartAssistant.Core (C++ Project)
   - Platform Toolset: Visual Studio 2022 (v143)
   - Windows SDK Version: Latest
   - Character Set: Unicode

## Development Tools Verification
1. Open Visual Studio Developer Command Prompt
2. Run the following commands to verify installation:
```cmd
msbuild -version
cl.exe
dotnet --info
```

## Troubleshooting
If you encounter any issues:
1. Verify all required workloads are installed
2. Check Windows SDK installation
3. Ensure Visual Studio is up to date
4. Verify .NET SDK installation

## Next Steps
After successful setup:
1. Clone the repository
2. Open SmartAssistant.sln
3. Build the solution
4. Run the application to verify everything works

# 构建说明

## 环境要求
- Windows 10或更高版本
- Visual Studio 2019或更高版本
- .NET 6.0 SDK
- CMake 3.20或更高版本
- vcpkg包管理器

## 构建步骤
1. 克隆仓库
```bash
git clone https://github.com/lusong2161/desktop-assistant-v2.git
cd desktop-assistant-v2
```

2. 安装依赖
```bash
# 安装vcpkg依赖
.\vcpkg install openssl:x64-windows
.\vcpkg install boost:x64-windows
.\vcpkg install sqlite3:x64-windows
.\vcpkg install websocketpp:x64-windows
.\vcpkg install gtest:x64-windows

# 安装.NET依赖
dotnet restore
```

3. 构建C++核心库
```bash
cmake -B build -S SmartAssistant.Core
cmake --build build --config Release
```

4. 构建C# UI项目
```bash
dotnet build SmartAssistant.UI/SmartAssistant.UI.csproj -c Release
```

5. 运行测试
```bash
cd build
ctest -C Release
cd ..
dotnet test
```

## 核心功能
- 智能对话系统
- 用户通信功能
- 文件传输共享
- 协同办公管理

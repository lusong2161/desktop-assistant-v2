#!/bin/bash

# 测试Flutter环境
echo "正在测试Flutter环境..."

# 创建Flutter项目
cd /app
flutter create .

# 添加依赖
flutter pub add window_manager file_picker shared_preferences http path_provider desktop_window flutter_markdown provider sqflite_common_ffi ffi

# 启用Linux桌面支持
flutter config --enable-linux-desktop

# 运行Flutter doctor
flutter doctor -v

# 尝试构建
flutter build linux

echo "环境测试完成"

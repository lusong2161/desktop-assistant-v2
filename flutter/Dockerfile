FROM ubuntu:22.04

# 避免交互式提示
ENV DEBIAN_FRONTEND=noninteractive

# 设置时区
ENV TZ=Asia/Shanghai

# 安装基础工具和依赖
RUN apt-get update && apt-get install -y \
    curl \
    git \
    unzip \
    xz-utils \
    zip \
    libglu1-mesa \
    clang \
    cmake \
    ninja-build \
    pkg-config \
    libgtk-3-dev \
    liblzma-dev \
    libstdc++-12-dev \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    x11-apps \
    xauth \
    && rm -rf /var/lib/apt/lists/*

# 创建flutter用户和工作目录
RUN useradd -ms /bin/bash flutter
USER flutter
WORKDIR /home/flutter

# 下载并配置Flutter SDK
RUN git clone https://github.com/flutter/flutter.git -b stable
ENV PATH="/home/flutter/flutter/bin:${PATH}"
RUN flutter precache
RUN flutter config --enable-linux-desktop
RUN flutter doctor

# 设置显示环境变量
ENV DISPLAY=:0

WORKDIR /app
CMD ["bash"]

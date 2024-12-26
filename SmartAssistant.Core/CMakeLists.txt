cmake_minimum_required(VERSION 3.15)
project(SmartAssistant.Core VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖项
find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
find_package(cpprestsdk CONFIG REQUIRED)

# 设置源文件
file(GLOB_RECURSE SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# 设置头文件
file(GLOB_RECURSE HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
)

# 确保有源文件
if(NOT SOURCES)
    message(FATAL_ERROR "No source files found!")
endif()

# 创建库
add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

# 设置C++标准属性
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# 设置包含目录
target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 链接依赖项
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Boost::boost
        OpenSSL::SSL
        OpenSSL::Crypto
        SQLite::SQLite3
        nlohmann_json::nlohmann_json
        CURL::libcurl
        fmt::fmt
        spdlog::spdlog
        cpprestsdk::cpprest
)

# 启用测试
enable_testing()
add_subdirectory(tests)

# 设置编译选项
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)

# 安装配置
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include/${PROJECT_NAME}
    FILES_MATCHING PATTERN "*.h"
)

cmake_minimum_required(VERSION 3.15)
project(SmartAssistant.Core.Tests LANGUAGES CXX)

# 启用测试
enable_testing()

# 查找GTest包
find_package(GTest CONFIG REQUIRED)

# 设置源文件
file(GLOB_RECURSE TEST_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
)

# 确保有源文件
if(NOT TEST_SOURCES)
    message(FATAL_ERROR "No test source files found!")
endif()

# 创建测试可执行文件
add_executable(${PROJECT_NAME} ${TEST_SOURCES})

# 设置C++标准
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# 链接依赖项
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        SmartAssistant.Core
        GTest::gtest
        GTest::gtest_main
)

# 添加测试
add_test(
    NAME ${PROJECT_NAME}
    COMMAND ${PROJECT_NAME}
)

# 设置包含目录
target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# 设置编译选项
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra>
)

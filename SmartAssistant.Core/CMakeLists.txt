cmake_minimum_required(VERSION 3.15)
project(SmartAssistant.Core)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(websocketpp REQUIRED)
find_package(GTest REQUIRED)

# Set source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

# Create shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENSSL_INCLUDE_DIR}
    ${SQLite3_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${OPENSSL_LIBRARIES}
    ${SQLite3_LIBRARIES}
    ws2_32
    userenv
)

# Set Windows-specific properties
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _WIN32_WINNT=0x0601
    )
endif()

# Tests
enable_testing()
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
add_executable(${PROJECT_NAME}.Tests ${TEST_SOURCES})
target_link_libraries(${PROJECT_NAME}.Tests
    ${PROJECT_NAME}
    GTest::GTest
    GTest::Main
)

add_test(NAME ${PROJECT_NAME}.Tests COMMAND ${PROJECT_NAME}.Tests)

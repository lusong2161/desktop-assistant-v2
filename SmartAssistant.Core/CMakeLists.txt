cmake_minimum_required(VERSION 3.15)
project(SmartAssistant.Core)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# Windows-specific settings
if(WIN32)
    add_definitions(
        -DWIN32_LEAN_AND_MEAN
        -DNOMINMAX
        -D_WIN32_WINNT=0x0A00
        -DUNICODE
        -D_UNICODE
    )
endif()

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(websocketpp REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem thread)
find_package(GTest REQUIRED)

# Set source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/AIService.cpp"
    "src/FileTransferManager.cpp"
    "src/DocumentService.cpp"
    "src/NetworkService.cpp"
    "src/NotificationService.cpp"
    "src/SystemControl.cpp"
    "src/Configuration.cpp"
    "src/AuthenticationManager.cpp"
    "src/ApiClient.cpp"
    "src/CryptoService.cpp"
)

file(GLOB_RECURSE HEADERS 
    "include/*.h"
    "include/AIService.h"
    "include/FileTransferManager.h"
    "include/DocumentService.h"
    "include/NetworkService.h"
    "include/NotificationService.h"
    "include/SystemControl.h"
    "include/Configuration.h"
    "include/AuthenticationManager.h"
    "include/ApiClient.h"
    "include/CryptoService.h"
    "include/Common.h"
)

# Create shared library
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENSSL_INCLUDE_DIR}
    ${SQLite3_INCLUDE_DIRS}
    ${Boost_INCLUDE_DIRS}
    ${WEBSOCKETPP_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${OPENSSL_LIBRARIES}
    ${SQLite3_LIBRARIES}
    ${Boost_LIBRARIES}
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME}
        ws2_32
        userenv
        shell32
        ole32
        oleaut32
    )
endif()

# Set Windows-specific properties
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _WIN32_WINNT=0x0A00  # Target Windows 10
        UNICODE
        _UNICODE
        STRICT
        SECURITY_WIN32
    )
    
    # Enable security features
    target_compile_options(${PROJECT_NAME} PRIVATE
        /guard:cf  # Control Flow Guard
        /GS        # Buffer Security Check
        /DYNAMICBASE # ASLR
        /NXCOMPAT  # DEP
        /MP        # Multi-processor compilation
        /W4        # Warning level 4
        /WX        # Treat warnings as errors
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

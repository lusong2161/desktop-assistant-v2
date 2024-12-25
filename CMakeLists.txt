cmake_minimum_required(VERSION 3.20)
project(SmartAssistant VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Windows specific settings
if(WIN32)
    # Enable Control Flow Guard
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /guard:cf")
    # Enable Buffer Security Check
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /GS")
    # Enable ASLR
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /DYNAMICBASE")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /DYNAMICBASE")
    # Enable NX
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NXCOMPAT")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NXCOMPAT")
    # Enable latest Windows SDK
    set(CMAKE_SYSTEM_VERSION "10.0")
endif()

# Find required packages using vcpkg
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem thread)
find_package(SQLite3 REQUIRED)
find_package(GTest REQUIRED)

# Add core library subdirectory
add_subdirectory(SmartAssistant.Core)

# Enable testing
enable_testing()
add_subdirectory(SmartAssistant.Core/tests)

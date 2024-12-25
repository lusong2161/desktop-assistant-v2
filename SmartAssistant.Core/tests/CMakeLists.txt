# Test configuration
enable_testing()

# Add test executable
add_executable(SmartAssistantTests
    main.cpp
    AIServiceTests.cpp
    FileTransferTests.cpp
    NetworkServiceTests.cpp
    DocumentServiceTests.cpp
    ApiIntegrationTests.cpp
    NetworkFeaturesTests.cpp
    SmartFeaturesTests.cpp
)

# Link test executable with Google Test and our library
target_link_libraries(SmartAssistantTests
    PRIVATE
    GTest::GTest
    GTest::Main
    SmartAssistant.Core
)

# Add tests to CTest
add_test(NAME SmartAssistantTests COMMAND SmartAssistantTests)

# Set test properties
set_target_properties(SmartAssistantTests PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Windows-specific test settings
if(WIN32)
    target_compile_definitions(SmartAssistantTests
        PRIVATE
        UNICODE
        _UNICODE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
    )
    
    # Enable security features
    target_compile_options(SmartAssistantTests
        PRIVATE
        /guard:cf
        /GS
        /sdl
    )
    
    target_link_options(SmartAssistantTests
        PRIVATE
        /DYNAMICBASE
        /NXCOMPAT
    )
endif()

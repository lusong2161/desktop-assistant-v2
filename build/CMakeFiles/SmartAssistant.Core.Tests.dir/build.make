# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ubuntu/desktop_assistant_v2/SmartAssistant.Core

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ubuntu/desktop_assistant_v2/build

# Include any dependencies generated for this target.
include CMakeFiles/SmartAssistant.Core.Tests.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include CMakeFiles/SmartAssistant.Core.Tests.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/SmartAssistant.Core.Tests.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/SmartAssistant.Core.Tests.dir/flags.make

CMakeFiles/SmartAssistant.Core.Tests.dir/tests/ApiIntegrationTests.cpp.o: CMakeFiles/SmartAssistant.Core.Tests.dir/flags.make
CMakeFiles/SmartAssistant.Core.Tests.dir/tests/ApiIntegrationTests.cpp.o: /home/ubuntu/desktop_assistant_v2/SmartAssistant.Core/tests/ApiIntegrationTests.cpp
CMakeFiles/SmartAssistant.Core.Tests.dir/tests/ApiIntegrationTests.cpp.o: CMakeFiles/SmartAssistant.Core.Tests.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/desktop_assistant_v2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/SmartAssistant.Core.Tests.dir/tests/ApiIntegrationTests.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/SmartAssistant.Core.Tests.dir/tests/ApiIntegrationTests.cpp.o -MF CMakeFiles/SmartAssistant.Core.Tests.dir/tests/ApiIntegrationTests.cpp.o.d -o CMakeFiles/SmartAssistant.Core.Tests.dir/tests/ApiIntegrationTests.cpp.o -c /home/ubuntu/desktop_assistant_v2/SmartAssistant.Core/tests/ApiIntegrationTests.cpp

CMakeFiles/SmartAssistant.Core.Tests.dir/tests/ApiIntegrationTests.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SmartAssistant.Core.Tests.dir/tests/ApiIntegrationTests.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/desktop_assistant_v2/SmartAssistant.Core/tests/ApiIntegrationTests.cpp > CMakeFiles/SmartAssistant.Core.Tests.dir/tests/ApiIntegrationTests.cpp.i

CMakeFiles/SmartAssistant.Core.Tests.dir/tests/ApiIntegrationTests.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SmartAssistant.Core.Tests.dir/tests/ApiIntegrationTests.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/desktop_assistant_v2/SmartAssistant.Core/tests/ApiIntegrationTests.cpp -o CMakeFiles/SmartAssistant.Core.Tests.dir/tests/ApiIntegrationTests.cpp.s

CMakeFiles/SmartAssistant.Core.Tests.dir/tests/DocumentServiceTests.cpp.o: CMakeFiles/SmartAssistant.Core.Tests.dir/flags.make
CMakeFiles/SmartAssistant.Core.Tests.dir/tests/DocumentServiceTests.cpp.o: /home/ubuntu/desktop_assistant_v2/SmartAssistant.Core/tests/DocumentServiceTests.cpp
CMakeFiles/SmartAssistant.Core.Tests.dir/tests/DocumentServiceTests.cpp.o: CMakeFiles/SmartAssistant.Core.Tests.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/desktop_assistant_v2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/SmartAssistant.Core.Tests.dir/tests/DocumentServiceTests.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/SmartAssistant.Core.Tests.dir/tests/DocumentServiceTests.cpp.o -MF CMakeFiles/SmartAssistant.Core.Tests.dir/tests/DocumentServiceTests.cpp.o.d -o CMakeFiles/SmartAssistant.Core.Tests.dir/tests/DocumentServiceTests.cpp.o -c /home/ubuntu/desktop_assistant_v2/SmartAssistant.Core/tests/DocumentServiceTests.cpp

CMakeFiles/SmartAssistant.Core.Tests.dir/tests/DocumentServiceTests.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SmartAssistant.Core.Tests.dir/tests/DocumentServiceTests.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/desktop_assistant_v2/SmartAssistant.Core/tests/DocumentServiceTests.cpp > CMakeFiles/SmartAssistant.Core.Tests.dir/tests/DocumentServiceTests.cpp.i

CMakeFiles/SmartAssistant.Core.Tests.dir/tests/DocumentServiceTests.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SmartAssistant.Core.Tests.dir/tests/DocumentServiceTests.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/desktop_assistant_v2/SmartAssistant.Core/tests/DocumentServiceTests.cpp -o CMakeFiles/SmartAssistant.Core.Tests.dir/tests/DocumentServiceTests.cpp.s

CMakeFiles/SmartAssistant.Core.Tests.dir/tests/NetworkFeaturesTests.cpp.o: CMakeFiles/SmartAssistant.Core.Tests.dir/flags.make
CMakeFiles/SmartAssistant.Core.Tests.dir/tests/NetworkFeaturesTests.cpp.o: /home/ubuntu/desktop_assistant_v2/SmartAssistant.Core/tests/NetworkFeaturesTests.cpp
CMakeFiles/SmartAssistant.Core.Tests.dir/tests/NetworkFeaturesTests.cpp.o: CMakeFiles/SmartAssistant.Core.Tests.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/desktop_assistant_v2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/SmartAssistant.Core.Tests.dir/tests/NetworkFeaturesTests.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/SmartAssistant.Core.Tests.dir/tests/NetworkFeaturesTests.cpp.o -MF CMakeFiles/SmartAssistant.Core.Tests.dir/tests/NetworkFeaturesTests.cpp.o.d -o CMakeFiles/SmartAssistant.Core.Tests.dir/tests/NetworkFeaturesTests.cpp.o -c /home/ubuntu/desktop_assistant_v2/SmartAssistant.Core/tests/NetworkFeaturesTests.cpp

CMakeFiles/SmartAssistant.Core.Tests.dir/tests/NetworkFeaturesTests.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SmartAssistant.Core.Tests.dir/tests/NetworkFeaturesTests.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/desktop_assistant_v2/SmartAssistant.Core/tests/NetworkFeaturesTests.cpp > CMakeFiles/SmartAssistant.Core.Tests.dir/tests/NetworkFeaturesTests.cpp.i

CMakeFiles/SmartAssistant.Core.Tests.dir/tests/NetworkFeaturesTests.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SmartAssistant.Core.Tests.dir/tests/NetworkFeaturesTests.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/desktop_assistant_v2/SmartAssistant.Core/tests/NetworkFeaturesTests.cpp -o CMakeFiles/SmartAssistant.Core.Tests.dir/tests/NetworkFeaturesTests.cpp.s

CMakeFiles/SmartAssistant.Core.Tests.dir/tests/SmartFeaturesTests.cpp.o: CMakeFiles/SmartAssistant.Core.Tests.dir/flags.make
CMakeFiles/SmartAssistant.Core.Tests.dir/tests/SmartFeaturesTests.cpp.o: /home/ubuntu/desktop_assistant_v2/SmartAssistant.Core/tests/SmartFeaturesTests.cpp
CMakeFiles/SmartAssistant.Core.Tests.dir/tests/SmartFeaturesTests.cpp.o: CMakeFiles/SmartAssistant.Core.Tests.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ubuntu/desktop_assistant_v2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/SmartAssistant.Core.Tests.dir/tests/SmartFeaturesTests.cpp.o"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT CMakeFiles/SmartAssistant.Core.Tests.dir/tests/SmartFeaturesTests.cpp.o -MF CMakeFiles/SmartAssistant.Core.Tests.dir/tests/SmartFeaturesTests.cpp.o.d -o CMakeFiles/SmartAssistant.Core.Tests.dir/tests/SmartFeaturesTests.cpp.o -c /home/ubuntu/desktop_assistant_v2/SmartAssistant.Core/tests/SmartFeaturesTests.cpp

CMakeFiles/SmartAssistant.Core.Tests.dir/tests/SmartFeaturesTests.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/SmartAssistant.Core.Tests.dir/tests/SmartFeaturesTests.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ubuntu/desktop_assistant_v2/SmartAssistant.Core/tests/SmartFeaturesTests.cpp > CMakeFiles/SmartAssistant.Core.Tests.dir/tests/SmartFeaturesTests.cpp.i

CMakeFiles/SmartAssistant.Core.Tests.dir/tests/SmartFeaturesTests.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/SmartAssistant.Core.Tests.dir/tests/SmartFeaturesTests.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ubuntu/desktop_assistant_v2/SmartAssistant.Core/tests/SmartFeaturesTests.cpp -o CMakeFiles/SmartAssistant.Core.Tests.dir/tests/SmartFeaturesTests.cpp.s

# Object files for target SmartAssistant.Core.Tests
SmartAssistant_Core_Tests_OBJECTS = \
"CMakeFiles/SmartAssistant.Core.Tests.dir/tests/ApiIntegrationTests.cpp.o" \
"CMakeFiles/SmartAssistant.Core.Tests.dir/tests/DocumentServiceTests.cpp.o" \
"CMakeFiles/SmartAssistant.Core.Tests.dir/tests/NetworkFeaturesTests.cpp.o" \
"CMakeFiles/SmartAssistant.Core.Tests.dir/tests/SmartFeaturesTests.cpp.o"

# External object files for target SmartAssistant.Core.Tests
SmartAssistant_Core_Tests_EXTERNAL_OBJECTS =

SmartAssistant.Core.Tests: CMakeFiles/SmartAssistant.Core.Tests.dir/tests/ApiIntegrationTests.cpp.o
SmartAssistant.Core.Tests: CMakeFiles/SmartAssistant.Core.Tests.dir/tests/DocumentServiceTests.cpp.o
SmartAssistant.Core.Tests: CMakeFiles/SmartAssistant.Core.Tests.dir/tests/NetworkFeaturesTests.cpp.o
SmartAssistant.Core.Tests: CMakeFiles/SmartAssistant.Core.Tests.dir/tests/SmartFeaturesTests.cpp.o
SmartAssistant.Core.Tests: CMakeFiles/SmartAssistant.Core.Tests.dir/build.make
SmartAssistant.Core.Tests: libSmartAssistant.Core.so
SmartAssistant.Core.Tests: /usr/lib/x86_64-linux-gnu/libssl.so
SmartAssistant.Core.Tests: /usr/lib/x86_64-linux-gnu/libcrypto.so
SmartAssistant.Core.Tests: /usr/lib/x86_64-linux-gnu/libsqlite3.so
SmartAssistant.Core.Tests: /usr/lib/x86_64-linux-gnu/libgtest_main.a
SmartAssistant.Core.Tests: /usr/lib/x86_64-linux-gnu/libgtest.a
SmartAssistant.Core.Tests: CMakeFiles/SmartAssistant.Core.Tests.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ubuntu/desktop_assistant_v2/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX executable SmartAssistant.Core.Tests"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/SmartAssistant.Core.Tests.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/SmartAssistant.Core.Tests.dir/build: SmartAssistant.Core.Tests
.PHONY : CMakeFiles/SmartAssistant.Core.Tests.dir/build

CMakeFiles/SmartAssistant.Core.Tests.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/SmartAssistant.Core.Tests.dir/cmake_clean.cmake
.PHONY : CMakeFiles/SmartAssistant.Core.Tests.dir/clean

CMakeFiles/SmartAssistant.Core.Tests.dir/depend:
	cd /home/ubuntu/desktop_assistant_v2/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ubuntu/desktop_assistant_v2/SmartAssistant.Core /home/ubuntu/desktop_assistant_v2/SmartAssistant.Core /home/ubuntu/desktop_assistant_v2/build /home/ubuntu/desktop_assistant_v2/build /home/ubuntu/desktop_assistant_v2/build/CMakeFiles/SmartAssistant.Core.Tests.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/SmartAssistant.Core.Tests.dir/depend


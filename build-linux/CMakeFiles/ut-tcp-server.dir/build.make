# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
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
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /media/lehoang318/wspace/docker-shared/communication

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /media/lehoang318/wspace/docker-shared/communication/build-linux

# Include any dependencies generated for this target.
include CMakeFiles/ut-tcp-server.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ut-tcp-server.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ut-tcp-server.dir/flags.make

CMakeFiles/ut-tcp-server.dir/test/ut_tcp_server.cpp.o: CMakeFiles/ut-tcp-server.dir/flags.make
CMakeFiles/ut-tcp-server.dir/test/ut_tcp_server.cpp.o: ../test/ut_tcp_server.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/media/lehoang318/wspace/docker-shared/communication/build-linux/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/ut-tcp-server.dir/test/ut_tcp_server.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ut-tcp-server.dir/test/ut_tcp_server.cpp.o -c /media/lehoang318/wspace/docker-shared/communication/test/ut_tcp_server.cpp

CMakeFiles/ut-tcp-server.dir/test/ut_tcp_server.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ut-tcp-server.dir/test/ut_tcp_server.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /media/lehoang318/wspace/docker-shared/communication/test/ut_tcp_server.cpp > CMakeFiles/ut-tcp-server.dir/test/ut_tcp_server.cpp.i

CMakeFiles/ut-tcp-server.dir/test/ut_tcp_server.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ut-tcp-server.dir/test/ut_tcp_server.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /media/lehoang318/wspace/docker-shared/communication/test/ut_tcp_server.cpp -o CMakeFiles/ut-tcp-server.dir/test/ut_tcp_server.cpp.s

# Object files for target ut-tcp-server
ut__tcp__server_OBJECTS = \
"CMakeFiles/ut-tcp-server.dir/test/ut_tcp_server.cpp.o"

# External object files for target ut-tcp-server
ut__tcp__server_EXTERNAL_OBJECTS =

ut-tcp-server: CMakeFiles/ut-tcp-server.dir/test/ut_tcp_server.cpp.o
ut-tcp-server: CMakeFiles/ut-tcp-server.dir/build.make
ut-tcp-server: libcomm.a
ut-tcp-server: libtest-vectors.a
ut-tcp-server: CMakeFiles/ut-tcp-server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/media/lehoang318/wspace/docker-shared/communication/build-linux/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ut-tcp-server"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ut-tcp-server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ut-tcp-server.dir/build: ut-tcp-server

.PHONY : CMakeFiles/ut-tcp-server.dir/build

CMakeFiles/ut-tcp-server.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ut-tcp-server.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ut-tcp-server.dir/clean

CMakeFiles/ut-tcp-server.dir/depend:
	cd /media/lehoang318/wspace/docker-shared/communication/build-linux && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /media/lehoang318/wspace/docker-shared/communication /media/lehoang318/wspace/docker-shared/communication /media/lehoang318/wspace/docker-shared/communication/build-linux /media/lehoang318/wspace/docker-shared/communication/build-linux /media/lehoang318/wspace/docker-shared/communication/build-linux/CMakeFiles/ut-tcp-server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ut-tcp-server.dir/depend


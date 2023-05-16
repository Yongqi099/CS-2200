make_minimum_required(VERSION 3.0)

project(Word_Generator)

# Set the C standard to use
set(CMAKE_C_STANDARD 11)

# Add the main executable target
add_executable(Word_Generator main.c arraylist.c arraylist_tests.c)

# Add any necessary include directories
target_include_directories(Word_Generator PUBLIC .)

# Set any necessary compile options
target_compile_options(Word_Generator PUBLIC -Wall -Wextra)

# Add a "clean" target to remove build artifacts
add_custom_target(clean
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
        COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/Makefile
        COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/cmake_install.cmake
        COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_BINARY_DIR}/CMakeCache.txt
        )

# Add an "all" target to build the main executable
add_custom_target(all DEPENDS Word_Generator)

# Add an "install" target to install the main executable
install(TARGETS Word_Generator DESTINATION bin)
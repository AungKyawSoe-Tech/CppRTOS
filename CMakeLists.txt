cmake_minimum_required(VERSION 3.16)
project(FATFileSystem CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include directories
include_directories(${CMAKE_SOURCE_DIR})

# 1. Original linked list demo
add_executable(linkedlist_demo 
    main.cpp
    singly_linked_list.cpp
)

# 2. Comprehensive FAT test suite
add_executable(fat_comprehensive_test
    test_fat_fs_comprehensive.cpp
    singly_linked_list.cpp
    fat_file_system.cpp
)

# 3. Interactive FAT test
add_executable(fat_interactive_test
    interactive_test.cpp
    singly_linked_list.cpp
    fat_file_system.cpp
)

# Set target properties
set_target_properties(linkedlist_demo fat_comprehensive_test fat_interactive_test
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Enable testing
enable_testing()

# Add tests
add_test(NAME FATComprehensiveTest COMMAND fat_comprehensive_test)
add_test(NAME LinkedlistDemo COMMAND linkedlist_demo)

message(STATUS "Project: FAT File System Test Suite")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
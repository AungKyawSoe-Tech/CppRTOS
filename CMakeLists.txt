cmake_minimum_required(VERSION 3.16)
project(SinglyLinkedList CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include directories
include_directories(${CMAKE_SOURCE_DIR})

# Create executable - ONLY main.cpp needed now
add_executable(singly_linked_list_demo 
    main.cpp
)

# Set target properties for better visibility
set_target_properties(singly_linked_list_demo PROPERTIES
    OUTPUT_NAME "linkedlist_demo"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Enable testing if you want to add tests later
enable_testing()

# Add a test for the executable (optional)
add_test(NAME RunLinkedListDemo 
    COMMAND singly_linked_list_demo
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

message(STATUS "Project Name: ${PROJECT_NAME}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
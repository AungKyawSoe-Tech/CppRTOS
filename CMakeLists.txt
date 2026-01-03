cmake_minimum_required(VERSION 3.16)
project(FATFileSystem CXX)

message(STATUS "Project: FAT File System Test Suite + RTOS")

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")

# Add embedded-safe compilation flags
add_compile_options(
    -Wall 
    -Wextra
    -fno-exceptions 
    -fno-rtti
)

message(STATUS "Embedded flags: -fno-exceptions -fno-rtti")

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include directories
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

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

# Re-enable exceptions for test files only
target_compile_options(fat_comprehensive_test PRIVATE -fexceptions)

# 3. Interactive FAT test
add_executable(fat_interactive_test
    interactive_test.cpp
    singly_linked_list.cpp
    fat_file_system.cpp
)

# Re-enable exceptions for interactive test
target_compile_options(fat_interactive_test PRIVATE -fexceptions)

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
add_test(NAME KernelTest COMMAND kernel_test)
add_test(NAME SyncTest COMMAND sync_test)
add_test(NAME MemoryTest COMMAND memory_test)

# RTOS utility library
add_library(rtos_util STATIC
    src/util/rtos_string.cpp
    src/util/rtos_heap.cpp
    src/util/rtos_memory.cpp
)

# RTOS HAL library
add_library(rtos_hal STATIC
    src/rtos/hal/rtos_uart.cpp
)

# RTOS Kernel library
add_library(rtos_kernel STATIC
    src/rtos/kernel/scheduler.cpp
    src/rtos/kernel/task.cpp
    src/rtos/kernel/context.cpp
    src/rtos/kernel/mutex.cpp
    src/rtos/kernel/semaphore.cpp
    src/rtos/kernel/queue.cpp
)

# 4. RTOS utility test
add_executable(rtos_util_test
    test_rtos_util.cpp
    src/util/rtos_string.cpp
    src/rtos/hal/rtos_uart.cpp
)

# Re-enable exceptions for RTOS test
target_compile_options(rtos_util_test PRIVATE -fexceptions)

# 5. RTOS kernel test
add_executable(kernel_test
    test_kernel.cpp
)

# Re-enable exceptions for kernel test
target_compile_options(kernel_test PRIVATE -fexceptions)

# 6. RTOS synchronization test
add_executable(sync_test
    test_sync.cpp
)

# Re-enable exceptions for sync test
target_compile_options(sync_test PRIVATE -fexceptions)

# 7. RTOS memory management test
add_executable(memory_test
    test_memory.cpp
)

# Re-enable exceptions for memory test
target_compile_options(memory_test PRIVATE -fexceptions)

target_link_libraries(fat_comprehensive_test rtos_util rtos_hal)
target_link_libraries(fat_interactive_test rtos_util rtos_hal)
target_link_libraries(kernel_test rtos_kernel rtos_util rtos_hal)
target_link_libraries(sync_test rtos_kernel rtos_util rtos_hal)
target_link_libraries(memory_test rtos_util rtos_hal)

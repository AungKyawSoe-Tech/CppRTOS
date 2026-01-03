#include "fat_file_system.h"
#include "src/rtos/hal/rtos_uart.h"
#include "src/rtos/kernel/timer.h"
#include "src/rtos/kernel/scheduler.h"
#include "src/util/rtos_memory.h"
#include <cassert>

// Test FAT filesystem with RTOS integration
void test_fat_init() {
    rtos_printf("\n=== Test: FAT Initialization ===\n");
    
    FATFileSystem fs(1024, 512, "RTOS_FAT");
    
    FATFileSystem::FSInfo info = fs.getFileSystemInfo();
    rtos_printf("Total space: %zu bytes\n", info.total_space);
    rtos_printf("Free space: %zu bytes\n", info.free_space);
    assert(info.total_space > 0);
    assert(info.free_space > 0);
    
    rtos_printf("✓ FAT initialization passed\n");
}

// Test file creation
void test_file_creation() {
    rtos_printf("\n=== Test: File Creation ===\n");
    
    FATFileSystem fs(2048, 512, "RTOS_FAT");
    
    bool result = fs.createFile("/test.txt", 512);
    assert(result);
    rtos_printf("Created file: test.txt\n");
    
    assert(fs.fileExists("/test.txt"));
    assert(!fs.isDirectory("/test.txt"));
    
    size_t size = fs.getFileSize("/test.txt");
    rtos_printf("File size: %zu bytes\n", size);
    assert(size == 512);
    
    rtos_printf("✓ File creation passed\n");
}

// Test directory operations
void test_directory_operations() {
    rtos_printf("\n=== Test: Directory Operations ===\n");
    
    FATFileSystem fs(2048, 512, "RTOS_FAT");
    
    // Create directory
    bool result = fs.createDirectory("/docs");
    assert(result);
    rtos_printf("Created directory: /docs\n");
    
    assert(fs.fileExists("/docs"));
    assert(fs.isDirectory("/docs"));
    
    // Create file in directory
    result = fs.createFile("/docs/readme.txt", 256);
    assert(result);
    rtos_printf("Created file: /docs/readme.txt\n");
    
    // List directory
    StaticVector<DirectoryEntry, 256> entries = fs.listDirectory("/docs");
    rtos_printf("Directory entries: %d\n", entries.size());
    assert(entries.size() >= 1);
    
    rtos_printf("✓ Directory operations passed\n");
}

// Test file copy and delete
void test_file_operations() {
    rtos_printf("\n=== Test: File Operations ===\n");
    
    FATFileSystem fs(4096, 512, "RTOS_FAT");
    
    // Create original file
    fs.createFile("/original.txt", 1024);
    assert(fs.fileExists("/original.txt"));
    rtos_printf("Created original.txt\n");
    
    // Copy file
    bool result = fs.copyFile("/original.txt", "/copy.txt");
    assert(result);
    assert(fs.fileExists("/copy.txt"));
    rtos_printf("Copied to copy.txt\n");
    
    size_t orig_size = fs.getFileSize("/original.txt");
    size_t copy_size = fs.getFileSize("/copy.txt");
    assert(orig_size == copy_size);
    rtos_printf("Sizes match: %zu bytes\n", orig_size);
    
    // Delete file
    result = fs.deleteFile("/copy.txt");
    assert(result);
    assert(!fs.fileExists("/copy.txt"));
    rtos_printf("Deleted copy.txt\n");
    
    rtos_printf("✓ File operations passed\n");
}

// Test file system info
void test_filesystem_info() {
    rtos_printf("\n=== Test: File System Info ===\n");
    
    FATFileSystem fs(2048, 512, "RTOS_FAT");
    
    // Create some files
    fs.createFile("/file1.txt", 512);
    fs.createFile("/file2.txt", 1024);
    fs.createDirectory("/testdir");
    
    FATFileSystem::FSInfo info = fs.getFileSystemInfo();
    rtos_printf("Total space: %zu bytes\n", info.total_space);
    rtos_printf("Used space: %zu bytes\n", info.used_space);
    rtos_printf("Free space: %zu bytes\n", info.free_space);
    rtos_printf("Total files: %zu\n", info.total_files);
    rtos_printf("Total directories: %zu\n", info.total_directories);
    
    assert(info.used_space > 0);
    assert(info.free_space < info.total_space);
    assert(info.total_files >= 2);
    assert(info.total_directories >= 1);
    
    rtos_printf("✓ File system info passed\n");
}

// Test with RTOS memory integration
void test_fat_with_memory() {
    rtos_printf("\n=== Test: FAT with RTOS Memory ===\n");
    
    // Initialize RTOS memory
    static uint8_t heap_buffer[32768];
    RTOSMemory::initialize(heap_buffer, sizeof(heap_buffer));
    
    // Allocate memory for file buffer
    void* buffer = RTOSMemory::malloc(2048);
    assert(buffer != nullptr);
    rtos_printf("Allocated 2KB buffer from RTOS heap\n");
    
    // Use FAT filesystem
    FATFileSystem fs(4096, 512, "RTOS_FAT");
    fs.createFile("/test.bin", 2048);
    
    RTOSMemory::MemoryInfo info = RTOSMemory::getMemoryInfo();
    rtos_printf("Memory used: %zu bytes\n", info.used_heap_size);
    rtos_printf("Fragmentation: %u%%\n", info.fragmentation_percent);
    
    RTOSMemory::free(buffer);
    
    rtos_printf("✓ FAT with RTOS memory passed\n");
}

// Test with timer callbacks (simulating periodic file operations)
static int timer_callback_count = 0;

void fileTimerCallback(TimerHandle_t timer, void* user_data) {
    (void)timer;
    FATFileSystem* fs = static_cast<FATFileSystem*>(user_data);
    
    timer_callback_count++;
    rtos_printf("[Timer %d] Checking filesystem...\n", timer_callback_count);
    
    FATFileSystem::FSInfo info = fs->getFileSystemInfo();
    rtos_printf("  Free space: %zu bytes\n", info.free_space);
}

void test_fat_with_timer() {
    rtos_printf("\n=== Test: FAT with Timer ===\n");
    
    // Initialize timer subsystem
    TimerAPI::init();
    
    // Create filesystem
    FATFileSystem fs(4096, 512, "RTOS_FAT");
    fs.createFile("/data.log", 1024);
    
    // Create periodic timer to check filesystem
    TimerHandle_t timer = TimerAPI::create(
        "FSCheck",
        10,  // Every 10 ticks
        TIMER_PERIODIC,
        fileTimerCallback,
        &fs
    );
    assert(timer != 0);
    
    TimerAPI::start(timer);
    
    // Simulate some ticks
    for (int i = 0; i < 35; i++) {
        TimerAPI::processTick();
    }
    
    TimerAPI::stop(timer);
    TimerAPI::destroy(timer);
    
    assert(timer_callback_count >= 3);
    rtos_printf("Timer fired %d times\n", timer_callback_count);
    
    rtos_printf("✓ FAT with timer passed\n");
}

// Test multiple files and fragmentation
void test_fragmentation() {
    rtos_printf("\n=== Test: File Fragmentation ===\n");
    
    FATFileSystem fs(8192, 512, "RTOS_FAT");
    
    // Create multiple files
    for (int i = 0; i < 10; i++) {
        char filename[32];
        snprintf(filename, sizeof(filename), "/file%d.dat", i);
        fs.createFile(filename, 512);
    }
    
    rtos_printf("Created 10 files\n");
    
    // Delete some files
    fs.deleteFile("/file3.dat");
    fs.deleteFile("/file7.dat");
    rtos_printf("Deleted 2 files\n");
    
    FATFileSystem::FSInfo info = fs.getFileSystemInfo();
    rtos_printf("Total files: %zu\n", info.total_files);
    rtos_printf("Free space: %zu bytes\n", info.free_space);
    
    // Try to defragment
    fs.defragment();
    rtos_printf("Defragmented filesystem\n");
    
    rtos_printf("✓ Fragmentation test passed\n");
}

// Main test runner
int main() {
    rtos_printf("\n");
    rtos_printf("╔════════════════════════════════════════════╗\n");
    rtos_printf("║   RTOS FAT File System Test Suite        ║\n");
    rtos_printf("╚════════════════════════════════════════════╝\n");
    
    RTOS_UART::init();
    Scheduler::getInstance()->initialize(SCHEDULER_ROUND_ROBIN);
    
    try {
        test_fat_init();
        test_file_creation();
        test_directory_operations();
        test_file_operations();
        test_filesystem_info();
        test_fat_with_memory();
        test_fat_with_timer();
        test_fragmentation();
        
        rtos_printf("\n");
        rtos_printf("╔════════════════════════════════════════════╗\n");
        rtos_printf("║   ✓ ALL TESTS PASSED                      ║\n");
        rtos_printf("╚════════════════════════════════════════════╝\n");
        rtos_printf("\n");
        
        return 0;
    }
    catch (...) {
        rtos_printf("\n✗ TEST FAILED!\n");
        return 1;
    }
}

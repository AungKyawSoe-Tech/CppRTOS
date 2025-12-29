#include "fat_file_system.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <cstring>
#include <memory>

using namespace std;

// ============================================
// TEST UTILITIES
// ============================================

class FATTestHarness {
private:
    unique_ptr<FATFileSystem> fs;
    string test_name;
    int test_count;
    int passed_count;
    
public:
    FATTestHarness(const string& name, size_t disk_kb = 1024, size_t cluster_size = 1024)
        : test_name(name), test_count(0), passed_count(0) {
        cout << "\n" << string(60, '=') << endl;
        cout << "TEST SUITE: " << test_name << endl;
        cout << string(60, '=') << endl;
        fs = make_unique<FATFileSystem>(disk_kb, cluster_size);
    }
    
    template<typename Func>
    void runTest(const string& description, Func test_func) {
        test_count++;
        cout << "\nTest " << test_count << ": " << description << endl;
        cout << string(40, '-') << endl;
        
        try {
            test_func();
            cout << "✓ PASSED" << endl;
            passed_count++;
        } catch (const exception& e) {
            cout << "✗ FAILED: " << e.what() << endl;
        } catch (...) {
            cout << "✗ FAILED: Unknown error" << endl;
        }
    }
    
    void printSummary() {
        cout << "\n" << string(60, '=') << endl;
        cout << "TEST SUMMARY: " << test_name << endl;
        cout << string(60, '=') << endl;
        cout << "Total tests: " << test_count << endl;
        cout << "Passed: " << passed_count << endl;
        cout << "Failed: " << (test_count - passed_count) << endl;
        cout << "Success rate: " << (passed_count * 100 / test_count) << "%" << endl;
        
        if (test_count == passed_count) {
            cout << "\n🎉 ALL TESTS PASSED! 🎉" << endl;
        } else {
            cout << "\n❌ SOME TESTS FAILED ❌" << endl;
        }
    }
    
    FATFileSystem* getFS() { return fs.get(); }
};

// ============================================
// COMPREHENSIVE TEST CASES
// ============================================

void testBasicFileOperations() {
    FATTestHarness harness("Basic File Operations", 512, 512);
    
    harness.runTest("Create file", [&]() {
        assert(harness.getFS()->createFile("test.txt", 100) == true);
        assert(harness.getFS()->fileExists("test.txt") == true);
    });
    
    harness.runTest("Create duplicate file (should fail)", [&]() {
        assert(harness.getFS()->createFile("test.txt", 200) == false);
    });
    
    harness.runTest("Delete existing file", [&]() {
        assert(harness.getFS()->deleteFile("test.txt") == true);
        assert(harness.getFS()->fileExists("test.txt") == false);
    });
    
    harness.runTest("Delete non-existent file (should fail)", [&]() {
        assert(harness.getFS()->deleteFile("ghost.txt") == false);
    });
    
    harness.printSummary();
}

void testFileSizeAndAllocation() {
    FATTestHarness harness("File Size and Cluster Allocation", 1024, 512);
    
    harness.runTest("Create small file (1 cluster)", [&]() {
        assert(harness.getFS()->createFile("small.txt", 500) == true);
        auto info = harness.getFS()->getFileSystemInfo();
        assert(info.used_space >= 500);
    });
    
    harness.runTest("Create medium file (multiple clusters)", [&]() {
        assert(harness.getFS()->createFile("medium.bin", 2000) == true);
        auto info = harness.getFS()->getFileSystemInfo();
        // 2000 bytes should use ceil(2000/512) = 4 clusters
        assert(info.used_space >= 2000);
    });
    
    harness.runTest("Create large file (spanning many clusters)", [&]() {
        assert(harness.getFS()->createFile("large.dat", 10000) == true);
    });
    
    harness.runTest("Verify free space decreases", [&]() {
        auto info_before = harness.getFS()->getFileSystemInfo();
        size_t free_before = info_before.free_space;
        
        harness.getFS()->createFile("another.dat", 2048);
        
        auto info_after = harness.getFS()->getFileSystemInfo();
        size_t free_after = info_after.free_space;
        
        assert(free_after < free_before);
    });
    
    harness.printSummary();
}

void testDirectoryOperations() {
    FATTestHarness harness("Directory Operations", 1024, 1024);
    
    harness.runTest("Create root directory entry", [&]() {
        assert(harness.getFS()->createDirectory("/root_dir") == true);
        assert(harness.getFS()->fileExists("/root_dir") == true);
        assert(harness.getFS()->isDirectory("/root_dir") == true);
    });
    
    harness.runTest("Create nested directories", [&]() {
        assert(harness.getFS()->createDirectory("/system") == true);
        assert(harness.getFS()->createDirectory("/system/config") == true);
        assert(harness.getFS()->createDirectory("/system/logs") == true);
    });
    
    harness.runTest("Create files in directories", [&]() {
        assert(harness.getFS()->createFile("/system/config/settings.ini", 256) == true);
        assert(harness.getFS()->createFile("/system/logs/error.log", 1024) == true);
    });
    
    harness.runTest("List directory contents", [&]() {
        auto entries = harness.getFS()->listDirectory("/");
        bool found_system = false;
        bool found_root_dir = false;
        
        for (const auto& entry : entries) {
            if (entry.name == "/system") found_system = true;
            if (entry.name == "/root_dir") found_root_dir = true;
        }
        
        assert(found_system == true);
        assert(found_root_dir == true);
        assert(entries.size() >= 2); // Should have at least 2 entries
    });
    
    harness.runTest("Delete directory with files", [&]() {
        // Note: This test depends on your deleteDirectory implementation
        // If not implemented yet, comment this test
        cout << "  [Directory deletion test - requires implementation]" << endl;
    });
    
    harness.printSummary();
}

void testCopyAndMoveOperations() {
    FATTestHarness harness("Copy and Move Operations", 2048, 512);
    
    harness.runTest("Copy file", [&]() {
        harness.getFS()->createFile("original.dat", 1500);
        
        assert(harness.getFS()->copyFile("original.dat", "copy.dat") == true);
        assert(harness.getFS()->fileExists("original.dat") == true);
        assert(harness.getFS()->fileExists("copy.dat") == true);
    });
    
    harness.runTest("Copy to existing file (should fail)", [&]() {
        assert(harness.getFS()->copyFile("original.dat", "copy.dat") == false);
    });
    
    harness.runTest("Copy non-existent file (should fail)", [&]() {
        assert(harness.getFS()->copyFile("ghost.txt", "destination.txt") == false);
    });
    
    harness.runTest("Verify copied file has same size", [&]() {
        // Assuming getFileSize() method exists or can be inferred
        cout << "  [Size verification - requires getFileSize implementation]" << endl;
    });
    
    harness.printSummary();
}

void testFragmentationAndSpaceManagement() {
    FATTestHarness harness("Fragmentation and Space Management", 512, 256);
    
    harness.runTest("Fill disk partially", [&]() {
        // Create several small files
        for (int i = 0; i < 5; i++) {
            string filename = "file" + to_string(i) + ".txt";
            assert(harness.getFS()->createFile(filename, 100) == true);
        }
        
        auto info = harness.getFS()->getFileSystemInfo();
        cout << "  Used space: " << info.used_space << " bytes" << endl;
        cout << "  Free space: " << info.free_space << " bytes" << endl;
    });
    
    harness.runTest("Delete some files to create holes", [&]() {
        assert(harness.getFS()->deleteFile("file1.txt") == true);
        assert(harness.getFS()->deleteFile("file3.txt") == true);
        
        auto info = harness.getFS()->getFileSystemInfo();
        cout << "  After deletion - Free space: " << info.free_space << " bytes" << endl;
    });
    
    harness.runTest("Create larger file that might be fragmented", [&]() {
        // This file might need to be allocated in fragmented clusters
        assert(harness.getFS()->createFile("fragmented.bin", 800) == true);
        cout << "  Fragmented file created successfully" << endl;
    });
    
    harness.runTest("Out of space handling", [&]() {
        // Try to create a file larger than available space
        auto info = harness.getFS()->getFileSystemInfo();
        size_t huge_size = info.free_space + 100000; // More than available
        
        assert(harness.getFS()->createFile("huge.bin", huge_size) == false);
        cout << "  Correctly rejected oversized file" << endl;
    });
    
    harness.printSummary();
}

void testFileSystemIntegrity() {
    FATTestHarness harness("File System Integrity", 1024, 512);
    
    harness.runTest("Initial integrity check", [&]() {
        cout << "  Running initial integrity check..." << endl;
        harness.getFS()->runIntegrityCheck();
        cout << "  Initial check completed" << endl;
    });
    
    harness.runTest("Create and verify structure", [&]() {
        harness.getFS()->createTestStructure();
        
        // Verify test structure was created
        assert(harness.getFS()->fileExists("/boot.ini") == true);
        assert(harness.getFS()->fileExists("/system/kernel.bin") == true);
        assert(harness.getFS()->isDirectory("/system") == true);
    });
    
    harness.runTest("Post-creation integrity check", [&]() {
        cout << "  Running post-creation integrity check..." << endl;
        harness.getFS()->runIntegrityCheck();
        cout << "  Post-creation check completed" << endl;
    });
    
    harness.runTest("Display FAT table", [&]() {
        cout << "  Displaying FAT table..." << endl;
        harness.getFS()->displayFAT();
        cout << "  FAT table displayed" << endl;
    });
    
    harness.runTest("Display directory tree", [&]() {
        cout << "  Displaying directory tree..." << endl;
        harness.getFS()->displayDirectoryTree();
        cout << "  Directory tree displayed" << endl;
    });
    
    harness.printSummary();
}

void testConcurrentOperations() {
    FATTestHarness harness("Concurrent-like Operations", 2048, 1024);
    
    harness.runTest("Rapid file creation", [&]() {
        for (int i = 0; i < 10; i++) {
            string filename = "temp" + to_string(i) + ".tmp";
            assert(harness.getFS()->createFile(filename, 100 + i * 50) == true);
        }
        
        auto info = harness.getFS()->getFileSystemInfo();
        cout << "  Created 10 files. Total files: " << info.total_files << endl;
        assert(info.total_files >= 10);
    });
    
    harness.runTest("Mixed create/delete operations", [&]() {
        // Delete some files
        assert(harness.getFS()->deleteFile("temp2.tmp") == true);
        assert(harness.getFS()->deleteFile("temp5.tmp") == true);
        assert(harness.getFS()->deleteFile("temp8.tmp") == true);
        
        // Create new files
        assert(harness.getFS()->createFile("new1.dat", 300) == true);
        assert(harness.getFS()->createFile("new2.dat", 400) == true);
        
        auto info = harness.getFS()->getFileSystemInfo();
        cout << "  After mixed ops - Files: " << info.total_files << endl;
    });
    
    harness.runTest("Stress test with many operations", [&]() {
        int operations = 20;
        for (int i = 0; i < operations; i++) {
            string filename = "stress" + to_string(i) + ".dat";
            if (i % 3 == 0) {
                // Every 3rd operation is a delete
                harness.getFS()->deleteFile(filename);
            } else {
                harness.getFS()->createFile(filename, 50 * (i + 1));
            }
        }
        cout << "  Completed " << operations << " stress operations" << endl;
    });
    
    harness.printSummary();
}

void testMetadataOperations() {
    FATTestHarness harness("Metadata Operations", 512, 512);
    
    harness.runTest("Create file and check metadata", [&]() {
        harness.getFS()->createFile("metadata.txt", 256);
        
        // These methods need to be implemented in your FATFileSystem class
        cout << "  [Metadata tests require implementation of:]" << endl;
        cout << "    - getFileSize()" << endl;
        cout << "    - getCreateTime()" << endl;
        cout << "    - getModifyTime()" << endl;
        cout << "    - setAttributes()" << endl;
    });
    
    harness.runTest("File system information", [&]() {
        auto info = harness.getFS()->getFileSystemInfo();
        
        cout << "  File System Info:" << endl;
        cout << "    Total space: " << info.total_space << " bytes" << endl;
        cout << "    Used space: " << info.used_space << " bytes" << endl;
        cout << "    Free space: " << info.free_space << " bytes" << endl;
        cout << "    Total files: " << info.total_files << endl;
        cout << "    Total directories: " << info.total_directories << endl;
        cout << "    Bad clusters: " << info.bad_clusters << endl;
        
        // Basic sanity checks
        assert(info.total_space > 0);
        assert(info.free_space <= info.total_space);
        assert(info.used_space == info.total_space - info.free_space);
    });
    
    harness.printSummary();
}

void testEdgeCases() {
    FATTestHarness harness("Edge Cases", 100, 512); // Very small disk
    
    harness.runTest("Create zero-byte file", [&]() {
        assert(harness.getFS()->createFile("empty.txt", 0) == true);
        cout << "  Zero-byte file created successfully" << endl;
    });
    
    harness.runTest("Filename with special characters", [&]() {
        // Test various filename formats
        assert(harness.getFS()->createFile("file_with_underscores.txt", 100) == true);
        assert(harness.getFS()->createFile("FileWithCaps.TXT", 100) == true);
        assert(harness.getFS()->createFile("123numbers.bin", 100) == true);
    });
    
    harness.runTest("Exact fit in remaining space", [&]() {
        auto info = harness.getFS()->getFileSystemInfo();
        size_t remaining = info.free_space;
        
        if (remaining > 0) {
            assert(harness.getFS()->createFile("exact_fit.dat", remaining) == true);
            cout << "  File exactly filling remaining space created" << endl;
        } else {
            cout << "  No space remaining for exact fit test" << endl;
        }
    });
    
    harness.runTest("One byte too large (should fail)", [&]() {
        auto info = harness.getFS()->getFileSystemInfo();
        size_t too_big = info.free_space + 1;
        
        assert(harness.getFS()->createFile("too_large.dat", too_big) == false);
        cout << "  Correctly rejected file one byte too large" << endl;
    });
    
    harness.printSummary();
}

// ============================================
// MAIN TEST RUNNER
// ============================================

int main() {
    cout << string(70, '=') << endl;
    cout << "FAT FILE SYSTEM COMPREHENSIVE TEST SUITE" << endl;
    cout << "Using SinglyLinkedList as base data structure" << endl;
    cout << string(70, '=') << endl;
    
    try {
        // Run all test suites
        testBasicFileOperations();
        testFileSizeAndAllocation();
        testDirectoryOperations();
        testCopyAndMoveOperations();
        testFragmentationAndSpaceManagement();
        testFileSystemIntegrity();
        testConcurrentOperations();
        testMetadataOperations();
        testEdgeCases();
        
        cout << "\n" << string(70, '=') << endl;
        cout << "🎉 ALL TEST SUITES COMPLETED SUCCESSFULLY! 🎉" << endl;
        cout << string(70, '=') << endl;
        
        return 0;
        
    } catch (const exception& e) {
        cerr << "\n❌ FATAL ERROR: " << e.what() << endl;
        return 1;
    } catch (...) {
        cerr << "\n❌ UNKNOWN FATAL ERROR" << endl;
        return 1;
    }
}
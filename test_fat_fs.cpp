#include "fat_file_system.h"
#include <iostream>
#include <cassert>
#include <vector>

using namespace std;

void testBasicFileOperations() {
    cout << "\n=== Test 1: Basic File Operations ===" << endl;
    
    // Create a small file system
    FATFileSystem fs(512, 512);  // 256KB total
    
    // Test file creation
    assert(fs.createFile("test1.txt", 100) == true);
    assert(fs.createFile("test2.dat", 500) == true);
    assert(fs.fileExists("test1.txt") == true);
    assert(fs.fileExists("test2.dat") == true);
    assert(fs.fileExists("nonexistent.txt") == false);
    
    // Test duplicate file creation
    assert(fs.createFile("test1.txt", 200) == false);
    
    cout << "✓ Basic file creation tests passed" << endl;
}

void testFileDeletion() {
    cout << "\n=== Test 2: File Deletion ===" << endl;
    
    FATFileSystem fs(512, 512);
    
    fs.createFile("delete_me.txt", 100);
    assert(fs.fileExists("delete_me.txt") == true);
    
    assert(fs.deleteFile("delete_me.txt") == true);
    assert(fs.fileExists("delete_me.txt") == false);
    
    // Test deleting non-existent file
    assert(fs.deleteFile("ghost.txt") == false);
    
    cout << "✓ File deletion tests passed" << endl;
}

void testCopyAndMove() {
    cout << "\n=== Test 3: Copy and Move Operations ===" << endl;
    
    FATFileSystem fs(1024, 1024);  // 1MB
    
    // Create source file
    fs.createFile("source.txt", 2048);
    
    // Test copy
    assert(fs.copyFile("source.txt", "dest.txt") == true);
    assert(fs.fileExists("source.txt") == true);
    assert(fs.fileExists("dest.txt") == true);
    
    // Test copy to existing file (should fail)
    assert(fs.copyFile("source.txt", "dest.txt") == false);
    
    cout << "✓ Copy operations tests passed" << endl;
}

void testDirectoryOperations() {
    cout << "\n=== Test 4: Directory Operations ===" << endl;
    
    FATFileSystem fs(1024, 512);
    
    // Create directories
    assert(fs.createDirectory("/system") == true);
    assert(fs.createDirectory("/users") == true);
    assert(fs.createDirectory("/temp") == true);
    
    // Create files in directories
    assert(fs.createFile("/system/config.ini", 256) == true);
    assert(fs.createFile("/users/profile.dat", 1024) == true);
    
    // List directory
    auto entries = fs.listDirectory("/");
    cout << "Directory listing:" << endl;
    for (const auto& entry : entries) {
        cout << "  " << (entry.is_dir ? "[DIR] " : "[FILE] ") 
             << entry.name;
        if (!entry.is_dir) {
            cout << " (" << entry.size << " bytes)";
        }
        cout << endl;
    }
    
    cout << "✓ Directory operations tests passed" << endl;
}

void testFileSystemInfo() {
    cout << "\n=== Test 5: File System Information ===" << endl;
    
    FATFileSystem fs(2048, 1024);  // 2MB
    
    // Create some files
    fs.createFile("file1.txt", 500);
    fs.createFile("file2.txt", 1500);
    fs.createDirectory("docs");
    
    // Get file system info
    auto info = fs.getFileSystemInfo();
    
    cout << "File System Info:" << endl;
    cout << "  Total space: " << info.total_space / 1024 << " KB" << endl;
    cout << "  Used space: " << info.used_space / 1024 << " KB" << endl;
    cout << "
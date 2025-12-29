#ifndef FAT_FILE_SYSTEM_H
#define FAT_FILE_SYSTEM_H

#include "singly_linked_list.h"
#include <string>
#include <vector>
#include <memory>
#include <ctime>
#include <map>

// ============================================
// FAT-SPECIFIC STRUCTURES
// ============================================

// FAT Cluster Entry (12/16/32-bit in real FAT)
struct FATCluster {
    int cluster_number;
    bool is_allocated;
    bool is_bad;
    int next_cluster;  // -1 for EOF, -2 for free
    
    FATCluster(int num) : cluster_number(num), 
                         is_allocated(false), 
                         is_bad(false), 
                         next_cluster(-2) {}
    
    bool isFree() const { return next_cluster == -2; }
    bool isEOF() const { return next_cluster == -1; }
    bool isChain() const { return next_cluster >= 0; }
};

// File Control Block (FCB) - like inode in Unix
struct FileControlBlock {
    std::string filename;
    int start_cluster;
    size_t file_size;
    time_t create_time;
    time_t modify_time;
    time_t access_time;
    bool is_directory;
    bool is_hidden;
    bool is_readonly;
    
    // For directories: list of child files
    SinglyLinkedList<std::string> directory_entries;
    
    FileControlBlock(const std::string& name, int start = -1, bool is_dir = false)
        : filename(name), start_cluster(start), file_size(0), 
          is_directory(is_dir), is_hidden(false), is_readonly(false) {
        time_t now = time(nullptr);
        create_time = modify_time = access_time = now;
    }
    
    void updateModifyTime() { modify_time = time(nullptr); }
    void updateAccessTime() { access_time = time(nullptr); }
};

// Directory Entry
struct DirectoryEntry {
    std::string name;
    int start_cluster;
    size_t size;
    bool is_dir;
    
    DirectoryEntry(const std::string& n, int cluster, size_t sz, bool dir)
        : name(n), start_cluster(cluster), size(sz), is_dir(dir) {}
};

// ============================================
// FAT FILE SYSTEM CLASS
// ============================================

class FATFileSystem {
private:
    // Core FAT structures using your SinglyLinkedList
    SinglyLinkedList<FATCluster> fat_table;      // FAT chain
    SinglyLinkedList<FileControlBlock> directory; // Root directory
    
    // File system parameters
    size_t total_clusters;
    size_t cluster_size;          // Bytes per cluster (typically 512B-4KB)
    size_t free_clusters;
    std::string volume_label;
    
    // Current working directory
    FileControlBlock* current_directory;
    
    // File handles for open files
    std::map<int, FileControlBlock*> open_files;
    int next_file_handle;
    
    // Helper methods
    int findFreeCluster() const;
    std::vector<int> getClusterChain(int start_cluster) const;
    void freeClusterChain(int start_cluster);
    FileControlBlock* findFile(const std::string& path);
    std::string getParentDirectory(const std::string& path) const;
    std::string getFilename(const std::string& path) const;
    
    // Directory operations
    bool addToDirectory(FileControlBlock* parent, const FileControlBlock& entry);
    bool removeFromDirectory(FileControlBlock* parent, const std::string& filename);
    
public:
    // ============== CONSTRUCTOR & DESTRUCTOR ==============
    
    FATFileSystem(size_t disk_size_kb = 1024, size_t cluster_size_bytes = 1024,
                  const std::string& label = "RTOS_FS");
    ~FATFileSystem();
    
    // ============== FILE SYSTEM OPERATIONS ==============
    
    bool format();
    void fsck();  // File system check
    void defragment();
    
    // ============== FILE OPERATIONS ==============
    
    bool createFile(const std::string& path, size_t initial_size = 0);
    bool deleteFile(const std::string& path);
    bool copyFile(const std::string& source, const std::string& dest);
    bool moveFile(const std::string& source, const std::string& dest);
    bool renameFile(const std::string& old_path, const std::string& new_path);
    
    // File I/O operations
    int openFile(const std::string& path, const std::string& mode = "r");
    bool closeFile(int handle);
    size_t readFile(int handle, void* buffer, size_t bytes);
    size_t writeFile(int handle, const void* data, size_t bytes);
    bool seekFile(int handle, size_t position);
    
    // ============== DIRECTORY OPERATIONS ==============
    
    bool createDirectory(const std::string& path);
    bool deleteDirectory(const std::string& path);
    bool changeDirectory(const std::string& path);
    std::vector<DirectoryEntry> listDirectory(const std::string& path = "");
    
    // ============== METADATA OPERATIONS ==============
    
    size_t getFileSize(const std::string& path) const;
    time_t getCreateTime(const std::string& path) const;
    time_t getModifyTime(const std::string& path) const;
    bool setAttributes(const std::string& path, bool hidden, bool readonly);
    
    // ============== FILE SYSTEM INFO ==============
    
    struct FSInfo {
        size_t total_space;
        size_t free_space;
        size_t used_space;
        size_t total_files;
        size_t total_directories;
        size_t bad_clusters;
    };
    
    FSInfo getFileSystemInfo() const;
    
    // ============== UTILITY METHODS ==============
    
    void displayFAT() const;
    void displayDirectoryTree() const;
    bool fileExists(const std::string& path) const;
    bool isDirectory(const std::string& path) const;
    
    // ============== TESTING HELPERS ==============
    
    void createTestStructure();
    void runIntegrityCheck() const;
};

#endif // FAT_FILE_SYSTEM_H
#include "fat_file_system.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <cstring>

using namespace std;

// ============================================
// IMPLEMENTATION
// ============================================

FATFileSystem::FATFileSystem(size_t disk_size_kb, size_t cluster_size_bytes, 
                           const std::string& label)
    : total_clusters(disk_size_kb * 1024 / cluster_size_bytes),
      cluster_size(cluster_size_bytes),
      free_clusters(total_clusters),
      volume_label(label),
      current_directory(nullptr),
      next_file_handle(1) {
    
    // Initialize FAT table
    for (size_t i = 0; i < total_clusters; i++) {
        fat_table.insertAtEnd(FATCluster(i));
    }
    
    // Mark first 2 clusters as reserved (like real FAT)
    if (total_clusters > 0) {
        FATCluster& cluster0 = fat_table.getRef(0);
        cluster0.is_bad = true;
        cluster0.is_allocated = true;
        
        if (total_clusters > 1) {
            FATCluster& cluster1 = fat_table.getRef(1);
            cluster1.is_bad = true;
            cluster1.is_allocated = true;
        }
    }
    
    // Create root directory
    FileControlBlock root("/", 2, true);
    directory.insertAtEnd(root);
    current_directory = &directory.getRef(0);
    
    // Reserve cluster 2 for root directory
    if (total_clusters > 2) {
        FATCluster& root_cluster = fat_table.getRef(2);
        root_cluster.is_allocated = true;
        root_cluster.next_cluster = -1;  // EOF for now
        free_clusters--;
    }
    
    cout << "FAT File System initialized" << endl;
    cout << "Total clusters: " << total_clusters 
         << " (" << (total_clusters * cluster_size / 1024) << " KB)" << endl;
    cout << "Cluster size: " << cluster_size << " bytes" << endl;
    cout << "Volume label: " << volume_label << endl;
}

FATFileSystem::~FATFileSystem() {
    // Close all open files
    for (auto& pair : open_files) {
        delete pair.second;
    }
    open_files.clear();
    cout << "FAT File System shutdown" << endl;
}

// ============== HELPER METHODS ==============

int FATFileSystem::findFreeCluster() const {
    for (int i = 0; i < fat_table.getSize(); i++) {
        const FATCluster& cluster = fat_table.getConstRef(i);
        if (!cluster.is_allocated && !cluster.is_bad && cluster.isFree()) {
            return i;
        }
    }
    return -1;  // No free clusters
}

vector<int> FATFileSystem::getClusterChain(int start_cluster) const {
    vector<int> chain;
    int current = start_cluster;
    
    while (current >= 0 && current < fat_table.getSize()) {
        const FATCluster& cluster = fat_table.getConstRef(current);
        chain.push_back(current);
        
        if (cluster.isEOF()) break;
        current = cluster.next_cluster;
    }
    
    return chain;
}

void FATFileSystem::freeClusterChain(int start_cluster) {
    vector<int> chain = getClusterChain(start_cluster);
    
    for (int cluster_num : chain) {
        FATCluster& cluster = fat_table.getRef(cluster_num);
        cluster.is_allocated = false;
        cluster.next_cluster = -2;  // Mark as free
        free_clusters++;
    }
}

FileControlBlock* FATFileSystem::findFile(const std::string& path) {
    // Simplified path lookup - in real FS would handle full paths
    for (int i = 0; i < directory.getSize(); i++) {
        FileControlBlock& fcb = directory.getRef(i);
        if (fcb.filename == path) {
            return &fcb;
        }
    }
    return nullptr;
}

// ============== FILE OPERATIONS ==============

bool FATFileSystem::createFile(const std::string& path, size_t initial_size) {
    if (fileExists(path)) {
        cout << "Error: File already exists: " << path << endl;
        return false;
    }
    
    // Calculate clusters needed
    size_t clusters_needed = (initial_size + cluster_size - 1) / cluster_size;
    
    if (clusters_needed > free_clusters) {
        cout << "Error: Not enough space. Need " << clusters_needed 
             << " clusters, have " << free_clusters << endl;
        return false;
    }
    
    // Allocate first cluster
    int first_cluster = findFreeCluster();
    if (first_cluster == -1) {
        cout << "Error: No free clusters found" << endl;
        return false;
    }
    
    // Mark first cluster as allocated
    FATCluster& first = fat_table.getRef(first_cluster);
    first.is_allocated = true;
    
    // Create file control block
    FileControlBlock new_file(path, first_cluster, false);
    new_file.file_size = initial_size;
    
    // Allocate additional clusters if needed
    int current_cluster = first_cluster;
    int clusters_allocated = 1;
    
    for (size_t i = 1; i < clusters_needed; i++) {
        int next_cluster = findFreeCluster();
        if (next_cluster == -1) {
            // Out of space - free what we allocated
            freeClusterChain(first_cluster);
            cout << "Error: Out of space during allocation" << endl;
            return false;
        }
        
        // Link clusters
        FATCluster& current = fat_table.getRef(current_cluster);
        current.is_allocated = true;
        current.next_cluster = next_cluster;
        
        FATCluster& next = fat_table.getRef(next_cluster);
        next.is_allocated = true;
        
        current_cluster = next_cluster;
        clusters_allocated++;
    }
    
    // Mark last cluster as EOF
    if (current_cluster != -1) {
        FATCluster& last = fat_table.getRef(current_cluster);
        last.next_cluster = -1;
    }
    
    // Update free clusters count
    free_clusters -= clusters_allocated;
    
    // Add to directory
    directory.insertAtEnd(new_file);
    
    cout << "Created file: " << path 
         << " (size: " << initial_size << " bytes, "
         << "clusters: " << clusters_allocated << ")" << endl;
    
    return true;
}

bool FATFileSystem::deleteFile(const std::string& path) {
    // Find the file and its position in the directory
    int file_position = -1;
    FileControlBlock* file = nullptr;
    
    for (int i = 0; i < directory.getSize(); i++) {
        FileControlBlock& fcb = directory.getRef(i);
        if (fcb.filename == path) {
            file = &fcb;
            file_position = i;
            break;
        }
    }
    
    if (!file) {
        cout << "Error: File not found: " << path << endl;
        return false;
    }
    
    if (file->is_directory) {
        cout << "Error: " << path << " is a directory. Use deleteDirectory()" << endl;
        return false;
    }
    
    // Free all clusters used by the file
    freeClusterChain(file->start_cluster);
    
    // Remove from directory
    directory.deleteFromPosition(file_position);
    
    cout << "Deleted file: " << path << endl;
    return true;
}

bool FATFileSystem::copyFile(const std::string& source, const std::string& dest) {
    if (!fileExists(source)) {
        cout << "Error: Source file not found: " << source << endl;
        return false;
    }
    
    if (fileExists(dest)) {
        cout << "Error: Destination file already exists: " << dest << endl;
        return false;
    }
    
    FileControlBlock* source_fcb = findFile(source);
    if (!source_fcb) return false;
    
    // Create new file with same size
    if (!createFile(dest, source_fcb->file_size)) {
        return false;
    }
    
    // In real implementation, would copy data from clusters
    // For simulation, we just copy metadata
    
    cout << "Copied file: " << source << " -> " << dest << endl;
    return true;
}

bool FATFileSystem::createDirectory(const std::string& path) {
    if (fileExists(path)) {
        cout << "Error: Path already exists: " << path << endl;
        return false;
    }
    
    // Allocate a cluster for directory (simplified)
    int dir_cluster = findFreeCluster();
    if (dir_cluster == -1) {
        cout << "Error: No space for directory" << endl;
        return false;
    }
    
    // Create directory FCB
    FileControlBlock new_dir(path, dir_cluster, true);
    
    // Mark cluster as allocated
    FATCluster& cluster = fat_table.getRef(dir_cluster);
    cluster.is_allocated = true;
    cluster.next_cluster = -1;
    free_clusters--;
    
    // Add to parent directory
    directory.insertAtEnd(new_dir);
    
    cout << "Created directory: " << path << endl;
    return true;
}

bool FATFileSystem::deleteDirectory(const std::string& path) {
    // Find the directory and its position
    int dir_position = -1;
    FileControlBlock* dir = nullptr;
    
    for (int i = 0; i < directory.getSize(); i++) {
        FileControlBlock& fcb = directory.getRef(i);
        if (fcb.filename == path) {
            dir = &fcb;
            dir_position = i;
            break;
        }
    }
    
    if (!dir) {
        cout << "Error: Directory not found: " << path << endl;
        return false;
    }
    
    if (!dir->is_directory) {
        cout << "Error: " << path << " is not a directory. Use deleteFile()" << endl;
        return false;
    }
    
    // Check if directory is empty
    if (!dir->directory_entries.isEmpty()) {
        cout << "Error: Directory is not empty: " << path << endl;
        return false;
    }
    
    // Free the cluster used by the directory
    freeClusterChain(dir->start_cluster);
    
    // Remove from directory list
    directory.deleteFromPosition(dir_position);
    
    cout << "Deleted directory: " << path << endl;
    return true;
}

vector<DirectoryEntry> FATFileSystem::listDirectory(const std::string& path) {
    vector<DirectoryEntry> entries;
    
    // Add special entries
    entries.push_back(DirectoryEntry(".", current_directory->start_cluster, 0, true));
    
    // List all files/directories
    for (int i = 0; i < directory.getSize(); i++) {
        const FileControlBlock& fcb = directory.getConstRef(i);
        entries.push_back(DirectoryEntry(
            fcb.filename, 
            fcb.start_cluster, 
            fcb.file_size, 
            fcb.is_directory
        ));
    }
    
    return entries;
}

// ============== FILE SYSTEM INFO ==============

FATFileSystem::FSInfo FATFileSystem::getFileSystemInfo() const {
    FSInfo info;
    
    info.total_space = total_clusters * cluster_size;
    info.free_space = free_clusters * cluster_size;
    info.used_space = info.total_space - info.free_space;
    
    // Count files and directories
    info.total_files = 0;
    info.total_directories = 0;
    
    for (int i = 0; i < directory.getSize(); i++) {
        const FileControlBlock& fcb = directory.getConstRef(i);
        if (fcb.is_directory) {
            info.total_directories++;
        } else {
            info.total_files++;
        }
    }
    
    // Count bad clusters
    info.bad_clusters = 0;
    for (int i = 0; i < fat_table.getSize(); i++) {
        if (fat_table.getConstRef(i).is_bad) {
            info.bad_clusters++;
        }
    }
    
    return info;
}

// ============== UTILITY METHODS ==============

void FATFileSystem::displayFAT() const {
    cout << "\n=== FAT Table (first 20 entries) ===" << endl;
    cout << "Cluster | Status    | Next" << endl;
    cout << "--------|-----------|------" << endl;
    
    int limit = min(20, fat_table.getSize());
    for (int i = 0; i < limit; i++) {
        const FATCluster& cluster = fat_table.getConstRef(i);
        
        string status;
        if (cluster.is_bad) {
            status = "BAD      ";
        } else if (cluster.is_allocated) {
            status = "ALLOCATED";
        } else {
            status = "FREE     ";
        }
        
        string next;
        if (cluster.next_cluster == -1) {
            next = "EOF";
        } else if (cluster.next_cluster == -2) {
            next = "---";
        } else {
            next = to_string(cluster.next_cluster);
        }
        
        cout << setw(7) << i << " | " << status << " | " << next << endl;
    }
    
    if (fat_table.getSize() > 20) {
        cout << "... (" << (fat_table.getSize() - 20) << " more entries)" << endl;
    }
}

void FATFileSystem::displayDirectoryTree() const {
    cout << "\n=== Directory Tree ===" << endl;
    
    for (int i = 0; i < directory.getSize(); i++) {
        const FileControlBlock& fcb = directory.getConstRef(i);
        
        string type = fcb.is_directory ? "<DIR>" : "FILE";
        string size = fcb.is_directory ? "" : to_string(fcb.file_size) + " bytes";
        
        cout << type << "\t" << fcb.filename;
        if (!size.empty()) cout << "\t" << size;
        cout << endl;
    }
}

bool FATFileSystem::fileExists(const std::string& path) const {
    for (int i = 0; i < directory.getSize(); i++) {
        if (directory.getConstRef(i).filename == path) {
            return true;
        }
    }
    return false;
}

// ============== TESTING HELPERS ==============

void FATFileSystem::createTestStructure() {
    cout << "\n=== Creating Test File Structure ===" << endl;
    
    // Create some directories
    createDirectory("/system");
    createDirectory("/users");
    createDirectory("/temp");
    
    // Create some files
    createFile("/boot.ini", 128);
    createFile("/config.txt", 512);
    createFile("/system/kernel.bin", 2048);
    createFile("/users/test.dat", 1024);
    createFile("/temp/cache.tmp", 256);
    
    cout << "Test structure created successfully" << endl;
}

void FATFileSystem::runIntegrityCheck() const {
    cout << "\n=== File System Integrity Check ===" << endl;
    
    FSInfo info = getFileSystemInfo();
    
    cout << "Total space: " << info.total_space / 1024 << " KB" << endl;
    cout << "Used space: " << info.used_space / 1024 << " KB" << endl;
    cout << "Free space: " << info.free_space / 1024 << " KB" << endl;
    cout << "Files: " << info.total_files << endl;
    cout << "Directories: " << info.total_directories << endl;
    cout << "Bad clusters: " << info.bad_clusters << endl;
    
    // Check for orphaned clusters
    int allocated_count = 0;
    for (int i = 0; i < fat_table.getSize(); i++) {
        if (fat_table.getConstRef(i).is_allocated && !fat_table.getConstRef(i).is_bad) {
            allocated_count++;
        }
    }
    
    cout << "Allocated clusters: " << allocated_count << endl;
    
    if (allocated_count == (total_clusters - free_clusters - info.bad_clusters)) {
        cout << "✓ Integrity check PASSED" << endl;
    } else {
        cout << "✗ Integrity check FAILED: Cluster count mismatch!" << endl;
    }
}

// Check if a path is a directory
bool FATFileSystem::isDirectory(const std::string& path) const {
    // Root directory
    if (path == "/" || path.empty()) {
        return true;
    }
    
    // Search in directory list
    for (int i = 0; i < directory.getSize(); i++) {
        const FileControlBlock& fcb = directory.getConstRef(i);
        if (fcb.filename == path && fcb.is_directory) {
            return true;
        }
    }
    
    return false;
}
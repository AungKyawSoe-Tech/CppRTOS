#include "fat_file_system.h"
#include "src/rtos/hal/rtos_uart.h"
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
                           const RTOSString& label)
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
    
    rtos_printf("FAT File System initialized\n");
    rtos_printf("Total clusters: %zu (%zu KB)\n", total_clusters, total_clusters * cluster_size / 1024);
    rtos_printf("Cluster size: %zu bytes\n", cluster_size);
    rtos_printf("Volume label: %s\n", volume_label.c_str());
}

FATFileSystem::~FATFileSystem() {
    // Close all open files
    for (auto it = open_files.begin(); it != open_files.end(); ++it) {
        delete it->value;
    }
    open_files.clear();
    rtos_printf("FAT File System shutdown\n");
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

StaticVector<int, 512> FATFileSystem::getClusterChain(int start_cluster) const {
    StaticVector<int, 512> chain;
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
    StaticVector<int, 512> chain = getClusterChain(start_cluster);
    
    for (size_t i = 0; i < chain.size(); i++) {
        int cluster_num = chain[i];
        FATCluster& cluster = fat_table.getRef(cluster_num);
        cluster.is_allocated = false;
        cluster.next_cluster = -2;  // Mark as free
        free_clusters++;
    }
}

FileControlBlock* FATFileSystem::findFile(const RTOSString& path) {
    // Basic path-aware lookup: handle leading '/', directory separators, and basename matches.
    // This still uses a flat directory list but allows simple hierarchical-style paths.

    // Normalize input path: remove leading '/' and extract the target filename component.
    RTOSString normalized_path = path;
    if (normalized_path.length() > 0 && (normalized_path[0] == '/' || normalized_path[0] == '\\')) {
        normalized_path = normalized_path.substr(1);
    }
    size_t sep_pos = normalized_path.find_last_of("/\\");
    RTOSString target_name = (sep_pos == RTOSString::npos)
                              ? normalized_path
                              : normalized_path.substr(sep_pos + 1);

    for (int i = 0; i < directory.getSize(); i++) {
        FileControlBlock& fcb = directory.getRef(i);

        // Normalize stored filename in the same way.
        RTOSString fcb_path = fcb.filename;
        if (fcb_path.length() > 0 && (fcb_path[0] == '/' || fcb_path[0] == '\\')) {
            fcb_path = fcb_path.substr(1);
        }

        // Prefer exact normalized path match if available.
        if (normalized_path.length() > 0 && fcb_path == normalized_path) {
            return &fcb;
        }

        // Fallback: compare only the basename (last path component).
        size_t fcb_sep_pos = fcb_path.find_last_of("/\\");
        RTOSString fcb_name = (fcb_sep_pos == RTOSString::npos)
                               ? fcb_path
                               : fcb_path.substr(fcb_sep_pos + 1);
        if (target_name.length() > 0 && fcb_name == target_name) {
            return &fcb;
        }
    }
    return nullptr;
}

// ============== FILE OPERATIONS ==============

bool FATFileSystem::createFile(const RTOSString& path, size_t initial_size) {
    if (fileExists(path)) {
        rtos_printf("Error: File already exists: %s\n", path.c_str());
        return false;
    }
    
    // Calculate clusters needed
    size_t clusters_needed = (initial_size + cluster_size - 1) / cluster_size;
    
    if (clusters_needed > free_clusters) {
        rtos_printf("Error: Not enough space. Need %zu clusters, have %zu\n", 
                    clusters_needed, free_clusters);
        return false;
    }
    
    // Allocate first cluster
    int first_cluster = findFreeCluster();
    if (first_cluster == -1) {
        rtos_printf("Error: No free clusters found\n");
        return false;
    }
    
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
            rtos_printf("Error: Out of space during allocation\n");
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
    
    rtos_printf("Created file: %s (size: %zu bytes, clusters: %d)\n",
                path.c_str(), initial_size, clusters_allocated);
    
    return true;
}

bool FATFileSystem::deleteFile(const RTOSString& path) {
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
        rtos_printf("Error: File not found: %s\n", path.c_str());
        return false;
    }
    
    if (file->is_directory) {
        rtos_printf("Error: %s is a directory. Use deleteDirectory()\n", path.c_str());
        return false;
    }
    
    // Free all clusters used by the file
    freeClusterChain(file->start_cluster);
    
    // Remove from directory
    directory.deleteFromPosition(file_position);
    
    rtos_printf("Deleted file: %s\n", path.c_str());
    return true;
}

bool FATFileSystem::copyFile(const RTOSString& source, const RTOSString& dest) {
    if (!fileExists(source)) {
        rtos_printf("Error: Source file not found: %s\n", source.c_str());
        return false;
    }
    
    if (fileExists(dest)) {
        rtos_printf("Error: Destination file already exists: %s\n", dest.c_str());
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
    
    rtos_printf("Copied file: %s -> %s\n", source.c_str(), dest.c_str());
    return true;
}

bool FATFileSystem::createDirectory(const RTOSString& path) {
    if (fileExists(path)) {
        rtos_printf("Error: Path already exists: %s\n", path.c_str());
        return false;
    }
    
    // Allocate a cluster for directory (simplified)
    int dir_cluster = findFreeCluster();
    if (dir_cluster == -1) {
        rtos_printf("Error: No space for directory\n");
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
    
    rtos_printf("Created directory: %s\n", path.c_str());
    return true;
}

bool FATFileSystem::deleteDirectory(const RTOSString& path) {
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
        rtos_printf("Error: Directory not found: %s\n", path.c_str());
        return false;
    }
    
    if (!dir->is_directory) {
        rtos_printf("Error: %s is not a directory. Use deleteFile()\n", path.c_str());
        return false;
    }
    
    // Check if directory is empty
    if (!dir->directory_entries.isEmpty()) {
        rtos_printf("Error: Directory is not empty: %s\n", path.c_str());
        return false;
    }
    
    // Free the cluster used by the directory
    freeClusterChain(dir->start_cluster);
    
    // Remove from directory list
    directory.deleteFromPosition(dir_position);
    
    rtos_printf("Deleted directory: %s\n", path.c_str());
    return true;
}

StaticVector<DirectoryEntry, 256> FATFileSystem::listDirectory(const RTOSString& path) {
    StaticVector<DirectoryEntry, 256> entries;
    
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
    rtos_printf("\n=== FAT Table (first 20 entries) ===\n");
    rtos_printf("Cluster | Status    | Next\n");
    rtos_printf("--------|-----------|------\n");
    
    int limit = min(20, fat_table.getSize());
    for (int i = 0; i < limit; i++) {
        const FATCluster& cluster = fat_table.getConstRef(i);
        
        const char* status;
        if (cluster.is_bad) {
            status = "BAD      ";
        } else if (cluster.is_allocated) {
            status = "ALLOCATED";
        } else {
            status = "FREE     ";
        }
        
        if (cluster.next_cluster == -1) {
            rtos_printf("%7d | %s | EOF\n", i, status);
        } else if (cluster.next_cluster == -2) {
            rtos_printf("%7d | %s | ---\n", i, status);
        } else {
            rtos_printf("%7d | %s | %d\n", i, status, cluster.next_cluster);
        }
    }
    
    if (fat_table.getSize() > 20) {
        rtos_printf("... (%d more entries)\n", fat_table.getSize() - 20);
    }
}

void FATFileSystem::displayDirectoryTree() const {
    rtos_printf("\n=== Directory Tree ===\n");
    
    for (int i = 0; i < directory.getSize(); i++) {
        const FileControlBlock& fcb = directory.getConstRef(i);
        
        const char* type = fcb.is_directory ? "<DIR>" : "FILE";
        
        if (fcb.is_directory) {
            rtos_printf("%s\t%s\n", type, fcb.filename.c_str());
        } else {
            rtos_printf("%s\t%s\t%zu bytes\n", type, fcb.filename.c_str(), fcb.file_size);
        }
    }
}

bool FATFileSystem::fileExists(const RTOSString& path) const {
    for (int i = 0; i < directory.getSize(); i++) {
        if (directory.getConstRef(i).filename == path) {
            return true;
        }
    }
    return false;
}

// ============== TESTING HELPERS ==============

void FATFileSystem::createTestStructure() {
    rtos_printf("\n=== Creating Test File Structure ===\n");
    
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
    
    rtos_printf("Test structure created successfully\n");
}

void FATFileSystem::runIntegrityCheck() const {
    rtos_printf("\n=== File System Integrity Check ===\n");
    
    FSInfo info = getFileSystemInfo();
    
    rtos_printf("Total space: %zu KB\n", info.total_space / 1024);
    rtos_printf("Used space: %zu KB\n", info.used_space / 1024);
    rtos_printf("Free space: %zu KB\n", info.free_space / 1024);
    rtos_printf("Files: %d\n", info.total_files);
    rtos_printf("Directories: %d\n", info.total_directories);
    rtos_printf("Bad clusters: %d\n", info.bad_clusters);
    
    // Check for orphaned clusters
    int allocated_count = 0;
    for (int i = 0; i < fat_table.getSize(); i++) {
        if (fat_table.getConstRef(i).is_allocated && !fat_table.getConstRef(i).is_bad) {
            allocated_count++;
        }
    }
    
    rtos_printf("Allocated clusters: %d\n", allocated_count);
    
    if (allocated_count == (total_clusters - free_clusters - info.bad_clusters)) {
        rtos_printf("✓ Integrity check PASSED\n");
    } else {
        rtos_printf("✗ Integrity check FAILED: Cluster count mismatch!\n");
    }
}

// Check if a path is a directory
bool FATFileSystem::isDirectory(const RTOSString& path) const {
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
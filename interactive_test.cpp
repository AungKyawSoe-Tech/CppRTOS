#include "fat_file_system.h"
#include <iostream>
#include <string>
#include <vector>

using namespace std;

void printMenu() {
    cout << "\n=== FAT File System Interactive Test ===" << endl;
    cout << "1. Create file       7. Display directory tree" << endl;
    cout << "2. Delete file       8. Get file system info" << endl;
    cout << "3. Copy file         9. Run integrity check" << endl;
    cout << "4. Create directory  10. Create test structure" << endl;
    cout << "5. List directory    11. Delete folder" << endl;
    cout << "6. Display FAT table 12. Exit" << endl;
    cout << "========================================" << endl;
    cout << "Enter choice: ";
}

void interactiveTest() {
    cout << "Initializing FAT File System..." << endl;
    FATFileSystem fs(1024, 512, "TEST_FS");
    
    int choice;
    string input;
    
    while (true) {
        printMenu();
        cin >> choice;
        cin.ignore(); // Clear newline
        
        switch (choice) {
            case 1: {
                string filename;
                size_t size;
                
                cout << "Enter filename: ";
                getline(cin, filename);
                cout << "Enter file size (bytes): ";
                cin >> size;
                cin.ignore();
                
                if (fs.createFile(filename, size)) {
                    cout << "File created successfully" << endl;
                } else {
                    cout << "Failed to create file" << endl;
                }
                break;
            }
            
            case 2: {
                string filename;
                cout << "Enter filename to delete: ";
                getline(cin, filename);
                
                if (fs.deleteFile(filename)) {
                    cout << "File deleted successfully" << endl;
                } else {
                    cout << "Failed to delete file" << endl;
                }
                break;
            }
            
            case 3: {
                string source, dest;
                cout << "Enter source filename: ";
                getline(cin, source);
                cout << "Enter destination filename: ";
                getline(cin, dest);
                
                if (fs.copyFile(source, dest)) {
                    cout << "File copied successfully" << endl;
                } else {
                    cout << "Failed to copy file" << endl;
                }
                break;
            }
            
            case 4: {
                string dirname;
                cout << "Enter directory name: ";
                getline(cin, dirname);
                
                if (fs.createDirectory(dirname)) {
                    cout << "Directory created successfully" << endl;
                } else {
                    cout << "Failed to create directory" << endl;
                }
                break;
            }
            
            case 5: {
                auto entries = fs.listDirectory();
                cout << "\nDirectory listing:" << endl;
                cout << "=================" << endl;
                for (const auto& entry : entries) {
                    cout << (entry.is_dir ? "[DIR]  " : "[FILE] ") 
                         << entry.name;
                    if (!entry.is_dir) {
                        cout << " (" << entry.size << " bytes)";
                    }
                    cout << endl;
                }
                break;
            }
            
            case 6:
                fs.displayFAT();
                break;
                
            case 7:
                fs.displayDirectoryTree();
                break;
                
            case 8: {
                auto info = fs.getFileSystemInfo();
                cout << "\nFile System Information:" << endl;
                cout << "========================" << endl;
                cout << "Total space: " << info.total_space << " bytes" << endl;
                cout << "Used space: " << info.used_space << " bytes" << endl;
                cout << "Free space: " << info.free_space << " bytes" << endl;
                cout << "Files: " << info.total_files << endl;
                cout << "Directories: " << info.total_directories << endl;
                cout << "Bad clusters: " << info.bad_clusters << endl;
                break;
            }
                
            case 9:
                fs.runIntegrityCheck();
                break;
                
            case 10:
                fs.createTestStructure();
                cout << "Test structure created" << endl;
                break;
                
            case 11: {
                string dirname;
                cout << "Enter directory name to delete: ";
                getline(cin, dirname);
                
                if (fs.deleteDirectory(dirname)) {
                    cout << "Directory deleted successfully" << endl;
                } else {
                    cout << "Failed to delete directory" << endl;
                }
                break;
            }
                
            case 12:
                cout << "Exiting..." << endl;
                return;
                
            default:
                cout << "Invalid choice. Try again." << endl;
        }
    }
}

int main() {
    try {
        interactiveTest();
        return 0;
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
}
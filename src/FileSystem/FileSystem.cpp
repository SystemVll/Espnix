#include <Arduino.h>
#include <sstream>

#include <FileSystem/Folder.h>
#include <FileSystem/file.h>
#include <Utils/BootMessages.h>

#include "FileSystem.h"

FileSystem *FileSystem::instance = nullptr;

FileSystem::FileSystem()
{
    this->root = nullptr;
    this->sdMounted = false;
    this->inInitramfs = true;
    this->autoSync = true;  // Auto-sync enabled by default
    this->currentPath = "/";
}

void FileSystem::InitializeInitramfs()
{
    BootMessages::PrintInfo("Initializing initramfs (temporary root filesystem)");

    this->root = new espnix::Folder();
    this->root->parent = nullptr;
    this->root->name = "/";

    BootMessages::PrintOK("Initramfs mounted at /");
    BootMessages::PrintInfo("Creating initial filesystem structure");

    auto *etc = new espnix::Folder();
    etc->name = "etc";
    etc->permissions = 0755;
    etc->parent = this->root;
    this->root->AddFolder(etc);

    auto *sys = new espnix::Folder();
    sys->name = "sys";
    sys->permissions = 0755;
    sys->parent = this->root;
    this->root->AddFolder(sys);

    auto *tmp = new espnix::Folder();
    tmp->name = "tmp";
    tmp->permissions = 0777;
    tmp->parent = this->root;
    this->root->AddFolder(tmp);

    auto *home = new espnix::Folder();
    home->name = "root";
    home->permissions = 0755;
    home->parent = this->root;
    this->root->AddFolder(home);

    this->currentPath = "/root";
    this->inInitramfs = true;

    auto *readme = new espnix::File();
    readme->name = "README.md";
    readme->Write("Welcome to Espnix - a simple Unix-like system for ESP32\n");
    readme->permissions = 0644;
    this->root->AddFile(readme);

    BootMessages::PrintOK("Initramfs initialized (tmpfs mode)");
}

bool FileSystem::MountSDCard()
{
    BootMessages::PrintInfo("Attempting to mount real filesystem from SD card");

    if (!SD.begin())
    {
        BootMessages::PrintError("SD card initialization failed!");
        BootMessages::PrintWarn("No SD card detected or card is not properly inserted");
        BootMessages::PrintInfo("Continuing with initramfs only (no persistence)");
        BootMessages::PrintWarn("All changes will be lost on reboot!");
        return false;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE)
    {
        BootMessages::PrintError("No SD card attached!");
        BootMessages::PrintWarn("Please insert an SD card for persistent storage");
        BootMessages::PrintInfo("Continuing with initramfs only (no persistence)");
        BootMessages::PrintWarn("All changes will be lost on reboot!");
        return false;
    }

    std::string cardTypeStr;
    if (cardType == CARD_MMC)
        cardTypeStr = "MMC";
    else if (cardType == CARD_SD)
        cardTypeStr = "SDSC";
    else if (cardType == CARD_SDHC)
        cardTypeStr = "SDHC";
    else
        cardTypeStr = "UNKNOWN";

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    BootMessages::PrintOK("SD card detected: " + cardTypeStr + " (" + std::to_string(cardSize) + " MB)");

    this->sdMounted = true;

    BootMessages::PrintInfo("Total space: " + std::to_string(SD.totalBytes() / (1024 * 1024)) + " MB");
    BootMessages::PrintInfo("Free space: " + std::to_string((SD.totalBytes() - SD.usedBytes()) / (1024 * 1024)) + " MB");

    // Check if this is the first boot
    if (IsFirstBoot())
    {
        BootMessages::PrintInfo("First boot detected - initializing filesystem structure");
        CreateDefaultDirectories();
        MarkInitialized();
        BootMessages::PrintOK("Default directories created on SD card");
    }
    else
    {
        BootMessages::PrintInfo("Existing filesystem detected on SD card");
    }

    // Switch from initramfs to SD card
    this->inInitramfs = false;
    BootMessages::PrintInfo("Switching root filesystem to SD card");
    BootMessages::PrintOK("Root filesystem mounted from SD card (read/write)");

    return true;
}

void FileSystem::LoadDirectoryFromSD(const char *path, espnix::Folder *parent)
{
    fs::File dir = SD.open(path);
    if (!dir || !dir.isDirectory())
    {
        return;
    }

    fs::File entry = dir.openNextFile();
    while (entry)
    {
        std::string entryName = entry.name();
        size_t lastSlash = entryName.find_last_of('/');
        if (lastSlash != std::string::npos)
        {
            entryName = entryName.substr(lastSlash + 1);
        }

        if (entry.isDirectory())
        {
            bool exists = false;
            espnix::Folder *existingFolder = nullptr;

            for (auto *folder : parent->folders)
            {
                if (folder->name == entryName)
                {
                    exists = true;
                    existingFolder = folder;
                    break;
                }
            }

            if (!exists)
            {
                espnix::Folder *folder = new espnix::Folder();
                folder->name = entryName;
                folder->permissions = 0755;
                folder->parent = parent;
                parent->AddFolder(folder);
                existingFolder = folder;
            }

            std::string subPath = std::string(path) + "/" + entryName;
            LoadDirectoryFromSD(subPath.c_str(), existingFolder);
        }
        else
        {
            bool exists = false;
            for (auto *file : parent->files)
            {
                if (file->name == entryName)
                {
                    exists = true;
                    break;
                }
            }

            if (!exists)
            {
                espnix::File *file = new espnix::File();
                file->name = entryName;
                file->permissions = 0644;
                file->size = entry.size();

                std::string content;
                content.reserve(entry.size());
                while (entry.available())
                {
                    content += (char)entry.read();
                }
                file->Write(content);

                parent->AddFile(file);
            }
        }

        entry = dir.openNextFile();
    }
}

void FileSystem::LoadFromSD()
{
    if (!this->sdMounted)
    {
        return;
    }

    BootMessages::PrintInfo("Loading filesystem from SD card");
    LoadDirectoryFromSD("/", this->root);
    BootMessages::PrintOK("Filesystem mounted at / on SD card");
}

void FileSystem::SaveDirectoryToSD(const char *sdPath, espnix::Folder *folder)
{
    if (!this->sdMounted)
    {
        return;
    }

    for (auto *subFolder : folder->folders)
    {
        std::string newPath = std::string(sdPath) + "/" + subFolder->name;
        if (!SD.exists(newPath.c_str()))
        {
            SD.mkdir(newPath.c_str());
        }
        SaveDirectoryToSD(newPath.c_str(), subFolder);
    }

    for (auto *file : folder->files)
    {
        std::string filePath = std::string(sdPath) + "/" + file->name;

        // Delete existing file to ensure clean write
        if (SD.exists(filePath.c_str()))
        {
            SD.remove(filePath.c_str());
        }

        // Open file for writing
        fs::File sdFile = SD.open(filePath.c_str(), FILE_WRITE);
        if (sdFile)
        {
            // Write binary data (handles both text and binary files)
            if (file->content != nullptr && file->size > 0)
            {
                sdFile.write((const uint8_t*)file->content, file->size);
            }
            sdFile.close();
        }
    }
}

void FileSystem::SyncToSD()
{
    if (!this->sdMounted)
    {
        return;
    }

    SaveDirectoryToSD("/", this->root);
}

void FileSystem::SyncFileToSD(espnix::File *file, const std::string &path)
{
    if (!this->sdMounted || this->inInitramfs || !this->autoSync)
    {
        return;
    }

    if (file == nullptr || file->content == nullptr || file->size == 0)
    {
        return;
    }

    // Delete existing file to ensure clean write
    if (SD.exists(path.c_str()))
    {
        SD.remove(path.c_str());
    }

    // Write file to SD card
    fs::File sdFile = SD.open(path.c_str(), FILE_WRITE);
    if (sdFile)
    {
        sdFile.write((const uint8_t*)file->content, file->size);
        sdFile.close();
    }
}

void FileSystem::WriteFile(espnix::File *file, const std::string &data, const std::string &path)
{
    if (file == nullptr)
    {
        return;
    }

    // Write to in-memory file
    file->Write(data);

    // Auto-sync to SD card if enabled
    if (this->autoSync && this->sdMounted && !this->inInitramfs)
    {
        SyncFileToSD(file, path);
    }
}

FileSystem *FileSystem::GetInstance()
{
    if (instance == nullptr)
    {
        instance = new FileSystem();
    }
    return instance;
}

std::string FileSystem::GetStringPermissions(int permissions, std::string entryType)
{
    std::string str = std::to_string(permissions);
    while (str.length() < 3)
    {
        str = "0" + str;
    }

    std::string result = "";

    for (int i = 0; i < 3; i++)
    {
        switch (str[i])
        {
        case '0':
            result += "---";
            break;
        case '1':
            result += "--x";
            break;
        case '2':
            result += "-w-";
            break;
        case '3':
            result += "-wx";
            break;
        case '4':
            result += "r--";
            break;
        case '5':
            result += "r-x";
            break;
        case '6':
            result += "rw-";
            break;
        case '7':
            result += "rwx";
            break;
        }
    }

    if (entryType == "folder")
    {
        return "d" + result;
    }
    else
    {
        return "-" + result;
    }
}

espnix::Folder *FileSystem::GetFolder(std::string path)
{
    espnix::Folder *current = this->root;
    if (!path.empty() && path[0] != '/')
    {
        path = "/" + path;
    }

    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(path);

    while (std::getline(tokenStream, token, '/'))
    {
        if (token == "..")
        {
            if (current->parent != nullptr)
            {
                current = current->parent;
            }
        }
        else if (!token.empty() && token != ".")
        {
            bool found = false;
            for (auto *folder : current->folders)
            {
                if (folder->name == token)
                {
                    current = folder;
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                return nullptr;
            }
        }
    }

    return current;
}

espnix::File *FileSystem::GetFile(std::string path)
{
    if (path[0] != '/' && path != ".")
    {
        path = this->currentPath + "/" + path;
    }

    std::vector<std::string> components;
    std::stringstream ss(path);
    std::string item;
    while (getline(ss, item, '/'))
    {
        if (!item.empty() && item != ".")
        {
            if (item == "..")
            {
                if (!components.empty())
                {
                    components.pop_back();
                }
            }
            else
            {
                components.push_back(item);
            }
        }
    }

    std::string folderPath;
    for (size_t i = 0; i < components.size() - 1; ++i)
    {
        folderPath += "/" + components[i];
    }

    espnix::Folder *folder = this->GetFolder(folderPath.empty() ? "/" : folderPath);
    if (folder == nullptr)
    {
        return nullptr;
    }

    std::string fileName = components.back();

    for (auto &file : folder->files)
    {
        if (file->name == fileName)
        {
            return file;
        }
    }

    return nullptr;
}

bool FileSystem::FolderExists(std::string path)
{
    espnix::Folder *folder = this->GetFolder(path);
    return folder != nullptr;
}

bool FileSystem::IsFirstBoot()
{
    if (!this->sdMounted)
    {
        return false;
    }

    // Check if /sys/.init exists on SD card
    return !SD.exists("/sys/.init");
}

void FileSystem::CreateDefaultDirectories()
{
    if (!this->sdMounted)
    {
        BootMessages::PrintWarn("Cannot create default directories - SD card not mounted");
        return;
    }

    BootMessages::PrintInfo("Creating default directory structure on SD card");

    // Create default directories
    const char* defaultDirs[] = {
        "/bin",      // System binaries
        "/boot",     // Boot configuration
        "/dev",      // Device files
        "/etc",      // Configuration files
        "/home",     // User home directories
        "/lib",      // System libraries
        "/mnt",      // Mount points
        "/opt",      // Optional software
        "/proc",     // Process information
        "/root",     // Root user home
        "/run",      // Runtime data
        "/sbin",     // System binaries
        "/srv",      // Service data
        "/sys",      // System information
        "/tmp",      // Temporary files
        "/usr",      // User programs
        "/usr/bin",  // User binaries
        "/usr/lib",  // User libraries
        "/usr/local",// Local software
        "/var",      // Variable data
        "/var/log",  // Log files
        "/var/tmp"   // Temporary files
    };

    for (const char* dir : defaultDirs)
    {
        if (!SD.exists(dir))
        {
            if (SD.mkdir(dir))
            {
                BootMessages::PrintOK("Created: " + std::string(dir));
            }
            else
            {
                BootMessages::PrintWarn("Failed to create: " + std::string(dir));
            }
        }
    }

    // Create a welcome file in /root
    fs::File welcomeFile = SD.open("/root/README.txt", FILE_WRITE);
    if (welcomeFile)
    {
        welcomeFile.println("Welcome to Espnix!");
        welcomeFile.println("");
        welcomeFile.println("This is a Unix-like operating system for ESP32.");
        welcomeFile.println("Your files are stored on the SD card for persistence.");
        welcomeFile.println("");
        welcomeFile.println("System initialized on: October 26, 2025");
        welcomeFile.println("");
        welcomeFile.println("Available commands:");
        welcomeFile.println("  ls      - List directory contents");
        welcomeFile.println("  cd      - Change directory");
        welcomeFile.println("  cat     - Display file contents");
        welcomeFile.println("  echo    - Print text");
        welcomeFile.println("  mkdir   - Create directory");
        welcomeFile.println("  clear   - Clear screen");
        welcomeFile.println("  compile - Compile source code to bytecode");
        welcomeFile.println("  run     - Execute compiled bytecode");
        welcomeFile.println("");
        welcomeFile.println("For more information, check /etc/motd");
        welcomeFile.close();
        BootMessages::PrintOK("Created welcome file: /root/README.txt");
    }

    // Create message of the day
    fs::File motdFile = SD.open("/etc/motd", FILE_WRITE);
    if (motdFile)
    {
        motdFile.println("╔══════════════════════════════════════════════════╗");
        motdFile.println("║            Welcome to Espnix v1.0                 ║");
        motdFile.println("║         Unix-like OS for ESP32                    ║");
        motdFile.println("╚══════════════════════════════════════════════════╝");
        motdFile.println("");
        motdFile.println("Type 'help' for available commands.");
        motdFile.close();
        BootMessages::PrintOK("Created: /etc/motd");
    }

    BootMessages::PrintOK("Default filesystem structure created");
}

void FileSystem::MarkInitialized()
{
    if (!this->sdMounted)
    {
        return;
    }

    // Create the .init marker file in /sys
    fs::File initFile = SD.open("/sys/.init", FILE_WRITE);
    if (initFile)
    {
        initFile.println("# Espnix initialization marker");
        initFile.println("# Created: October 26, 2025");
        initFile.println("# This file indicates that the filesystem has been initialized.");
        initFile.println("# Do not delete this file unless you want to reinitialize the system.");
        initFile.close();
        BootMessages::PrintOK("Created initialization marker: /sys/.init");
    }
    else
    {
        BootMessages::PrintWarn("Failed to create initialization marker");
    }
}


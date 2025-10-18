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

    espnix::File *readme = new espnix::File();
    readme->name = "README.md";
    readme->Write("Welcome to Espnix - a simple Unix-like system for ESP32\n");
    readme->permissions = 0644;
    this->root->AddFile(readme);

    espnix::Folder *etc = new espnix::Folder();
    etc->name = "etc";
    etc->permissions = 0755;
    etc->parent = this->root;
    this->root->AddFolder(etc);

    espnix::Folder *tmp = new espnix::Folder();
    tmp->name = "tmp";
    tmp->permissions = 0777;
    tmp->parent = this->root;
    this->root->AddFolder(tmp);

    espnix::Folder *home = new espnix::Folder();
    home->name = "home";
    home->permissions = 0755;
    home->parent = this->root;
    this->root->AddFolder(home);

    this->currentPath = "/";
    this->inInitramfs = true;

    BootMessages::PrintOK("Initramfs initialized (tmpfs mode)");
}

bool FileSystem::MountSDCard()
{
    BootMessages::PrintInfo("Attempting to mount real filesystem from SD card");

    if (!SD.begin())
    {
        BootMessages::PrintWarn("SD card initialization failed");
        BootMessages::PrintInfo("Continuing with initramfs only (no persistence)");
        return false;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE)
    {
        BootMessages::PrintWarn("No SD card attached");
        BootMessages::PrintInfo("Continuing with initramfs only (no persistence)");
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
    this->inInitramfs = false;

    BootMessages::PrintInfo("Switching root filesystem to SD card");
    BootMessages::PrintOK("Root filesystem mounted from SD card (read/write)");
    BootMessages::PrintInfo("Total space: " + std::to_string(SD.totalBytes() / (1024 * 1024)) + " MB");
    BootMessages::PrintInfo("Free space: " + std::to_string((SD.totalBytes() - SD.usedBytes()) / (1024 * 1024)) + " MB");

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

    if (SD.exists("/espnix"))
    {
        LoadDirectoryFromSD("/espnix", this->root);
        BootMessages::PrintOK("Filesystem loaded from /espnix on SD card");
    }
    else
    {
        SD.mkdir("/espnix");
        BootMessages::PrintInfo("Created /espnix directory on SD card");
    }

    BootMessages::PrintOK("SD card ready for persistence");
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
        fs::File sdFile = SD.open(filePath.c_str(), FILE_WRITE);
        if (sdFile)
        {
            sdFile.print(file->Read().c_str());
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

    SaveDirectoryToSD("/espnix", this->root);
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

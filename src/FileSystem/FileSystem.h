#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <vector>
#include <string>
#include <SD.h>

namespace espnix
{
    class Folder;
    class File;
}

class FileSystem
{
private:
    static FileSystem *instance;
    FileSystem();

    void LoadDirectoryFromSD(const char *path, espnix::Folder *parent);
    void SaveDirectoryToSD(const char *sdPath, espnix::Folder *folder);

public:
    std::string currentPath;
    espnix::Folder *root;
    bool sdMounted;
    bool inInitramfs;

    FileSystem(const FileSystem &) = delete;
    FileSystem &operator=(const FileSystem &) = delete;

    static FileSystem *GetInstance();
    void InitializeInitramfs();
    bool MountSDCard();
    void LoadFromSD();
    void SyncToSD();
    bool IsFirstBoot();
    void CreateDefaultDirectories();
    void MarkInitialized();
    std::string GetStringPermissions(int permissions, std::string entryType);
    espnix::File *GetFile(std::string path);
    espnix::Folder *GetFolder(std::string path);
    bool FolderExists(std::string path);
};

#endif
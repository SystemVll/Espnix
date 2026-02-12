#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <string>
#include <sys/_default_fcntl.h>

namespace espnix
{
    class Folder;
    class File;
}

class FileDescriptor;

class FileSystem
{
    static FileSystem *instance;
    FileSystem();

    void LoadDirectoryFromSD(const char *path, espnix::Folder *parent);
    void SaveDirectoryToSD(const char *sdPath, espnix::Folder *folder);

public:
    std::string currentPath;
    espnix::Folder *root;
    bool sdMounted;
    bool inInitramfs;
    bool autoSync;  // Auto-sync files to SD card on write

    FileSystem(const FileSystem &) = delete;
    FileSystem &operator=(const FileSystem &) = delete;

    static FileSystem *GetInstance();
    void InitializeInitramfs();
    bool MountSDCard();
    void LoadFromSD();
    void SyncToSD();

    int SyncFileToSD(espnix::File *file, const std::string &path);
    void WriteFile(espnix::File *file, const std::string &data, const std::string &path);
    espnix::File* CreateFile(const std::string &path, int permissions = 0644);
    FileDescriptor* OpenFile(const std::string &path, int flags = O_RDWR);
    bool IsFirstBoot();
    void CreateDefaultDirectories();
    void MarkInitialized();
    std::string GetStringPermissions(int permissions, std::string entryType);
    espnix::File *GetFile(std::string path);
    espnix::Folder *GetFolder(std::string path);
    bool FolderExists(std::string path);
};

#endif
#ifndef FOLDER_H
#define FOLDER_H

#include <string>
#include <vector>

#include "FileSystemEntity.h"

namespace espnix
{
    class File;
    class Folder;

    class Folder : public FileSystemEntity
    {
    public:
        std::vector<File *> files;
        std::vector<Folder *> folders;
        std::vector<FileSystemEntity *> entities;  // Unified collection
        Folder *current{};

        Folder();
        void AddFile(File *file);
        void AddFolder(Folder *folder);
        std::vector<void *> ListContent();
        void RemoveFile(std::string filename);
        void RemoveFolder(std::string foldername);

        // Virtual method implementations
        std::string GetDisplayName() const override;
        size_t GetSize() const override;

        ~Folder();
    };
}

#endif
#include <Arduino.h>

#include "Folder.h"
#include "File.h"

namespace espnix
{
    Folder::Folder()
    {
        this->permissions = 0777;
    }

    void Folder::AddFile(File *file)
    {
        file->creationDate = time(0);
        this->files.push_back(file);
    }

    void Folder::AddFolder(Folder *folder)
    {
        folder->creationDate = time(0);
        this->folders.push_back(folder);
    }

    std::vector<void *> Folder::ListContent()
    {
        std::vector<void *> contents;

        for (auto &file : this->files)
        {
            contents.push_back(static_cast<void *>(&file));
        }

        for (auto &folder : this->folders)
        {
            contents.push_back(static_cast<void *>(&folder));
        }

        return contents;
    }

    void Folder::RemoveFile(std::string filename)
    {
        for (size_t i = 0; i < this->files.size(); i++)
        {
            if (this->files[i]->name == filename)
            {
                delete this->files[i];

                this->files.erase(this->files.begin() + i);
                break;
            }
        }
    }

    void Folder::RemoveFolder(std::string foldername)
    {
        for (size_t i = 0; i < this->folders.size(); i++)
        {
            if (this->folders[i]->name == foldername)
            {
                delete this->folders[i];

                this->folders.erase(this->folders.begin() + i);
                break;
            }
        }
    }

    Folder::~Folder()
    {
        for (const auto &file : this->files)
        {
            delete file;
        }

        for (const auto &folder : this->folders)
        {
            delete folder;
        }
    }
}
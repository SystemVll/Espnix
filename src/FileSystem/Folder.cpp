#include <Arduino.h>
#include <algorithm>
#include <time.h>

#include "Folder.h"
#include "File.h"

namespace espnix
{
    Folder::Folder() : FileSystemEntity(EntityType::FOLDER)
    {
        this->permissions = 0755;
        this->files.clear();
        this->folders.clear();
        this->entities.clear();
    }

    void Folder::AddFile(File *file)
    {
        file->creationDate = time(0);
        file->parent = this;
        this->files.push_back(file);
        this->entities.push_back(static_cast<FileSystemEntity*>(file));
    }

    void Folder::AddFolder(Folder *folder)
    {
        folder->creationDate = time(0);
        folder->parent = this;
        this->folders.push_back(folder);
        this->entities.push_back(static_cast<FileSystemEntity*>(folder));
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
                // Remove from entities collection first
                auto entityIt = std::find(this->entities.begin(), this->entities.end(),
                                        static_cast<FileSystemEntity*>(this->files[i]));
                if (entityIt != this->entities.end())
                {
                    this->entities.erase(entityIt);
                }

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
                // Remove from entities collection first
                auto entityIt = std::find(this->entities.begin(), this->entities.end(),
                                        static_cast<FileSystemEntity*>(this->folders[i]));
                if (entityIt != this->entities.end())
                {
                    this->entities.erase(entityIt);
                }

                delete this->folders[i];
                this->folders.erase(this->folders.begin() + i);
                break;
            }
        }
    }

    std::string Folder::GetDisplayName() const
    {
        return this->name + "/";
    }

    size_t Folder::GetSize() const
    {
        size_t totalSize = 0;
        for (const auto& file : this->files)
        {
            totalSize += file->GetSize();
        }
        for (const auto& folder : this->folders)
        {
            totalSize += folder->GetSize();
        }
        return totalSize;
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

        // entities vector will automatically clean up its pointers
        // but we don't delete them again since they're already deleted above
        this->entities.clear();
    }
}
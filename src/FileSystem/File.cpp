#include <string>
#include <cstring>
#include <vector>
#include <Arduino.h>
#include <SD.h>

#include "File.h"
#include "Folder.h"
#include "FileSystem.h"

namespace espnix
{
    File::File() : FileSystemEntity(EntityType::FILE)
    {
        this->permissions = 0644;
        this->fd = nullptr;
    }

    std::string File::Read()
    {
        if (!this->fd)
        {
            // Auto-open for reading if not already open
            this->fd = Open(O_RDONLY);
            if (!this->fd)
            {
                return "";
            }
        }

        // If FileDescriptor has buffer content, return it
        if (!this->fd->buffer.empty())
        {
            return this->fd->buffer;
        }

        // Read from SD card via FileDescriptor
        if (this->fd->sdFile && this->fd->sdFile.available())
        {
            std::string content;
            this->fd->sdFile.seek(0);  // Reset to beginning
            while (this->fd->sdFile.available())
            {
                content += (char)this->fd->sdFile.read();
            }
            this->fd->buffer = content;  // Cache in buffer
            return content;
        }

        return "";
    }

    void File::Append(std::string data)
    {
        if (!this->fd)
        {
            // Auto-open for appending if not already open
            this->fd = Open(O_APPEND | O_CREAT);
            if (!this->fd)
            {
                return;
            }
        }

        // Write data using FileDescriptor
        this->fd->write(data.c_str(), data.length());
    }

    void File::Write(std::string data)
    {
        if (!this->fd)
        {
            // Auto-open for writing if not already open
            this->fd = Open(O_WRONLY | O_CREAT);
            if (!this->fd)
            {
                return;
            }
        }

        // Clear the buffer and write new data
        this->fd->clearBuffer();
        this->fd->write(data.c_str(), data.length());
    }

    FileDescriptor* File::Open(int flags)
    {
        if (this->fd)
        {
            return this->fd;  // Already open
        }

        // Construct the full path to the file
        std::string fullPath;

        if (this->parent)
        {
            // Build path from parent hierarchy
            Folder* currentFolder = this->parent;
            std::vector<std::string> pathComponents;

            while (currentFolder && currentFolder->parent)
            {
                pathComponents.push_back(currentFolder->name);
                currentFolder = currentFolder->parent;
            }

            // Reverse to get correct path order
            for (int i = pathComponents.size() - 1; i >= 0; i--)
            {
                fullPath += "/" + pathComponents[i];
            }
        }
        fullPath += "/" + this->name;

        // Create FileDescriptor for this file
        this->fd = new FileDescriptor(this, fullPath, flags);

        return this->fd;
    }

    void File::Close()
    {
        if (this->fd)
        {
            this->fd->close();
            delete this->fd;
            this->fd = nullptr;
        }
    }

    void File::Sync()
    {
        if (this->fd)
        {
            this->fd->flush();
        }
    }

    std::string File::GetDisplayName() const
    {
        return this->name;
    }

    size_t File::GetSize() const
    {
        if (this->fd)
        {
            return this->fd->bufferSize();
        }

        // If no FileDescriptor, try to read file size from SD card
        if (this->parent)
        {
            std::string fullPath = "";
            Folder* currentFolder = this->parent;
            std::vector<std::string> pathComponents;

            while (currentFolder && currentFolder->parent)
            {
                pathComponents.push_back(currentFolder->name);
                currentFolder = currentFolder->parent;
            }

            for (int i = pathComponents.size() - 1; i >= 0; i--)
            {
                fullPath += "/" + pathComponents[i];
            }
            fullPath += "/" + this->name;

            if (SD.exists(fullPath.c_str()))
            {
                fs::File sdFile = SD.open(fullPath.c_str(), FILE_READ);
                if (sdFile)
                {
                    size_t fileSize = sdFile.size();
                    sdFile.close();
                    return fileSize;
                }
            }
        }

        return 0;
    }

    File::~File()
    {
        if (this->fd)
        {
            Close();
        }
    }
}
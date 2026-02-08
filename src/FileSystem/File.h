#ifndef FILE_H
#define FILE_H

#include <string>
#include <fcntl.h>

#include "FileSystemEntity.h"
#include "IO/FileDescriptor.h"

namespace espnix
{
    class Folder;

    class File : public FileSystemEntity
    {
    public:
        FileDescriptor *fd;

        File();
        std::string Read();
        void Append(std::string data);
        void Write(std::string data);

        // FileDescriptor operations
        FileDescriptor* Open(int flags = O_RDWR);
        void Close();
        void Sync();  // Sync to SD card

        // Virtual method implementations
        std::string GetDisplayName() const override;
        size_t GetSize() const override;

        ~File();
    };
}

#endif
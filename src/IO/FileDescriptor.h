#ifndef FILE_DESCRIPTOR_IO_H
#define FILE_DESCRIPTOR_IO_H

#include <vector>
#include <string>
#include <SD.h>
#include <sys/_default_fcntl.h>

namespace espnix {
    class File;
}

enum class FDType {
    STANDARD,    // stdin, stdout, stderr
    FILE,        // Regular file
    DEVICE       // Device file
};

class FileDescriptor
{
public:
    int descriptor;
    static std::vector<FileDescriptor*> dtable;

    FDType type;
    std::string buffer;
    std::string filePath;  // Path for file descriptors
    espnix::File* file;    // Associated file object
    File sdFile;       // SD card file handle
    bool isOpen;
    int flags;             // Open flags (read/write/append)

    FileDescriptor(int descriptor);
    FileDescriptor(espnix::File* file, const std::string& path, int flags = O_RDWR);

    ssize_t read(void *buffer, size_t count, size_t nmemb = 1);
    ssize_t write(const void *buffer, size_t count);

    bool open();
    void close();
    void flush();

    // Buffer management
    void clearBuffer();
    size_t bufferSize() const;

    FileDescriptor(const FileDescriptor &) = delete;
    FileDescriptor &operator=(const FileDescriptor &) = delete;

    ~FileDescriptor();
};

#endif
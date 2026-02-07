#ifndef FILE_DESCRIPTOR_IO_H
#define FILE_DESCRIPTOR_IO_H

#include <fcntl.h>

class FileDescriptor
{
public:
    FileDescriptor(int descriptor);

    ssize_t read(void *buffer, size_t count);

    ssize_t write(const void *buffer, size_t count);

    FileDescriptor(const FileDescriptor &) = delete;
    FileDescriptor &operator=(const FileDescriptor &) = delete;

private:
    int descriptor;
};

#endif
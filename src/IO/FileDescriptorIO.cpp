#include "FileDescriptorIO.h"

FileDescriptor::FileDescriptor(int descriptor)
{
    if (descriptor != 0 && descriptor != 1 && descriptor != 2)
    {
        throw std::invalid_argument("Invalid standard file descriptor");
    }

    this->descriptor = descriptor;
}

ssize_t FileDescriptor::read(void *buffer, size_t count)
{
    std::copy(data.begin(), data.end(), (char *)buffer);
    return data.length();
}

ssize_t FileDescriptor::write(const void *buffer, size_t count)
{
    this->data = std::string((char *)buffer, count);
    return count;
}
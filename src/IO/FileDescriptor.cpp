#include <Arduino.h>

#include "FileDescriptor.h"

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
    if (count == 0) return 0;

    // For character-by-character reading
    if (count == 1 && Serial.available() > 0)
    {
        char *charBuffer = static_cast<char *>(buffer);
        *charBuffer = Serial.read();
        return 1;
    }
    else if (count == 1)
    {
        // No data available
        return 0;
    }

    // For larger reads, use the original method
    size_t bytesRead = Serial.readBytes(static_cast<char *>(buffer), count);
    return static_cast<ssize_t>(bytesRead);
}

ssize_t FileDescriptor::write(const void *buffer, size_t count)
{
    Serial.write(static_cast<const char *>(buffer), count);
    return count;
}
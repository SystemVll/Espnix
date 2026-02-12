#include <Arduino.h>
#include <vector>
#include <stdexcept>
#include <algorithm>

#include "FileDescriptor.h"
#include "FileSystem/File.h"

std::vector<FileDescriptor*> FileDescriptor::dtable;

FileDescriptor::FileDescriptor(int descriptor)
{
    if (descriptor < -1)
    {
        throw std::invalid_argument("Invalid standard file descriptor");
    }

    this->type = FDType::STANDARD;
    this->file = nullptr;
    this->isOpen = true;
    this->flags = O_RDWR;

    if (descriptor == -1) {
        // This is a file descriptor for a file asking for an id, assign the next available id
        this->dtable.push_back(this);
        descriptor = this->dtable.size() - 1;
    }

    this->descriptor = descriptor;
}

FileDescriptor::FileDescriptor(espnix::File* file, const std::string& path, int flags)
{
    this->type = FDType::FILE;
    this->file = file;
    this->filePath = path;
    this->flags = flags;
    this->isOpen = false;

    // Add to descriptor table
    this->dtable.push_back(this);
    this->descriptor = this->dtable.size() - 1;

    // Attempt to open the file
    open();
}

bool FileDescriptor::open()
{
    if (this->type == FDType::STANDARD)
    {
        this->isOpen = true;
        return true;
    }

    if (this->type == FDType::FILE && !this->filePath.empty())
    {
        const char* mode = FILE_READ;
        if (this->flags & O_WRONLY)
            mode = FILE_WRITE;
        else if (this->flags & O_RDWR)
            mode = FILE_WRITE;  // SD library uses FILE_WRITE for read/write
        else if (this->flags & O_APPEND)
            mode = FILE_APPEND;

        this->sdFile = SD.open(this->filePath.c_str(), mode);
        this->isOpen = this->sdFile ? true : false;

        // If opening for read and file exists, load content into buffer
        if (this->isOpen && (this->flags & O_RDONLY || this->flags & O_RDWR))
        {
            this->buffer.clear();
            while (this->sdFile.available())
            {
                this->buffer += (char)this->sdFile.read();
            }
            // Reset file position for read operations
            this->sdFile.seek(0);
        }

        return this->isOpen;
    }

    return false;
}

void FileDescriptor::close()
{
    if (this->type == FDType::FILE && this->sdFile)
    {
        flush();
        this->sdFile.close();
    }

    this->isOpen = false;
}

void FileDescriptor::flush()
{
    if (this->type == FDType::FILE && this->isOpen && this->sdFile)
    {
        this->sdFile.flush();

        // Note: File object no longer stores content directly
        // All content is managed through the FileDescriptor buffer
        // and synchronized to SD card automatically
    }
}

ssize_t FileDescriptor::read(void *buffer, size_t count, size_t nmemb)
{
    if (!this->isOpen)
        return -1;

    if (this->type == FDType::STANDARD)
    {
        if (count == 0) return 0;

        // For character-by-character reading
        if (count == 1 && Serial.available() > 0)
        {
            char *charBuffer = static_cast<char *>(buffer);
            *charBuffer = Serial.read();
            return 1;
        } else if (count == 1) {
            // No data available
            return 0;
        }

        // For larger reads, use the original method
        size_t bytesRead = Serial.readBytes(static_cast<char *>(buffer), count);
        return static_cast<ssize_t>(bytesRead);
    }
    else if (this->type == FDType::FILE)
    {
        if (!this->sdFile)
            return -1;

        size_t totalBytes = count * nmemb;
        size_t bytesRead = 0;
        char* charBuffer = static_cast<char*>(buffer);

        // Read from SD file
        while (bytesRead < totalBytes && this->sdFile.available())
        {
            charBuffer[bytesRead] = this->sdFile.read();
            bytesRead++;
        }

        return static_cast<ssize_t>(bytesRead);
    }

    return -1;
}

ssize_t FileDescriptor::write(const void *buffer, size_t count)
{
    if (!this->isOpen)
        return -1;

    if (this->type == FDType::STANDARD)
    {
        const char* charBuffer = static_cast<const char*>(buffer);

        size_t written = Serial.write(reinterpret_cast<const uint8_t*>(charBuffer), count);

        this->buffer.append(charBuffer, count);

        return static_cast<ssize_t>(written);
    }

    if (this->type == FDType::FILE)
    {
        if (!(this->flags & (O_WRONLY | O_RDWR | O_APPEND)))
            return -1;  // File not open for writing

        const char* charBuffer = static_cast<const char*>(buffer);
        std::string data(charBuffer, count);

        // Add to internal buffer
        if (this->flags & O_APPEND)
        {
            this->buffer += data;
        }
        else
        {
            // For write mode, replace buffer content
            this->buffer = data;
        }

        // Write to SD file if available
        if (this->sdFile)
        {
            size_t written = this->sdFile.write(static_cast<const uint8_t*>(buffer), count);
            this->sdFile.flush();

            return static_cast<ssize_t>(written);
        }

        return count;
    }

    return -1;
}

void FileDescriptor::clearBuffer()
{
    this->buffer.clear();
}

size_t FileDescriptor::bufferSize() const
{
    return this->buffer.size();
}

FileDescriptor::~FileDescriptor()
{
    close();

    // Remove from descriptor table
    auto it = std::find(dtable.begin(), dtable.end(), this);
    if (it != dtable.end())
    {
        dtable.erase(it);
    }
}

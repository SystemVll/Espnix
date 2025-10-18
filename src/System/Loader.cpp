#include "Loader.h"
#include <Terminal/Terminal.h>
#include <Utils/BootMessages.h>
#include <esp_partition.h>
#include <esp_spi_flash.h>
#include <esp_heap_caps.h>
#include <esp_cpu.h>

#define ESPNIX_MAGIC 0x45535058 // "ESPX"

typedef int (*EntryPointFunc)(void *context);

Loader::Loader(Terminal *term) : terminal(term)
{
    BootMessages::PrintInfo("Initializing binary loader");

    execMemory = (uint8_t *)heap_caps_malloc(MAX_BINARY_SIZE, MALLOC_CAP_EXEC);
    if (!execMemory)
    {
        BootMessages::PrintFail("Failed to allocate execution memory");
        terminal->Write("Failed to allocate execution memory!\n");
    }
    else
    {
        BootMessages::PrintOK("Allocated " + std::to_string(MAX_BINARY_SIZE / 1024) + "KB executable memory");
    }

    execContext = new ExecutionContext();
    execContext->terminal = terminal;
    execContext->loader = this;

    BootMessages::PrintOK("Binary loader ready");
}

Loader::~Loader()
{
    if (execMemory)
    {
        heap_caps_free(execMemory);
    }

    if (execContext)
    {
        delete execContext;
    }
}

bool Loader::verifyHeader(const BinaryHeader &header)
{
    // Check if the binary has the correct magic number
    if (header.magic != ESPNIX_MAGIC)
    {
        terminal->Write("Invalid binary format (wrong magic number)\n");
        return false;
    }

    // Check version
    if (header.version > 1)
    {
        terminal->Write("Unsupported binary version\n");
        return false;
    }

    // Check if it will fit in our memory
    if (header.dataSize + header.bssSize > MAX_BINARY_SIZE)
    {
        terminal->Write("Binary too large to fit in memory\n");
        return false;
    }

    return true;
}

uint32_t Loader::allocateMemory(uint32_t size)
{
    // Simplified memory allocation - in a real system you'd do proper memory management
    static uint32_t nextOffset = 0;

    if (nextOffset + size > MAX_BINARY_SIZE)
    {
        terminal->Write("Out of memory for binary allocation\n");
        return 0;
    }

    uint32_t allocatedOffset = nextOffset;
    nextOffset += size;

    // Align to 4-byte boundary
    nextOffset = (nextOffset + 3) & ~3;

    return allocatedOffset;
}

bool Loader::LoadAndExecute(const std::string &path)
{
    // Check if SD is available
    if (!SD.exists(path.c_str()))
    {
        terminal->Write("Binary file not found: " + path + "\n");
        return false;
    }

    File binaryFile = SD.open(path.c_str());
    if (!binaryFile)
    {
        terminal->Write("Failed to open binary file\n");
        return false;
    }

    // Read header
    BinaryHeader header;
    if (binaryFile.read((uint8_t *)&header, sizeof(BinaryHeader)) != sizeof(BinaryHeader))
    {
        terminal->Write("Failed to read binary header\n");
        binaryFile.close();
        return false;
    }

    if (!verifyHeader(header))
    {
        binaryFile.close();
        return false;
    }

    uint32_t memOffset = allocateMemory(header.dataSize + header.bssSize);
    if (memOffset == 0)
    {
        binaryFile.close();
        return false;
    }

    if (binaryFile.read(execMemory + memOffset, header.dataSize) != header.dataSize)
    {
        terminal->Write("Failed to read binary data\n");
        binaryFile.close();
        return false;
    }

    // Initialize BSS section with zeros
    memset(execMemory + memOffset + header.dataSize, 0, header.bssSize);

    binaryFile.close();

    terminal->Write("Loaded binary: " + std::string(header.name) + "\n");

    struct Process proc;
    proc.name = header.name;
    proc.memoryOffset = memOffset;
    proc.isRunning = true;
    processes.push_back(proc);

    EntryPointFunc entryPoint = (EntryPointFunc)(execMemory + memOffset + header.entryPoint);

    terminal->Write("Executing binary...\n");

    // esp_cpu_inval_icache_range((uint32_t)execMemory + memOffset, header.dataSize);

    int result = entryPoint(execContext);

    terminal->Write("Binary execution completed with result code: " + std::to_string(result) + "\n");

    return true;
}

void Loader::Process()
{
    // for (auto &proc : processes)
    //{
    //
    // }

    delay(1);
}

int Loader::ExecuteSysCall(int callNumber, void *params)
{
    switch (callNumber)
    {
    case SYS_WRITE:
        terminal->Write(*(std::string *)params);
        return 0;

    case SYS_READ:
    {
        if (!Serial.available())
        {
            return -1; // Nothing to read
        }

        ReadParams *readParams = (ReadParams *)params;
        String input = Serial.readStringUntil('\n');
        readParams->buffer = input.c_str();
        return input.length();
    }

    case SYS_FILE_OPEN:
    {
        FileParams *fileParams = (FileParams *)params;
        File file = SD.open(fileParams->path, fileParams->mode == 0 ? FILE_READ : FILE_WRITE);
        if (!file)
        {
            return -1;
        }
        // Store the file and return a handle
        int handle = nextFileHandle++;
        openFiles[handle] = file;
        return handle;
    }

    case SYS_FILE_READ:
    {
        FileIOParams *ioParams = (FileIOParams *)params;
        if (openFiles.find(ioParams->handle) == openFiles.end())
        {
            return -1;
        }
        // return openFiles[ioParams->handle].read(ioParams->buffer, ioParams->size);
    }

    case SYS_FILE_WRITE:
    {
        FileIOParams *ioParams = (FileIOParams *)params;
        if (openFiles.find(ioParams->handle) == openFiles.end())
        {
            return -1;
        }
        // return openFiles[ioParams->handle].write(ioParams->buffer, ioParams->size);
    }

    case SYS_FILE_CLOSE:
    {
        int handle = *(int *)params;
        if (openFiles.find(handle) == openFiles.end())
        {
            return -1;
        }
        openFiles[handle].close();
        openFiles.erase(handle);
        return 0;
    }

    default:
        terminal->Write("Unknown system call: " + std::to_string(callNumber) + "\n");
        return -1;
    }
}
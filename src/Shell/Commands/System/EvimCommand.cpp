#include <FileSystem/File.h>
#include <FileSystem/FileSystem.h>
#include <FileSystem/Folder.h>
#include <Terminal/Terminal.h>
#include <cstring>

#include "EvimCommand.h"

#include "IO/FileDescriptor.h"

void EvimCommand::Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output)
{
    if (args.size() != 1)
    {
        terminal->Write("Usage: evim <file>\n");
        return;
    }

    FileSystem *fileSystem = FileSystem::GetInstance();
    std::string filePath = args[0];
    espnix::File *file = fileSystem->GetFile(filePath);

    // If file doesn't exist, create it
    if (file == nullptr)
    {
        // Create a new file
        file = new espnix::File();
        file->name = filePath;
        file->content = nullptr;
        file->size = 0;
        file->permissions = 0644;
        file->owner = 0;

        // Find the parent directory and add the file
        // For simplicity, assume files are created in current directory
        espnix::Folder *currentFolder = fileSystem->GetFolder(fileSystem->currentPath);
        if (currentFolder == nullptr)
        {
            terminal->Write("nano: cannot create " + filePath + ": Invalid current directory\n");
            delete file;
            return;
        }

        currentFolder->AddFile(file);
        terminal->Write("nano: " + filePath + ": New file\n");
    }

    // Display current file content if it exists
    std::string currentContent = file->Read();
    if (!currentContent.empty())
    {
        terminal->Write("Current content:\n");
        terminal->Write(currentContent);
        terminal->Write("\n");
    }

    terminal->Write("Editing " + filePath + " (type ':wq' to save and exit, ':q' to exit without saving)\n");
    terminal->Write("Enter your text:\n");

    std::string newContent = "";
    char buffer[256];

    while (true)
    {
        // Clear buffer
        memset(buffer, 0, sizeof(buffer));

        // Read input
        ssize_t bytesRead = input->read(buffer, sizeof(buffer) - 1);

        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0';
            std::string line(buffer);

            // Remove any trailing newlines for command checking
            std::string trimmedLine = line;
            while (!trimmedLine.empty() && (trimmedLine.back() == '\n' || trimmedLine.back() == '\r'))
            {
                trimmedLine.pop_back();
            }

            // Check for commands
            if (trimmedLine == ":wq")
            {
                // Save and quit
                file->Write(newContent);
                terminal->Write("File saved successfully.\n");
                break;
            }

            if (trimmedLine == ":q")
            {
                // Quit without saving
                terminal->Write("File not saved.\n");
                break;
            }

            // Add content
            newContent += line;
        }
    }
}

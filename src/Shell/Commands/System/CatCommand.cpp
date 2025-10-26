#include <sstream>
#include <string.h>

#include <FileSystem/Folder.h>
#include <FileSystem/File.h>
#include <FileSystem/FileSystem.h>
#include <Terminal/Terminal.h>

#include "CatCommand.h"

void CatCommand::Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output)
{
    if (args.size() != 1)
    {
        terminal->Write("Usage: cat <file>\n");
        return;
    }

    FileSystem *fileSystem = FileSystem::GetInstance();
    std::string filePath = args[0];
    espnix::File *file = fileSystem->GetFile(filePath);

    if (file == nullptr)
    {
        terminal->Write("cat: " + filePath + ": No such file or directory\n");
        return;
    }

    terminal->Write(file->Read());
}
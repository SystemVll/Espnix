#include <FileSystem/File.h>
#include <FileSystem/FileSystem.h>
#include <Terminal/Terminal.h>
#include <IO/FileDescriptor.h>

#include "CatCommand.h"

void CatCommand::Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output)
{
    input->open();

    if (args.size() != 1)
    {
        const std::string msg = "Usage: cat <file>\n";
        output->write(msg.c_str(), msg.size());
        return;
    }

    FileSystem *fileSystem = FileSystem::GetInstance();
    const std::string& filePath = args[0];
    espnix::File *file = fileSystem->GetFile(filePath);

    if (file == nullptr)
    {
        const std::string errorMsg = "cat: " + filePath + ": No such file or directory\n";

        output->write(errorMsg.c_str(), errorMsg.size());
        return;
    }

    const std::string content = file->Read();

    output->write(content.c_str(), content.size());
}
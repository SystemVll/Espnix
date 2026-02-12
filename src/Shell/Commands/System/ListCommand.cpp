#include <set>
#include <string>

#include <FileSystem/Folder.h>
#include <FileSystem/File.h>
#include <FileSystem/FileSystem.h>
#include <Terminal/Terminal.h>
#include <Utils/Utils.h>
#include <IO/FileDescriptor.h>

#include "ListCommand.h"

void ListCommand::Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output)
{
    std::set<std::string> flags(args.begin(), args.end());
    FileSystem *fileSystem = FileSystem::GetInstance();
    const espnix::Folder *folder = fileSystem->GetFolder(fileSystem->currentPath);

    if (flags.find("-a") != flags.end() || flags.find("--all") != flags.end())
    {
        for (const espnix::Folder *subFolder : folder->folders)
        {
            std::string folderName = subFolder->name + " ";
            output->write(folderName.c_str(), folderName.size());
        }

        for (const espnix::File *file : folder->files)
        {
            std::string fileName = file->name + " ";
            output->write(fileName.c_str(), fileName.size());
        }

        output->write("\n", 1);
        return;
    }

    if (flags.find("-l") != flags.end())
    {
        for (const espnix::Folder *subFolder : folder->folders)
        {
            std::string permissions = fileSystem->GetStringPermissions(subFolder->permissions, "folder");
            std::string line = permissions + " root root 4096 " + Utils::FormatDate(subFolder->creationDate) + " " + subFolder->name + "\n";
            output->write(line.c_str(), line.size());
        }

        for (const espnix::File *file : folder->files)
        {
            std::string permissions = fileSystem->GetStringPermissions(file->permissions, "file");
            std::string line = permissions + " root root " + std::to_string(file->GetSize()) + " " + Utils::FormatDate(file->creationDate) + " " + file->name + "\n";
            output->write(line.c_str(), line.size());
        }

        return;
    }

    for (const espnix::Folder *subFolder : folder->folders)
    {
        std::string folderName = subFolder->name + " ";
        output->write(folderName.c_str(), folderName.size());
    }

    for (espnix::File *file : folder->files)
    {
        std::string fileName = file->name + " ";
        output->write(fileName.c_str(), fileName.size());
    }

    output->write("\n", 1);
}

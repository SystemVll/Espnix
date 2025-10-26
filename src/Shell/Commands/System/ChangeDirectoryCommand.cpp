#include "ChangeDirectoryCommand.h"

#include <FileSystem/Folder.h>
#include <FileSystem/FileSystem.h>
#include <Terminal/Terminal.h>

void ChangeDirectoryCommand::Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output)
{
    if (args.empty())
    {
        return;
    }

    std::string path = args[0];
    FileSystem *fileSystem = FileSystem::GetInstance();

    if (path == ".")
    {
        return;
    }

    if (path == "..")
    {
        if (fileSystem->currentPath != "/")
        {
            fileSystem->currentPath = fileSystem->currentPath.substr(0, fileSystem->currentPath.find_last_of('/'));
            if (fileSystem->currentPath.empty())
            {
                fileSystem->currentPath = "/";
            }
        }
        return;
    }

    std::string newPath;
    if (path[0] == '/')
    {
        newPath = path;
    }
    else
    {
        newPath = fileSystem->currentPath == "/" ? "/" + path : fileSystem->currentPath + "/" + path;
    }

    if (fileSystem->FolderExists(newPath))
    {
        fileSystem->currentPath = newPath;
    }
    else
    {
        terminal->Write("-esh: cd: " + path + ": No such file or directory\n");
    }
}
#include "MakeDirectoryCommand.h"

#include <FileSystem/Folder.h>
#include <FileSystem/File.h>
#include <FileSystem/FileSystem.h>
#include <Terminal/Terminal.h>
#include <Session/Session.h>
#include <IO/FileDescriptor.h>

void MakeDirectoryCommand::Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output)
{
    if (args.size() < 1)
    {
        const std::string msg1 = "mkdir: missing operand\n";
        output->write(msg1.c_str(), msg1.size());
        const std::string msg2 = "Try 'mkdir --help' for more information.\n";
        output->write(msg2.c_str(), msg2.size());
        return;
    }

    if (args[0] == "--help")
    {
        const std::string help =
            "Usage: mkdir [OPTION]... DIRECTORY...\n"
            "Create the DIRECTORY(ies), if they do not already exist.\n"
            "\n"
            "Mandatory arguments to long options are mandatory for short options too.\n"
            "  -m, --mode=MODE   set file mode (as in chmod), not a=rwx - umask\n"
            "  -p, --parents     no error if existing, make parent directories as needed\n"
            "  -v, --verbose     print a message for each created directory\n"
            "      --help        display this help and exit\n"
            "      --version     output version information and exit\n";
        output->write(help.c_str(), help.size());
        return;
    }

    FileSystem *fileSystem = FileSystem::GetInstance();
    espnix::Folder *folder = fileSystem->GetFolder(fileSystem->currentPath);

    for (auto subFolder : folder->folders)
    {
        if (subFolder->name == args[0])
        {
            const std::string errorMsg = "mkdir: cannot create directory '" + args[0] + "': File exists\n";
            output->write(errorMsg.c_str(), errorMsg.size());
            return;
        }
    }

    espnix::Folder *newFolder = new espnix::Folder();
    newFolder->name = args[0];
    newFolder->owner = terminal->session->user;
    newFolder->parent = folder;

    folder->AddFolder(newFolder);

    if (fileSystem->sdMounted)
    {
        fileSystem->SyncToSD();
    }
}
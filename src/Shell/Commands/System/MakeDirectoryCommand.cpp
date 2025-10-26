#include "MakeDirectoryCommand.h"

#include <FileSystem/Folder.h>
#include <FileSystem/File.h>
#include <FileSystem/FileSystem.h>
#include <Terminal/Terminal.h>
#include <Session/Session.h>

void MakeDirectoryCommand::Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output)
{
    if (args.size() < 1)
    {
        terminal->Write("mkdir: missing operand\n");
        terminal->Write("Try 'mkdir --help' for more information.\n");
        return;
    }

    if (args[0] == "--help")
    {
        terminal->Write("Usage: mkdir [OPTION]... DIRECTORY...\n");
        terminal->Write("Create the DIRECTORY(ies), if they do not already exist.\n");
        terminal->Write("\n");
        terminal->Write("Mandatory arguments to long options are mandatory for short options too.\n");
        terminal->Write("  -m, --mode=MODE   set file mode (as in chmod), not a=rwx - umask\n");
        terminal->Write("  -p, --parents     no error if existing, make parent directories as needed\n");
        terminal->Write("  -v, --verbose     print a message for each created directory\n");
        terminal->Write("      --help        display this help and exit\n");
        terminal->Write("      --version     output version information and exit\n");
        return;
    }

    FileSystem *fileSystem = FileSystem::GetInstance();
    espnix::Folder *folder = fileSystem->GetFolder(fileSystem->currentPath);

    for (auto subFolder : folder->folders)
    {
        if (subFolder->name == args[0])
        {
            terminal->Write("mkdir: cannot create directory '" + args[0] + "': File exists\n");
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
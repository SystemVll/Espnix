#include <iostream>
#include <string>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

#include <FileSystem/Folder.h>
#include <FileSystem/File.h>
#include <FileSystem/FileSystem.h>

#include <IO/FileDescriptorIO.h>

#include <Terminal/Terminal.h>

#include <Shell/Commands/ICommand.h>

#include <Shell/Commands/Busybox/ListCommand.h>
#include <Shell/Commands/Busybox/ChangeDirectoryCommand.h>
#include <Shell/Commands/Busybox/MakeDirectoryCommand.h>
#include <Shell/Commands/Busybox/EchoCommand.h>
#include <Shell/Commands/Busybox/CatCommand.h>
#include <Shell/Commands/Busybox/ClearCommand.h>

#include <Shell/Commands/Other/IwctlCommand.h>

#include "Shell.h"

Shell::Shell() : terminal(nullptr)
{
    commandRegistry["ls"] = std::make_shared<ListCommand>();
    commandRegistry["cd"] = std::make_shared<ChangeDirectoryCommand>();
    commandRegistry["echo"] = std::make_shared<EchoCommand>();
    commandRegistry["mkdir"] = std::make_shared<MakeDirectoryCommand>();
    commandRegistry["cat"] = std::make_shared<CatCommand>();
    commandRegistry["clear"] = std::make_shared<ClearCommand>();
    commandRegistry["iwctl"] = std::make_shared<IwctlCommand>();

    this->prompt = "espnix:/# ";
}

void Shell::Interpret(const std::string &input)
{
    std::istringstream iss(input);
    std::string command;
    std::vector<std::string> args;
    iss >> command;
    std::string arg;
    while (iss >> arg)
    {
        args.push_back(arg);
    }

    // Skip if command is empty
    if (command.empty())
    {
        this->Prompt();
        return;
    }

    terminal->Write("\n");

    auto it = commandRegistry.find(command);
    if (it != commandRegistry.end())
    {
        FileDescriptor *stdin_fd = new FileDescriptor(0);
        FileDescriptor *stdout_fd = new FileDescriptor(1);

        it->second->Execute(args, terminal, stdin_fd, stdout_fd);
    }
    else
    {
        terminal->Write(command + ": command not found\n");
    }

    FileSystem *fs = FileSystem::GetInstance();

    this->prompt = "espnix:" + fs->currentPath + "# ";
    this->Prompt();
}

void Shell::Prompt()
{
    terminal->Write(prompt);
}
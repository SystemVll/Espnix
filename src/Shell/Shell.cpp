#include <string>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

#include <FileSystem/FileSystem.h>

#include <IO/FileDescriptor.h>

#include <Terminal/Terminal.h>
#include <Utils/BootMessages.h>

#include <Shell/Commands/ICommand.h>

#include <Shell/Commands/System/ListCommand.h>
#include <Shell/Commands/System/ChangeDirectoryCommand.h>
#include <Shell/Commands/System/MakeDirectoryCommand.h>
#include <Shell/Commands/System/EchoCommand.h>
#include <Shell/Commands/System/CatCommand.h>
#include <Shell/Commands/System/EvimCommand.h>
#include <Shell/Commands/System/ClearCommand.h>
#include <Shell/Commands/System/CompileCommand.h>
#include <Shell/Commands/System/RunCommand.h>

#include <Shell/Commands/Other/IwctlCommand.h>

#include "Shell.h"

Shell::Shell() : terminal(nullptr)
{
    BootMessages::PrintInfo("Registering system commands");

    commandRegistry["ls"] = std::make_shared<ListCommand>();
    commandRegistry["cd"] = std::make_shared<ChangeDirectoryCommand>();
    commandRegistry["echo"] = std::make_shared<EchoCommand>();
    commandRegistry["mkdir"] = std::make_shared<MakeDirectoryCommand>();
    commandRegistry["cat"] = std::make_shared<CatCommand>();
    commandRegistry["evim"] = std::make_shared<EvimCommand>();
    commandRegistry["clear"] = std::make_shared<ClearCommand>();
    commandRegistry["compile"] = std::make_shared<CompileCommand>();
    commandRegistry["run"] = std::make_shared<RunCommand>();

    BootMessages::PrintOK("Registered 8 built-in commands");

    BootMessages::PrintInfo("Loading additional modules");
    commandRegistry["iwctl"] = std::make_shared<IwctlCommand>();
    BootMessages::PrintOK("Network utilities loaded");

    this->fs = FileSystem::GetInstance();

    this->prompt = "espnix:" + this->fs->currentPath + "# ";
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

    if (!command.empty())
    {
        const auto it = commandRegistry.find(command);
        if (it != commandRegistry.end())
        {
            auto *stdin_fd = new FileDescriptor(0);
            auto *stdout_fd = new FileDescriptor(1);

            if (std::find(args.begin(), args.end(), "|") == args.end()) {

            }

            terminal->Write("\n");
            it->second->Execute(args, terminal, stdin_fd, stdout_fd);
        }
        else
        {
            terminal->Write("\n");
            terminal->Write(command + ": command not found\n");
        }
    }
    else
    {
        terminal->Write("\n");
    }

    this->prompt = "espnix:" + this->fs->currentPath + "# ";
    this->Prompt();
}

void Shell::Prompt()
{
    terminal->Write(prompt);
}
#include "RunCommand.h"
#include <Terminal/Terminal.h>
#include <FileSystem/FileSystem.h>
#include <FileSystem/File.h>
#include <Runtime/VirtualMachine.h>
#include <vector>

void RunCommand::Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output)
{
    if (args.size() < 1)
    {
        terminal->Write("Usage: run <bytecode_file>\n");
        terminal->Write("  Executes compiled .enix bytecode file\n");
        return;
    }

    FileSystem *fileSystem = FileSystem::GetInstance();
    std::string bytecodeFilePath = args[0];

    // Get the bytecode file
    espnix::File *bytecodeFile = fileSystem->GetFile(bytecodeFilePath);

    if (bytecodeFile == nullptr)
    {
        terminal->Write("run: error: " + bytecodeFilePath + ": No such file or directory\n");
        return;
    }

    terminal->Write("Loading " + bytecodeFilePath + "...\n");

    try
    {
        // Read bytecode
        std::string bytecodeStr = bytecodeFile->Read();

        // Convert string to byte vector
        std::vector<uint8_t> bytecode;
        bytecode.reserve(bytecodeStr.size());
        for (char c : bytecodeStr)
        {
            bytecode.push_back(static_cast<uint8_t>(c));
        }

        terminal->Write("Executing (" + std::to_string(bytecode.size()) + " bytes)...\n");
        terminal->Write("--- Output ---\n");

        // Create and run VM
        VirtualMachine vm;
        vm.load(bytecode);
        vm.execute();

        terminal->Write("\n--- Execution complete ---\n");
    }
    catch (const std::exception& e)
    {
        terminal->Write("\nrun: runtime error: " + std::string(e.what()) + "\n");
    }
    catch (...)
    {
        terminal->Write("\nrun: runtime error: Unknown execution error\n");
    }
}
#ifndef RUN_COMMAND_H
#define RUN_COMMAND_H

#include <vector>
#include <string>

#include <Shell/Commands/ICommand.h>

class Terminal;

class RunCommand : public ICommand
{
public:
    void Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output) override;
};

#endif


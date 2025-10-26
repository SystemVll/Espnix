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

    if (bytecodeFilePath.length() > 3 &&
        bytecodeFilePath.substr(bytecodeFilePath.length() - 3) == ".es")
    {
        terminal->Write("run: error: cannot execute source file '" + bytecodeFilePath + "'\n");
        terminal->Write("  Did you mean: run " + bytecodeFilePath.substr(0, bytecodeFilePath.length() - 3) + ".enix\n");
        terminal->Write("  First compile with: compile " + bytecodeFilePath + "\n");
        return;
    }

    espnix::File *bytecodeFile = fileSystem->GetFile(bytecodeFilePath);

    if (bytecodeFile == nullptr)
    {
        terminal->Write("run: error: " + bytecodeFilePath + ": No such file or directory\n");

        if (bytecodeFilePath.length() > 5 &&
            bytecodeFilePath.substr(bytecodeFilePath.length() - 5) == ".enix")
        {
            std::string sourceFile = bytecodeFilePath.substr(0, bytecodeFilePath.length() - 5) + ".es";
            if (fileSystem->GetFile(sourceFile) != nullptr)
            {
                terminal->Write("  Found source file: " + sourceFile + "\n");
                terminal->Write("  Compile it first: compile " + sourceFile + "\n");
            }
        }

        return;
    }

    terminal->Write("Loading " + bytecodeFilePath + "...\n");

    try
    {
        std::string bytecodeStr = bytecodeFile->Read();

        std::vector<uint8_t> bytecode;
        bytecode.reserve(bytecodeStr.size());
        for (char c : bytecodeStr)
        {
            bytecode.push_back(static_cast<uint8_t>(c));
        }

        VirtualMachine vm;
        vm.load(bytecode);
        vm.execute();
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


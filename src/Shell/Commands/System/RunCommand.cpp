#include "RunCommand.h"
#include <Terminal/Terminal.h>
#include <FileSystem/FileSystem.h>
#include <FileSystem/File.h>
#include <Runtime/VirtualMachine.h>
#include <IO/FileDescriptor.h>
#include <vector>

void RunCommand::Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output)
{
    if (args.size() < 1)
    {
        const std::string msg1 = "Usage: run <bytecode_file>\n";
        output->write(msg1.c_str(), msg1.size());
        const std::string msg2 = "  Executes compiled .enix bytecode file\n";
        output->write(msg2.c_str(), msg2.size());
        return;
    }

    FileSystem *fileSystem = FileSystem::GetInstance();
    std::string bytecodeFilePath = args[0];

    if (bytecodeFilePath.length() > 3 &&
        bytecodeFilePath.substr(bytecodeFilePath.length() - 3) == ".es")
    {
        const std::string err1 = "run: error: cannot execute source file '" + bytecodeFilePath + "'\n";
        output->write(err1.c_str(), err1.size());
        const std::string err2 = "  Did you mean: run " + bytecodeFilePath.substr(0, bytecodeFilePath.length() - 3) + ".enix\n";
        output->write(err2.c_str(), err2.size());
        const std::string err3 = "  First compile with: compile " + bytecodeFilePath + "\n";
        output->write(err3.c_str(), err3.size());
        return;
    }

    espnix::File *bytecodeFile = fileSystem->GetFile(bytecodeFilePath);

    if (bytecodeFile == nullptr)
    {
        const std::string err1 = "run: error: " + bytecodeFilePath + ": No such file or directory\n";
        output->write(err1.c_str(), err1.size());

        if (bytecodeFilePath.length() > 5 &&
            bytecodeFilePath.substr(bytecodeFilePath.length() - 5) == ".enix")
        {
            std::string sourceFile = bytecodeFilePath.substr(0, bytecodeFilePath.length() - 5) + ".es";
            if (fileSystem->GetFile(sourceFile) != nullptr)
            {
                const std::string err2 = "  Found source file: " + sourceFile + "\n";
                output->write(err2.c_str(), err2.size());
                const std::string err3 = "  Compile it first: compile " + sourceFile + "\n";
                output->write(err3.c_str(), err3.size());
            }
        }

        return;
    }

    const std::string loadMsg = "Loading " + bytecodeFilePath + "...\n";
    output->write(loadMsg.c_str(), loadMsg.size());

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
        const std::string errMsg = "\nrun: runtime error: " + std::string(e.what()) + "\n";
        output->write(errMsg.c_str(), errMsg.size());
    }
    catch (...)
    {
        const std::string errMsg = "\nrun: runtime error: Unknown execution error\n";
        output->write(errMsg.c_str(), errMsg.size());
    }
}


#include "CompileCommand.h"
#include <Terminal/Terminal.h>
#include <FileSystem/FileSystem.h>
#include <FileSystem/File.h>
#include <FileSystem/Folder.h>
#include <Runtime/Lexer.h>
#include <Runtime/Compiler.h>
#include <IO/FileDescriptor.h>
#include <sstream>
#include <vector>


void CompileCommand::Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output)
{
    if (args.size() < 1)
    {
        const std::string msg1 = "Usage: compile <source_file> [output_file]\n";
        output->write(msg1.c_str(), msg1.size());
        const std::string msg2 = "  Compiles source code to .enix bytecode format\n";
        output->write(msg2.c_str(), msg2.size());
        return;
    }

    FileSystem *fileSystem = FileSystem::GetInstance();
    const std::string& sourceFilePath = args[0];

    espnix::File *sourceFile = fileSystem->GetFile(sourceFilePath);

    if (sourceFile == nullptr)
    {
        const std::string errMsg = "compile: error: " + sourceFilePath + ": No such file or directory\n";
        output->write(errMsg.c_str(), errMsg.size());
        return;
    }

    std::string outputFilePath;
    if (args.size() >= 2)
    {
        outputFilePath = args[1];
    }
    else
    {
        size_t dotPos = sourceFilePath.find_last_of('.');
        if (dotPos != std::string::npos)
        {
            outputFilePath = sourceFilePath.substr(0, dotPos) + ".enix";
        }
        else
        {
            outputFilePath = sourceFilePath + ".enix";
        }
    }

    if (!outputFilePath.empty() && outputFilePath[0] != '/')
    {
        outputFilePath = fileSystem->currentPath + "/" + outputFilePath;
    }

    const std::string compilingMsg = "Compiling " + sourceFilePath + "...\n";
    output->write(compilingMsg.c_str(), compilingMsg.size());

    try
    {
        std::string sourceCode = sourceFile->Read();

        Lexer lexer(sourceCode.c_str());
        std::vector<Token>& tokens = lexer.tokenize();

        const std::string lexMsg = "Lexical analysis complete (" + std::to_string(tokens.size()) + " tokens)\n";
        output->write(lexMsg.c_str(), lexMsg.size());

        Compiler compiler(tokens);
        std::vector<uint8_t>& bytecode = compiler.compile();

        const std::string compMsg = "Compilation complete (" + std::to_string(bytecode.size()) + " bytes)\n";
        output->write(compMsg.c_str(), compMsg.size());

        std::string bytecodeStr;
        bytecodeStr.reserve(bytecode.size());
        for (uint8_t byte : bytecode)
        {
            bytecodeStr.push_back(static_cast<char>(byte));
        }

        espnix::File *outputFile = fileSystem->GetFile(outputFilePath);

        if (outputFile != nullptr)
        {
            // File exists, overwrite using WriteFile for auto-sync
            fileSystem->WriteFile(outputFile, bytecodeStr, outputFilePath);
            const std::string sizeMsg = "Binary file size : " + std::to_string(bytecodeStr.size()) + " bytes\n";
            output->write(sizeMsg.c_str(), sizeMsg.size());
            const std::string outMsg = "Output written to " + outputFilePath + " (overwritten)\n";
            output->write(outMsg.c_str(), outMsg.size());
        }
        else
        {
            size_t lastSlash = outputFilePath.find_last_of('/');
            std::string fileName = (lastSlash != std::string::npos)
                ? outputFilePath.substr(lastSlash + 1)
                : outputFilePath;

            std::string folderPath;
            if (lastSlash != std::string::npos)
            {
                folderPath = outputFilePath.substr(0, lastSlash);
                if (folderPath.empty()) folderPath = "/";
            }
            else
            {
                folderPath = fileSystem->currentPath;
            }

            espnix::Folder *targetFolder = fileSystem->GetFolder(folderPath);
            if (targetFolder == nullptr)
            {
                const std::string errMsg = "compile: error: directory not found: " + folderPath + "\n";
                output->write(errMsg.c_str(), errMsg.size());
                return;
            }

            espnix::File *newFile = new espnix::File();
            newFile->name = fileName;
            newFile->permissions = 0644;

            // Write content using WriteFile for auto-sync
            fileSystem->WriteFile(newFile, bytecodeStr, outputFilePath);

            targetFolder->AddFile(newFile);
            const std::string sizeMsg = "Binary file size : " + std::to_string(bytecodeStr.size()) + " bytes\n";
            output->write(sizeMsg.c_str(), sizeMsg.size());
            const std::string outMsg = "Output written to " + outputFilePath + "\n";
            output->write(outMsg.c_str(), outMsg.size());
        }

        // Auto-sync handled by WriteFile, no manual sync needed
        const std::string successMsg = "Compilation successful!\n";
        output->write(successMsg.c_str(), successMsg.size());
    }
    catch (const std::exception& e)
    {
        const std::string errMsg = "compile: error: " + std::string(e.what()) + "\n";
        output->write(errMsg.c_str(), errMsg.size());
    }
    catch (...)
    {
        const std::string errMsg = "compile: error: Unknown compilation error\n";
        output->write(errMsg.c_str(), errMsg.size());
    }
}

#include "CompileCommand.h"
#include <Terminal/Terminal.h>
#include <FileSystem/FileSystem.h>
#include <FileSystem/File.h>
#include <FileSystem/Folder.h>
#include <Runtime/Lexer.h>
#include <Runtime/Compiler.h>
#include <sstream>
#include <vector>


void CompileCommand::Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output)
{
    if (args.size() < 1)
    {
        terminal->Write("Usage: compile <source_file> [output_file]\n");
        terminal->Write("  Compiles source code to .enix bytecode format\n");
        return;
    }

    FileSystem *fileSystem = FileSystem::GetInstance();
    std::string sourceFilePath = args[0];

    espnix::File *sourceFile = fileSystem->GetFile(sourceFilePath);

    if (sourceFile == nullptr)
    {
        terminal->Write("compile: error: " + sourceFilePath + ": No such file or directory\n");
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

    terminal->Write("Compiling " + sourceFilePath + "...\n");

    try
    {
        std::string sourceCode = sourceFile->Read();

        Lexer lexer(sourceCode.c_str());
        std::vector<Token>& tokens = lexer.tokenize();

        terminal->Write("Lexical analysis complete (" + std::to_string(tokens.size()) + " tokens)\n");

        Compiler compiler(tokens);
        std::vector<uint8_t>& bytecode = compiler.compile();

        terminal->Write("Compilation complete (" + std::to_string(bytecode.size()) + " bytes)\n");

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
            terminal->Write("Binary file size : " + std::to_string(bytecodeStr.size()) + " bytes\n");
            terminal->Write("Output written to " + outputFilePath + " (overwritten)\n");
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
                terminal->Write("compile: error: directory not found: " + folderPath + "\n");
                return;
            }

            espnix::File *newFile = new espnix::File();
            newFile->name = fileName;
            newFile->permissions = 0644;

            // Write content using WriteFile for auto-sync
            fileSystem->WriteFile(newFile, bytecodeStr, outputFilePath);

            targetFolder->AddFile(newFile);
            terminal->Write("Binary file size : " + std::to_string(bytecodeStr.size()) + " bytes\n");
            terminal->Write("Output written to " + outputFilePath + "\n");
        }

        // Auto-sync handled by WriteFile, no manual sync needed
        terminal->Write("Compilation successful!\n");
    }
    catch (const std::exception& e)
    {
        terminal->Write("compile: error: " + std::string(e.what()) + "\n");
    }
    catch (...)
    {
        terminal->Write("compile: error: Unknown compilation error\n");
    }
}

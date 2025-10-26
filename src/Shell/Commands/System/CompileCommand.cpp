#include <Terminal/Terminal.h>
#include <FileSystem/FileSystem.h>
#include <FileSystem/File.h>
#include <Runtime/Lexer.h>
#include <Runtime/Compiler.h>
#include <sstream>
#include <vector>

#include "CompileCommand.h"

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

    terminal->Write("Compiling " + sourceFilePath + "...\n");

    try
    {
        // Read source code
        std::string sourceCode = sourceFile->Read();

        // Tokenize the source code
        Lexer lexer(sourceCode.c_str());
        std::vector<Token>& tokens = lexer.tokenize();

        terminal->Write("Lexical analysis complete (" + std::to_string(tokens.size()) + " tokens)\n");

        // Compile tokens to bytecode
        Compiler compiler(tokens);
        std::vector<uint8_t>& bytecode = compiler.compile();

        terminal->Write("Compilation complete (" + std::to_string(bytecode.size()) + " bytes)\n");

        // Convert bytecode to string for file writing
        std::string bytecodeStr;
        bytecodeStr.reserve(bytecode.size());
        for (uint8_t byte : bytecode)
        {
            bytecodeStr.push_back(static_cast<char>(byte));
        }

        // Check if output file already exists
        espnix::File *outputFile = fileSystem->GetFile(outputFilePath);

        if (outputFile != nullptr)
        {
            // File exists, overwrite it
            outputFile->Write(bytecodeStr);
            terminal->Write("Output written to " + outputFilePath + " (overwritten)\n");
        }
        else
        {
            size_t lastSlash = outputFilePath.find_last_of('/');
            std::string fileName = (lastSlash != std::string::npos)
                ? outputFilePath.substr(lastSlash + 1)
                : outputFilePath;

            espnix::File *newFile = new espnix::File();
            newFile->name = fileName;
            newFile->Write(bytecodeStr);

            terminal->Write("Output written to " + outputFilePath + "\n");
        }

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

#include <FileSystem/File.h>
#include <FileSystem/FileSystem.h>
#include <FileSystem/Folder.h>
#include <Terminal/Terminal.h>
#include <Arduino.h>
#include <cstring>
#include <vector>
#include <sstream>

#include "EvimCommand.h"
#include "IO/FileDescriptor.h"

enum class EditorMode {
    COMMAND,
    INSERT
};

// Helper function to display current line status
void showCurrentLineStatus(Terminal *terminal, const std::vector<std::string> &lines, size_t currentLine, FileDescriptor *output) {
    if (currentLine < lines.size()) {
        const std::string statusMsg = "Line " + std::to_string(currentLine + 1) + "/" + std::to_string(lines.size()) + ": " + lines[currentLine] + "\n";
        output->write(statusMsg.c_str(), statusMsg.size());
    }
}

void EvimCommand::Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output)
{
    if (args.size() != 1)
    {
        const std::string msg = "Usage: evim <file>\n";
        output->write(msg.c_str(), msg.size());
        return;
    }

    FileSystem *fileSystem = FileSystem::GetInstance();
    std::string filePath = args[0];
    espnix::File *file = fileSystem->GetFile(filePath);

    // If file doesn't exist, create it
    if (file == nullptr)
    {
        // Create a new file using FileSystem
        file = fileSystem->CreateFile(filePath, 0644);
        if (file == nullptr)
        {
            const std::string errMsg = "Error: Could not create file " + filePath + "\n";
            output->write(errMsg.c_str(), errMsg.size());
            return;
        }
        const std::string newMsg = "evim: " + filePath + ": New file\n";
        output->write(newMsg.c_str(), newMsg.size());
    }

    // Read existing content and split into lines
    std::vector<std::string> lines;
    std::string content = file->Read();
    if (!content.empty())
    {
        std::istringstream iss(content);
        std::string line;
        while (std::getline(iss, line))
        {
            lines.push_back(line);
        }
    }

    // Ensure there's at least one empty line for editing
    if (lines.empty())
    {
        lines.push_back("");
    }

    // Display the file content with line numbers
    const std::string editingMsg = "=== Editing " + filePath + " ===\n";
    output->write(editingMsg.c_str(), editingMsg.size());
    for (size_t i = 0; i < lines.size(); i++)
    {
        const std::string lineMsg = std::to_string(i + 1) + ": " + lines[i] + "\n";
        output->write(lineMsg.c_str(), lineMsg.size());
    }

    // Initialize editor state
    EditorMode mode = EditorMode::COMMAND;
    size_t currentLine = 0;

    const std::string header1 = "\n--- EVIM Editor ---\n";
    output->write(header1.c_str(), header1.size());
    const std::string header2 = "Commands: ':q' (quit), ':wq' (save & quit), ':w' (save)\n";
    output->write(header2.c_str(), header2.size());
    const std::string header3 = "          'i' (insert), 'I' (insert at start), 'a' (append), 'A' (append at end)\n";
    output->write(header3.c_str(), header3.size());
    const std::string header4 = "          'o' (new line), 'dd' (delete line)\n";
    output->write(header4.c_str(), header4.size());
    const std::string header5 = "          'j' (next line), 'k' (prev line), '[num]' (goto line)\n";
    output->write(header5.c_str(), header5.size());
    const std::string statusMsg = "Current mode: COMMAND | Line: " + std::to_string(currentLine + 1) + "/" + std::to_string(lines.size()) + "\n";
    output->write(statusMsg.c_str(), statusMsg.size());
    const std::string prompt = "> ";
    output->write(prompt.c_str(), prompt.size());
    std::string currentInput = "";
    bool fileModified = false;

    while (true)
    {
        // Read one character at a time
        char ch;
        ssize_t bytesRead = input->read(&ch, 1);

        if (bytesRead <= 0)
        {
            // Add small delay to prevent busy waiting
            delay(10);
            continue;
        }

        // Handle ESC key immediately in insert mode
        if (mode == EditorMode::INSERT && ch == 27) // ESC key
        {
            mode = EditorMode::COMMAND;
            const std::string cmdMode = "\n-- COMMAND MODE --\n";
            output->write(cmdMode.c_str(), cmdMode.size());
            const std::string linePrompt = "Line: " + std::to_string(currentLine + 1) + "/" + std::to_string(lines.size()) + " > ";
            output->write(linePrompt.c_str(), linePrompt.size());
            continue;
        }

        // Handle different characters
        if (ch == '\n' || ch == '\r')
        {
            // Process the complete command/line
            output->write("\n", 1); // Echo newline

            std::string command = currentInput;
            currentInput = ""; // Reset input buffer

            if (mode == EditorMode::COMMAND)
            {
                // Handle command mode
                if (command == ":q")
                {
                    if (fileModified)
                    {
                        const std::string msg = "File has unsaved changes. Use ':wq' to save or ':q!' to force quit.\n";
                        output->write(msg.c_str(), msg.size());
                    }
                    else
                    {
                        const std::string msg = "Exiting without saving.\n";
                        output->write(msg.c_str(), msg.size());
                        return;
                    }
                }
                else if (command == ":q!")
                {
                    const std::string msg = "Exiting without saving (forced).\n";
                    output->write(msg.c_str(), msg.size());
                    return;
                }
                else if (command == ":wq")
                {
                    // Save and quit
                    std::string newContent = "";
                    for (size_t i = 0; i < lines.size(); i++)
                    {
                        newContent += lines[i];
                        if (i < lines.size() - 1)
                        {
                            newContent += "\n";
                        }
                    }
                    file->Write(newContent);
                    const std::string msg = "File saved and exiting.\n";
                    output->write(msg.c_str(), msg.size());
                    return;
                }
                else if (command == ":w")
                {
                    // Save only
                    std::string newContent = "";
                    for (size_t i = 0; i < lines.size(); i++)
                    {
                        newContent += lines[i];
                        if (i < lines.size() - 1)
                        {
                            newContent += "\n";
                        }
                    }
                    file->Write(newContent);
                    fileModified = false;
                    const std::string msg = "File saved.\n";
                    output->write(msg.c_str(), msg.size());
                }
                else if (command == "i")
                {
                    mode = EditorMode::INSERT;
                    const std::string msg1 = "-- INSERT MODE --\n";
                    output->write(msg1.c_str(), msg1.size());
                    const std::string msg2 = "Type your text. Press ESC to return to command mode.\n";
                    output->write(msg2.c_str(), msg2.size());
                }
                else if (command == "I")
                {
                    mode = EditorMode::INSERT;
                    const std::string msg1 = "-- INSERT MODE (at line start) --\n";
                    output->write(msg1.c_str(), msg1.size());
                    const std::string msg2 = "Type your text. Press ESC to return to command mode.\n";
                    output->write(msg2.c_str(), msg2.size());
                }
                else if (command == "a")
                {
                    mode = EditorMode::INSERT;
                    const std::string msg1 = "-- APPEND MODE --\n";
                    output->write(msg1.c_str(), msg1.size());
                    const std::string msg2 = "Type your text. Press ESC to return to command mode.\n";
                    output->write(msg2.c_str(), msg2.size());
                }
                else if (command == "A")
                {
                    mode = EditorMode::INSERT;
                    const std::string msg1 = "-- APPEND MODE (at line end) --\n";
                    output->write(msg1.c_str(), msg1.size());
                    const std::string msg2 = "Type your text. Press ESC to return to command mode.\n";
                    output->write(msg2.c_str(), msg2.size());
                }
                else if (command == "j")
                {
                    // Move to next line
                    if (currentLine < lines.size() - 1)
                    {
                        currentLine++;
                        showCurrentLineStatus(terminal, lines, currentLine, output);
                    }
                    else
                    {
                        const std::string msg = "Already at last line.\n";
                        output->write(msg.c_str(), msg.size());
                    }
                }
                else if (command == "k")
                {
                    // Move to previous line
                    if (currentLine > 0)
                    {
                        currentLine--;
                        showCurrentLineStatus(terminal, lines, currentLine, output);
                    }
                    else
                    {
                        const std::string msg = "Already at first line.\n";
                        output->write(msg.c_str(), msg.size());
                    }
                }
                else if (command == "o")
                {
                    // Insert new line after current
                    if (currentLine < lines.size())
                    {
                        lines.insert(lines.begin() + currentLine + 1, "");
                        currentLine++;
                    }
                    else
                    {
                        lines.push_back("");
                        currentLine = lines.size() - 1;
                    }
                    mode = EditorMode::INSERT;
                    fileModified = true;
                    const std::string msg = "-- INSERT MODE (new line) --\n";
                    output->write(msg.c_str(), msg.size());
                }
                else if (command == "dd")
                {
                    // Delete current line
                    if (!lines.empty() && currentLine < lines.size())
                    {
                        lines.erase(lines.begin() + currentLine);
                        if (currentLine >= lines.size() && currentLine > 0)
                        {
                            currentLine--;
                        }
                        if (lines.empty())
                        {
                            lines.push_back("");
                            currentLine = 0;
                        }
                        fileModified = true;
                        const std::string msg = "Line deleted.\n";
                        output->write(msg.c_str(), msg.size());
                    }
                }
                else if (!command.empty())
                {
                    // Try to parse line number for navigation
                    try
                    {
                        size_t lineNum = std::stoul(command);
                        if (lineNum > 0 && lineNum <= lines.size())
                        {
                            currentLine = lineNum - 1;
                            showCurrentLineStatus(terminal, lines, currentLine, output);
                        }
                        else
                        {
                            const std::string msg = "Invalid line number. Valid range: 1-" + std::to_string(lines.size()) + "\n";
                            output->write(msg.c_str(), msg.size());
                        }
                    }
                    catch (...)
                    {
                        const std::string msg = "Unknown command: " + command + "\n";
                        output->write(msg.c_str(), msg.size());
                    }
                }

                if (mode == EditorMode::COMMAND)
                {
                    const std::string linePrompt = "Line: " + std::to_string(currentLine + 1) + "/" + std::to_string(lines.size()) + " > ";
                    output->write(linePrompt.c_str(), linePrompt.size());
                }
            }
            else if (mode == EditorMode::INSERT)
            {
                // Handle insert mode - just insert the text
                // Insert/replace text at current line
                if (currentLine < lines.size())
                {
                    lines[currentLine] = command;
                }
                else
                {
                    lines.push_back(command);
                    currentLine = lines.size() - 1;
                }
                fileModified = true;

                // Move to next line for continuous editing
                currentLine++;
                if (currentLine >= lines.size())
                {
                    lines.push_back("");
                }
            }
        }
        else if (ch == 127 || ch == 8) // Backspace or DEL
        {
            if (!currentInput.empty())
            {
                currentInput.pop_back();
                const std::string backspace = "\b \b";
                output->write(backspace.c_str(), backspace.size()); // Backspace, space, backspace to erase character
            }
        }
        else if (ch >= 32 && ch <= 126) // Printable characters
        {
            currentInput += ch;
            output->write(&ch, 1); // Echo the character
        }
        // Ignore other control characters
    }
}

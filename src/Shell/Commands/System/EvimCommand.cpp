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
void showCurrentLineStatus(Terminal *terminal, const std::vector<std::string> &lines, size_t currentLine) {
    if (currentLine < lines.size()) {
        terminal->Write("Line " + std::to_string(currentLine + 1) + "/" + std::to_string(lines.size()) + ": " + lines[currentLine] + "\n");
    }
}

void EvimCommand::Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output)
{
    if (args.size() != 1)
    {
        terminal->Write("Usage: evim <file>\n");
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
            terminal->Write("Error: Could not create file " + filePath + "\n");
            return;
        }
        terminal->Write("evim: " + filePath + ": New file\n");
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
    terminal->Write("=== Editing " + filePath + " ===\n");
    for (size_t i = 0; i < lines.size(); i++)
    {
        terminal->Write(std::to_string(i + 1) + ": " + lines[i] + "\n");
    }

    // Initialize editor state
    EditorMode mode = EditorMode::COMMAND;
    size_t currentLine = 0;

    terminal->Write("\n--- EVIM Editor ---\n");
    terminal->Write("Commands: ':q' (quit), ':wq' (save & quit), ':w' (save)\n");
    terminal->Write("          'i' (insert), 'I' (insert at start), 'a' (append), 'A' (append at end)\n");
    terminal->Write("          'o' (new line), 'dd' (delete line)\n");
    terminal->Write("          'j' (next line), 'k' (prev line), '[num]' (goto line)\n");
    terminal->Write("Current mode: COMMAND | Line: " + std::to_string(currentLine + 1) + "/" + std::to_string(lines.size()) + "\n");
    terminal->Write("> ");
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
            terminal->Write("\n-- COMMAND MODE --\n");
            terminal->Write("Line: " + std::to_string(currentLine + 1) + "/" + std::to_string(lines.size()) + " > ");
            continue;
        }

        // Handle different characters
        if (ch == '\n' || ch == '\r')
        {
            // Process the complete command/line
            terminal->Write("\n"); // Echo newline

            std::string command = currentInput;
            currentInput = ""; // Reset input buffer

            if (mode == EditorMode::COMMAND)
            {
                // Handle command mode
                if (command == ":q")
                {
                    if (fileModified)
                    {
                        terminal->Write("File has unsaved changes. Use ':wq' to save or ':q!' to force quit.\n");
                    }
                    else
                    {
                        terminal->Write("Exiting without saving.\n");
                        return;
                    }
                }
                else if (command == ":q!")
                {
                    terminal->Write("Exiting without saving (forced).\n");
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
                    terminal->Write("File saved and exiting.\n");
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
                    terminal->Write("File saved.\n");
                }
                else if (command == "i")
                {
                    mode = EditorMode::INSERT;
                    terminal->Write("-- INSERT MODE --\n");
                    terminal->Write("Type your text. Press ESC to return to command mode.\n");
                }
                else if (command == "I")
                {
                    mode = EditorMode::INSERT;
                    terminal->Write("-- INSERT MODE (at line start) --\n");
                    terminal->Write("Type your text. Press ESC to return to command mode.\n");
                }
                else if (command == "a")
                {
                    mode = EditorMode::INSERT;
                    terminal->Write("-- APPEND MODE --\n");
                    terminal->Write("Type your text. Press ESC to return to command mode.\n");
                }
                else if (command == "A")
                {
                    mode = EditorMode::INSERT;
                    terminal->Write("-- APPEND MODE (at line end) --\n");
                    terminal->Write("Type your text. Press ESC to return to command mode.\n");
                }
                else if (command == "j")
                {
                    // Move to next line
                    if (currentLine < lines.size() - 1)
                    {
                        currentLine++;
                        showCurrentLineStatus(terminal, lines, currentLine);
                    }
                    else
                    {
                        terminal->Write("Already at last line.\n");
                    }
                }
                else if (command == "k")
                {
                    // Move to previous line
                    if (currentLine > 0)
                    {
                        currentLine--;
                        showCurrentLineStatus(terminal, lines, currentLine);
                    }
                    else
                    {
                        terminal->Write("Already at first line.\n");
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
                    terminal->Write("-- INSERT MODE (new line) --\n");
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
                        terminal->Write("Line deleted.\n");
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
                            showCurrentLineStatus(terminal, lines, currentLine);
                        }
                        else
                        {
                            terminal->Write("Invalid line number. Valid range: 1-" + std::to_string(lines.size()) + "\n");
                        }
                    }
                    catch (...)
                    {
                        terminal->Write("Unknown command: " + command + "\n");
                    }
                }

                if (mode == EditorMode::COMMAND)
                {
                    terminal->Write("Line: " + std::to_string(currentLine + 1) + "/" + std::to_string(lines.size()) + " > ");
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
                terminal->Write("\b \b"); // Backspace, space, backspace to erase character
            }
        }
        else if (ch >= 32 && ch <= 126) // Printable characters
        {
            currentInput += ch;
            terminal->Write(std::string(1, ch)); // Echo the character
        }
        // Ignore other control characters
    }
}

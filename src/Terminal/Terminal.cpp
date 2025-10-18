#include <Arduino.h>
#include <string.h>

#include <Session/Session.h>
#include <Shell/Shell.h>
#include <Utils/BootMessages.h>

#include "Terminal.h"

Terminal::Terminal(int baudRate, int user)
{
    Serial.begin(baudRate);
    // Wait for serial to initialize
    delay(100);

    BootMessages::PrintInfo("Starting serial communication at " + std::to_string(baudRate) + " baud");

    this->baudRate = baudRate;

    BootMessages::PrintInfo("Initializing session management");
    this->session = new Session();
    this->session->SetUser(user);
    BootMessages::PrintOK("Session created for user " + std::to_string(user));

    BootMessages::PrintInfo("Loading shell environment");
    this->shell = new Shell();
    this->shell->terminal = this;
    BootMessages::PrintOK("Shell initialized successfully");

    BootMessages::PrintOK("Terminal ready");
    Serial.println("");

    this->shell->Prompt();
}

void Terminal::Read()
{
    while (Serial.available())
    {
        char c = Serial.read();

        if (c == '\n' || c == '\r')
        {
            if (inputBuffer.empty())
            {
                return;
            }

            size_t start = inputBuffer.find_first_not_of(" \t\r\n");
            size_t end = inputBuffer.find_last_not_of(" \t\r\n");

            if (start != std::string::npos)
            {
                std::string input = inputBuffer.substr(start, end - start + 1);
                inputBuffer.clear();
                this->shell->Interpret(input);
            }
            else
            {
                inputBuffer.clear();
                this->Write("\n");
                this->shell->Prompt();
            }
            return;
        }

        Serial.write(c);
        inputBuffer += c;
    }
}

void Terminal::Write(std::string output)
{
    Serial.write(output.c_str());
}

void Terminal::Clear()
{
    Serial.write("\033[2J\033[H");
}
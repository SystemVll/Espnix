#include <Arduino.h>
#include <string.h>

#include <Session/Session.h>
#include <Shell/Shell.h>

#include "Terminal.h"

Terminal::Terminal(int baudRate, int user)
{
    Serial.begin(baudRate);
    this->baudRate = baudRate;
    this->session = new Session();
    this->session->SetUser(user);

    this->shell = new Shell();
    this->shell->terminal = this;
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
                this->Write("\n");
                this->shell->Prompt();
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
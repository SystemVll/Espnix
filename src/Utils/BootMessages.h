#ifndef BOOT_MESSAGES_H
#define BOOT_MESSAGES_H

#include <Arduino.h>
#include <string>

class BootMessages
{
public:
    static void PrintOK(const std::string &message)
    {
        Serial.print("[  \033[32mOK\033[0m  ] ");
        Serial.println(message.c_str());
    }

    static void PrintInfo(const std::string &message)
    {
        Serial.print("[ \033[36mINFO\033[0m ] ");
        Serial.println(message.c_str());
    }

    static void PrintWarn(const std::string &message)
    {
        Serial.print("[ \033[33mWARN\033[0m ] ");
        Serial.println(message.c_str());
    }

    static void PrintError(const std::string &message)
    {
        Serial.print("[\033[31mERROR\033[0m] ");
        Serial.println(message.c_str());
    }

    static void PrintFail(const std::string &message)
    {
        Serial.print("[ \033[31mFAIL\033[0m ] ");
        Serial.println(message.c_str());
    }

    static void PrintBanner()
    {
        Serial.println("\033[1;36m");
        Serial.println("  ███████╗███████╗██████╗ ███╗   ██╗██╗██╗  ██╗");
        Serial.println("  ██╔════╝██╔════╝██╔══██╗████╗  ██║██║╚██╗██╔╝");
        Serial.println("  █████╗  ███████╗██████╔╝██╔██╗ ██║██║ ╚███╔╝ ");
        Serial.println("  ██╔══╝  ╚════██║██╔═══╝ ██║╚██╗██║██║ ██╔██╗ ");
        Serial.println("  ███████╗███████║██║     ██║ ╚████║██║██╔╝ ██╗");
        Serial.println("  ╚══════╝╚══════╝╚═╝     ╚═╝  ╚═══╝╚═╝╚═╝  ╚═╝");
        Serial.println("\033[0m");
        Serial.println("  ESP32 Unix-like Operating System");
        Serial.println("");
    }
};

#endif


#include <Arduino.h>
#include <Terminal/Terminal.h>
#include <Utils/BootMessages.h>
#include <FileSystem/FileSystem.h>
#include <SD.h>

Terminal *terminalFrame;

void setup()
{
    Serial.begin(250000);
    delay(300);

    BootMessages::PrintBanner();

    BootMessages::PrintInfo("Espnix version 1.0 booting...");
    BootMessages::PrintInfo("Platform: ESP32");

    BootMessages::PrintInfo("Initializing hardware");
    BootMessages::PrintOK("CPU frequency: " + std::to_string(getCpuFrequencyMhz()) + " MHz");
    BootMessages::PrintOK("Free heap: " + std::to_string(ESP.getFreeHeap()) + " bytes");

    FileSystem *fileSystem = FileSystem::GetInstance();

    fileSystem->InitializeInitramfs();

    bool sdMounted = fileSystem->MountSDCard();

    if (sdMounted)
    {
        fileSystem->LoadFromSD();
    }

    BootMessages::PrintInfo("Starting terminal subsystem");
    terminalFrame = new Terminal(250000, 1);

    Serial.println("");
    BootMessages::PrintOK("System boot completed");

    if (fileSystem->inInitramfs)
    {
        BootMessages::PrintWarn("Running in initramfs mode - changes will not persist!");
    }
    else
    {
        BootMessages::PrintOK("Persistent storage available on SD card");
    }

    Serial.println("");
}

void loop()
{
    terminalFrame->Read();
}
#include "IwctlCommand.h"

#include <Terminal/Terminal.h>
#include <IO/FileDescriptorIO.h>
#include "WiFi.h"

void IwctlCommand::Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output)
{
    if (args.empty())
    {
        terminal->Write("ESP32 Interactive Wireless control utility\n");
        terminal->Write("Usage: iwctl [options] [command]\n");
        terminal->Write("\nCommands:\n");
        terminal->Write("  station scan                 Scan for networks\n");
        terminal->Write("  station get-networks         List available networks\n");
        terminal->Write("  station connect <ssid> <password>   Connect to network\n");
        terminal->Write("  device status                Show WiFi device status\n");
        terminal->Write("  device on                    Turn on WiFi\n");
        terminal->Write("  device off                   Turn off WiFi\n");
        return;
    }

    if (args[0] == "device")
    {
        if (args.size() > 1 && args[1] == "status")
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                terminal->Write("WiFi Status: Connected\n");
                terminal->Write("SSID: " + std::string(WiFi.SSID().c_str()) + "\n");
                terminal->Write("IP: " + std::string(WiFi.localIP().toString().c_str()) + "\n");
                terminal->Write("RSSI: " + std::to_string(WiFi.RSSI()) + " dBm\n");
            }
            else
            {
                terminal->Write("WiFi Status: Disconnected\n");
                terminal->Write("Mode: " + std::string(WiFi.getMode() == WIFI_OFF ? "OFF" : "ON") + "\n");
            }
            return;
        }

        if (args.size() > 1 && args[1] == "on")
        {
            WiFi.mode(WIFI_STA);
            terminal->Write("WiFi device turned on\n");
            return;
        }

        if (args.size() > 1 && args[1] == "off")
        {
            WiFi.mode(WIFI_OFF);
            terminal->Write("WiFi device turned off\n");
            return;
        }
    }

    if (args[0] == "station")
    {
        if (args.size() > 1 && args[1] == "scan")
        {
            terminal->Write("Scanning for networks...\n");
            int numNetworks = WiFi.scanNetworks();
            terminal->Write("Scan completed, found " + std::to_string(numNetworks) + " networks.\n");
            return;
        }

        if (args.size() > 1 && args[1] == "get-networks")
        {
            int numNetworks = WiFi.scanNetworks();
            if (numNetworks == 0)
            {
                terminal->Write("No networks found.\n");
                return;
            }

            terminal->Write("Networks available:\n");
            terminal->Write("# SSID                  Channel  RSSI  Security\n");
            for (int i = 0; i < numNetworks; i++)
            {
                std::string securityType;
                switch (WiFi.encryptionType(i))
                {
                case WIFI_AUTH_OPEN:
                    securityType = "open";
                    break;
                case WIFI_AUTH_WEP:
                    securityType = "wep";
                    break;
                case WIFI_AUTH_WPA_PSK:
                    securityType = "wpa-psk";
                    break;
                case WIFI_AUTH_WPA2_PSK:
                    securityType = "wpa2-psk";
                    break;
                case WIFI_AUTH_WPA_WPA2_PSK:
                    securityType = "wpa/wpa2";
                    break;
                default:
                    securityType = "unknown";
                    break;
                }

                terminal->Write(std::to_string(i) + " " + WiFi.SSID(i).c_str() +
                                "  " + std::to_string(WiFi.channel(i)) +
                                "  " + std::to_string(WiFi.RSSI(i)) + "  " +
                                securityType + "\n");
            }
            return;
        }

        if (args.size() > 3 && args[1] == "connect")
        {
            std::string ssid = args[2];
            std::string password = args[3];

            terminal->Write("Connecting to " + ssid + "...\n");
            WiFi.begin(ssid.c_str(), password.c_str());

            int retries = 0;
            while (WiFi.status() != WL_CONNECTED && retries < 20)
            {
                delay(500);
                terminal->Write(".");
                retries++;
            }
            terminal->Write("\n");

            if (WiFi.status() == WL_CONNECTED)
            {
                terminal->Write("Connected successfully!\n");
                terminal->Write("IP address: " + std::string(WiFi.localIP().toString().c_str()) + "\n");
            }
            else
            {
                terminal->Write("Failed to connect. Please check credentials or network availability.\n");
            }
            return;
        }
    }

    terminal->Write("Unknown iwctl command. Use 'iwctl' without arguments to see usage.\n");
}
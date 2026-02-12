#include "IwctlCommand.h"

#include <Terminal/Terminal.h>
#include <IO/FileDescriptor.h>
#include "WiFi.h"

void IwctlCommand::Execute(const std::vector<std::string> &args, Terminal *terminal, FileDescriptor *input, FileDescriptor *output)
{
    if (args.empty())
    {
        const std::string help =
            "ESP32 Interactive Wireless control utility\n"
            "Usage: iwctl [options] [command]\n"
            "\nCommands:\n"
            "  station scan                 Scan for networks\n"
            "  station get-networks         List available networks\n"
            "  station connect <ssid> <password>   Connect to network\n"
            "  device status                Show WiFi device status\n"
            "  device on                    Turn on WiFi\n"
            "  device off                   Turn off WiFi\n";
        output->write(help.c_str(), help.size());
        return;
    }

    if (args[0] == "device")
    {
        if (args.size() > 1 && args[1] == "status")
        {
            if (WiFi.status() == WL_CONNECTED)
            {
                const std::string status = "WiFi Status: Connected\n";
                output->write(status.c_str(), status.size());
                const std::string ssid = "SSID: " + std::string(WiFi.SSID().c_str()) + "\n";
                output->write(ssid.c_str(), ssid.size());
                const std::string ip = "IP: " + std::string(WiFi.localIP().toString().c_str()) + "\n";
                output->write(ip.c_str(), ip.size());
                const std::string rssi = "RSSI: " + std::to_string(WiFi.RSSI()) + " dBm\n";
                output->write(rssi.c_str(), rssi.size());
            }
            else
            {
                const std::string status = "WiFi Status: Disconnected\n";
                output->write(status.c_str(), status.size());
                const std::string mode = "Mode: " + std::string(WiFi.getMode() == WIFI_OFF ? "OFF" : "ON") + "\n";
                output->write(mode.c_str(), mode.size());
            }
            return;
        }

        if (args.size() > 1 && args[1] == "on")
        {
            WiFi.mode(WIFI_STA);
            const std::string msg = "WiFi device turned on\n";
            output->write(msg.c_str(), msg.size());
            return;
        }

        if (args.size() > 1 && args[1] == "off")
        {
            WiFi.mode(WIFI_OFF);
            const std::string msg = "WiFi device turned off\n";
            output->write(msg.c_str(), msg.size());
            return;
        }
    }

    if (args[0] == "station")
    {
        if (args.size() > 1 && args[1] == "scan")
        {
            const std::string msg1 = "Scanning for networks...\n";
            output->write(msg1.c_str(), msg1.size());
            int numNetworks = WiFi.scanNetworks();
            const std::string msg2 = "Scan completed, found " + std::to_string(numNetworks) + " networks.\n";
            output->write(msg2.c_str(), msg2.size());
            return;
        }

        if (args.size() > 1 && args[1] == "get-networks")
        {
            int numNetworks = WiFi.scanNetworks();
            if (numNetworks == 0)
            {
                const std::string msg = "No networks found.\n";
                output->write(msg.c_str(), msg.size());
                return;
            }

            const std::string header1 = "Networks available:\n";
            output->write(header1.c_str(), header1.size());
            const std::string header2 = "# SSID                  Channel  RSSI  Security\n";
            output->write(header2.c_str(), header2.size());
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

                const std::string line = std::to_string(i) + " " + WiFi.SSID(i).c_str() +
                                "  " + std::to_string(WiFi.channel(i)) +
                                "  " + std::to_string(WiFi.RSSI(i)) + "  " +
                                securityType + "\n";
                output->write(line.c_str(), line.size());
            }
            return;
        }

        if (args.size() > 3 && args[1] == "connect")
        {
            std::string ssid = args[2];
            std::string password = args[3];

            const std::string msg1 = "Connecting to " + ssid + "...\n";
            output->write(msg1.c_str(), msg1.size());
            WiFi.begin(ssid.c_str(), password.c_str());

            int retries = 0;
            while (WiFi.status() != WL_CONNECTED && retries < 20)
            {
                delay(500);
                const std::string dot = ".";
                output->write(dot.c_str(), dot.size());
                retries++;
            }
            output->write("\n", 1);

            if (WiFi.status() == WL_CONNECTED)
            {
                const std::string msg2 = "Connected successfully!\n";
                output->write(msg2.c_str(), msg2.size());
                const std::string msg3 = "IP address: " + std::string(WiFi.localIP().toString().c_str()) + "\n";
                output->write(msg3.c_str(), msg3.size());
            }
            else
            {
                const std::string msg2 = "Failed to connect. Please check credentials or network availability.\n";
                output->write(msg2.c_str(), msg2.size());
            }
            return;
        }
    }

    const std::string errMsg = "Unknown iwctl command. Use 'iwctl' without arguments to see usage.\n";
    output->write(errMsg.c_str(), errMsg.size());
}
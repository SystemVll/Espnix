#include <WiFi.h>

#include "WiFiDriver.h"

void WiFiDriver::Initialize()
{
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
}

void WiFiDriver::Shutdown()
{
    WiFi.disconnect();
    delay(100);
}

void WiFiDriver::ListNetworks()
{
    Serial.println("Scanning WiFi networks...");
    int n = WiFi.scanNetworks();
    if (n == 0)
    {
        Serial.println("No networks found.");
    }
    else
    {
        for (int i = 0; i < n; ++i)
        {
            Serial.print(i + 1);
            Serial.print(": ");
            Serial.print(WiFi.SSID(i));
            Serial.print(" (");
            Serial.print(WiFi.RSSI(i));
            Serial.print(" dBm) ");
            Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? "Open" : "Encrypted");
            delay(10);
        }
    }
}

bool WiFiDriver::Connect(const char *ssid, const char *password)
{
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi..");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.print("Connected to ");
    Serial.println(ssid);
    return true;
}

void WiFiDriver::Disconnect()
{
    WiFi.disconnect();
    Serial.println("Disconnected from WiFi");
}
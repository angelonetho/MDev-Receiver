#include <Arduino.h>
#include <HardwareSerial.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "WiFi.h"

// #define SSID "visitante"
// #define PASSWORD "v1s1t4nt3"

// #define SSID "Patrimonial-Gil 2.4"
// #define PASSWORD "32569874"

// #define SSID "infoprojetos"
// #define PASSWORD "sistemas987"

// #define SSID "Incubadora"
// #define PASSWORD "incubadora453"

#define SSID "NETHO-2.4G"
#define PASSWORD "televisao"

// #define SSID "Galaxy S10 Lite31ab"
// #define PASSWORD "12345678"

#define MAXML 128 // Default Size - Max Number of Sniffed MACs
#define ENTER 0   // Device on Range
#define EXIT 1    // Device off Range
#define RXD2 16
#define TXD2 17
#define PIN_RESTART 15
#define PIN_STOP 4
#define PLACE 1

struct Device
{
    String mac;
    int enterTime;
};

struct Device sniffedDevices[MAXML]; // List of Sniffed Devices
int countDevices = 0;                // Sniffed Devices Counter
int lastPosMac = 0;                  // Last Position - Last Received MAC
String currentMac;                   // Received MAC - Current
String lastMac = "";                 // Received MAC - Last
int TTL = 90;                        // Time to Live to remove device from list
bool block = false;                  // Block rotine "setSniffedMacs()"
int interval = 30;                   // Interval to reconnect with WiFi Router
int lastTimeConn;                    // Last time where ESP-32 is connected with AP
int intiTimeReconn;                  // Init time where ESP-32 is reconnected with AP
int timeToRestart = 90;              // Time to restart Local ESP when WiFi signal is losted
int intiTimeRemoteRestart;           // Init time where ESP-32 is not received MACs
int timeToRestartSender = 90;        // Time to restart Remote ESP when there are not any MACs
int intiTimeDebug;                   // Init time where ESP-32 is not received MACs

// URLs to ascess API - send devices/mac (in / out)
String url_in = "http://vast-zodiac-393718.rj.r.appspot.com/logs/in";
String url_out = "http://vast-zodiac-393718.rj.r.appspot.com/logs/out";
String url_turnedon = "http://vast-zodiac-393718.rj.r.appspot.com/mdevs";
String url_debug = "http://vast-zodiac-393718.rj.r.appspot.com/debug";

// https://api.ifprinteligente.com.br/mdev/rest.php/in/74:15:75:F5:D8:7F/1
// https://api.ifprinteligente.com.br/mdev/rest.php/out/74:15:75:F5:D8:7F/1
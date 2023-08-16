#include "variables.h"

void setSniffedMacs();             // Update List of Sniffed MACs
void notifyAPTurnedOn();           // Notify API that devices have been turned on
void sendAPI(int, struct Device);  // Send ENTER or EXIT device to API
void sendDebugAPI(String message); // Send Debug Message to API
bool checkExistsMac();             // Checks if MAC already exists in "sniffedMacs"
void checkExitedDevice(int);       // Checks if some device exited
void removeDeviceFromList(int);    // Remove Device with TTL expired
bool isMacAddress();               // Verify if data receiver is a MAC Address
void wifiConnect();                // Routine to connect with WiFi Router
String trim(String);               // Remove '\n'
void wifiReconnect();              // Routine to reconnect with WiFi Router
void espRestart();                 // Routine to restart Local ESP when WiFi is not reconecting
// void espRemoteRestart();             // Routine to restart Remote ESP

void setSniffedMacs()
{

    // Check if string is a MAC
    if (isMacAddress())
        return;

    // Repeated MAC
    if (currentMac == lastMac)
    {
        sniffedDevices[lastPosMac].enterTime = (int)time(NULL);
    }
    else
    {
        lastMac = currentMac;

        // Push MAC in "sniffedMacs" list
        if (checkExistsMac() && countDevices < MAXML)
        {
            sniffedDevices[countDevices].mac = currentMac;
            sniffedDevices[countDevices].enterTime = (int)time(NULL);
            lastPosMac = countDevices;
            sendAPI(ENTER, sniffedDevices[countDevices]);
            countDevices++;
        }
    }
}

void notifyAPTurnedOn()
{

    HTTPClient http;
    int httpResponseCode;
    String serverPath = url_turnedon + "/" + PLACE;

    if (WiFi.status() == WL_CONNECTED)
    {

        // Set domain name with URL
        http.begin(serverPath.c_str());
        http.addHeader("Content-Type", "text/html");
        http.addHeader("Content-Length", "0");
        // Send HTTP GET request
        httpResponseCode = http.POST("");
        // Free resources
        http.end();
        Serial.println("Turned on!");
        Serial.println(httpResponseCode);
    }
}

void sendAPI(int event, struct Device dev)
{

    HTTPClient http;
    int httpResponseCode;
    String serverPath;

    if (WiFi.status() == WL_CONNECTED)
    {
        StaticJsonDocument<200> jsonBuffer;
        JsonObject root = jsonBuffer.to<JsonObject>();

        dev.mac.trim();

        root["macAddress"] = dev.mac;
        root["mdevId"] = PLACE;

        Serial.println(root + "  root");

        String requestBody;
        serializeJson(root, requestBody);

        Serial.println(requestBody + "   rb");

        // Serial.print(dev.enterTime);
        if (event == ENTER)
        {
            serverPath = url_in;
            Serial.println("ENTER: " + dev.mac);
        }
        else
        {
            serverPath = url_out;
            // Serial.print("<=>");
            // Serial.print(time(NULL));
            Serial.println("EXIT: " + dev.mac);
        }
        // Serial.println(serverPath);

        WiFiClient client;

        // Set domain name with URL
        http.begin(client, serverPath.c_str());
        http.addHeader("Content-Type", "application/json");
        // Send HTTP GET request

        if (event == ENTER)
        {
            httpResponseCode = http.POST(requestBody);
        }
        else
        {
            httpResponseCode = http.PUT(requestBody);
        }

        Serial.println(httpResponseCode + "  response code");

        // Free resources
        http.end();

        // Serial.println(serverPath);
    }
}

bool checkExistsMac()
{

    int flag = true;
    for (int a = 0; a < countDevices; a++)
    {
        if (currentMac == sniffedDevices[a].mac)
        {
            sniffedDevices[a].enterTime = (int)time(NULL);
            flag = false;
        }
        else
        {
            checkExitedDevice(a);
        }
    }

    return flag;
}

void checkExitedDevice(int pos)
{

    int elapsedTime = ((int)time(NULL)) - sniffedDevices[pos].enterTime;

    if (elapsedTime > TTL)
    {
        sendAPI(EXIT, sniffedDevices[pos]);
        block = true;
        removeDeviceFromList(pos);
        block = false;
    }
}

void removeDeviceFromList(int pos)
{

    for (int a = pos; a < countDevices; a++)
        sniffedDevices[a] = sniffedDevices[a + 1];

    countDevices--;
}

bool isMacAddress()
{

    if (currentMac.charAt(2) == ':' &&
        currentMac.charAt(5) == ':' &&
        currentMac.length() == 18)
    {
        return false;
    }

    return true;
}

void wifiConnect()
{

    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID, PASSWORD);
    intiTimeReconn = (int)time(NULL);

    Serial.print("Connecting to WiFi Router.");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(1000);
        espRestart();
    }

    Serial.print("Connected to the WiFi network: ");
    Serial.println(WiFi.localIP());

    Serial.print("Wifi network signal strenght (RSSI): ");
    Serial.println(WiFi.RSSI());

    // Notify API that devices have been turned on
    notifyAPTurnedOn();

    // Notify ESP32-Listener After Connection
    if (digitalRead(PIN_STOP) == HIGH)
    {
        Serial2.println("OK\n");
    }
    digitalWrite(PIN_STOP, HIGH);
    Serial.print("Send 'OK' Notification to ESP32-Listener!");
    // intiTimeRemoteRestart = (int) time(NULL);
}

void wifiReconnect()
{

    int currentTime = (int)time(NULL);

    if ((currentTime - lastTimeConn) >= interval)
    {
        // Send STOP - ESP32-Listener
        digitalWrite(PIN_STOP, LOW);
        Serial.println("Reconnecting to WiFi...");
        WiFi.disconnect();
        wifiConnect();
    }
}

void espRestart()
{

    int currentTime = (int)time(NULL);

    if ((currentTime - intiTimeReconn) >= timeToRestart)
    {
        Serial.println("ESP - Restart!");
        // Restart - Local ESP
        ESP.restart();
    }
}

/*void espRemoteRestart() {

    int currentTime = (int) time(NULL);

    if((currentTime - intiTimeRemoteRestart) >= timeToRestartSender) {
        Serial.println("Remote ESP  - Restart!");
        // Restart - Remote ESP
        digitalWrite(PIN_RESTART, LOW);
        delay(100);
        // Restart - Local ESP
        ESP.restart();
        // intiTimeRemoteRestart = (int) time(NULL);
    }
}*/

void sendDebugAPI(String message)
{

    int currentTime = (int)time(NULL);

    if ((currentTime - intiTimeDebug) >= 10)
    {

        HTTPClient http;
        int httpResponseCode;
        String serverPath;

        if (WiFi.status() == WL_CONNECTED)
        {
            StaticJsonDocument<200> jsonBuffer;
            JsonObject root = jsonBuffer.to<JsonObject>();

            root["message"] = message;
            root["mdevId"] = PLACE;

            String requestBody;
            serializeJson(root, requestBody);

            Serial.println("Request Body: " + requestBody);

            serverPath = url_debug;
            Serial.println("[DEBUG] " + serverPath);
            // Set domain name with URL
            http.begin(serverPath.c_str());
            http.addHeader("Content-Type", "application/json");
            // Send HTTP GET request
            httpResponseCode = http.POST(requestBody);

            Serial.println("Response:" + httpResponseCode);
            // Free resources
            http.end();
        }

        intiTimeDebug = (int)time(NULL);
    }
}
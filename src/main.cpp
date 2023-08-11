#include "functions.h"

void setup()
{
  Serial.begin(9600);
  // Serial2.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  // pinMode(PIN_RESTART, OUTPUT);
  // digitalWrite(PIN_RESTART, HIGH);
  pinMode(PIN_STOP, OUTPUT);
  digitalWrite(PIN_STOP, HIGH);
  Serial.println("Receiver Start!");
  wifiConnect();
  intiTimeDebug = (int)time(NULL);
  delay(1000);
}

void loop()
{

  if (Serial2.available())
  {
    // intiTimeRemoteRestart = (int) time(NULL);
    intiTimeDebug = (int)time(NULL);
    Serial.println("Received MAC");
    currentMac = Serial2.readStringUntil('\n');
    if (!block)
    {
      if (WiFi.status() == WL_CONNECTED)
      {
        lastTimeConn = (int)time(NULL);
        setSniffedMacs();
      }
      else
      {
        wifiReconnect();
      }
    }
  }
  else
  {
    // espRemoteRestart();
    sendDebugAPI("MAC_Not_Available");
  }
  delay(100);
}
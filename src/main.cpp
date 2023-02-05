#include <Arduino.h>
#include <EEPROM.h>

#include <ESP8266WiFi.h>
// сбор mac-адресов устройств через UDP для будущего общения через ESP-NOW
#include <WiFiUdp.h>
WiFiUDP Udp;

#include "WiFiController.h"
WiFiController ctrl(256);


const uint16_t UDP_PORT = 4210;
char incomingPacket[255];
char replyPacekt[] = "OK";

void setup()
{
    Serial.begin(115200);
    delay(500);

    // ctrl.reset();
    ctrl.connect();


    // Udp.begin(UDP_PORT);
}

void loop()
{
    ctrl.tick();
}



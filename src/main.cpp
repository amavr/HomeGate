#include <Arduino.h>
#include <EEPROM.h>

#include <ESP8266WiFi.h>
// сбор mac-адресов устройств через UDP для будущего общения через ESP-NOW
#include <WiFiUdp.h>
WiFiUDP Udp;

#include "WiFiController.h"
WiFiController ctrl;


const uint16_t UDP_PORT = 4210;
char incomingPacket[255];
char replyPacekt[] = "OK";

void setup()
{
    Serial.begin(115200);
    delay(500);

    // ctrl.onUpload();
    ctrl.connect();


    // Udp.begin(UDP_PORT);
}

void loop()
{
    ctrl.tick();
}

void startPortal()
{
        // strcpy(cfg.ssid, String("gvv").c_str());
        // strcpy(cfg.pass, String("09090909").c_str());

        // saveConfig();
        // readConfig();


    // portalRun(); // запустить с таймаутом 60с
    // portalRun(120000); // запустить с кастомным таймаутом

    // Serial.println(portalStatus());
    // // статус: 0 error, 1 connect, 2 ap, 3 local, 4 exit, 5 timeout

    // if (portalStatus() == SP_SUBMIT)
    // {
    //     Serial.println(portalCfg.SSID);
    //     Serial.println(portalCfg.pass);
    //     // забираем логин-пароль
    //     memcpy(cfg.ssid, portalCfg.SSID, sizeof(cfg.ssid));
    //     memcpy(cfg.pass, portalCfg.pass, sizeof(cfg.pass));
    //     // strcpy(cfg.ssid, String("gvv").c_str());
    //     // strcpy(cfg.pass, String("09090909").c_str());

    //     saveConfig();
    //     readConfig();
    // }
}


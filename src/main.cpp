#include <Arduino.h>
#include <EEPROM.h>

#include <ESP8266WiFi.h>
// сбор mac-адресов устройств через UDP для будущего общения через ESP-NOW
#include <WiFiUdp.h>
WiFiUDP Udp;

#include "WiFiController.h"
WiFiController ctrl;

struct TConfig
{
    uint16_t timeouts = 20;
    char ssid[32] = "amavrx";
    char pass[32] = "oooooooo";
} cfg;

const uint16_t UDP_PORT = 4210;
char incomingPacket[255];
char replyPacekt[] = "OK";

void readConfig();
void saveConfig();
void initWiFi();

void cleanEEPROM()
{
    EEPROM.begin(256);
    for (int i = 0; i < (int)EEPROM.length(); i++)
    {
        EEPROM.put(i, 0);
    }
    EEPROM.put(0, cfg);
    EEPROM.commit();
}

void setup()
{
    Serial.begin(115200);
    delay(500);

    // cleanEEPROM();
    ctrl.onUpload();
    ctrl.connect();

    // readConfig();
    // initWiFi();

    // Udp.begin(UDP_PORT);
}

void loop()
{
}

void readConfig()
{
    EEPROM.get(0, cfg);
    Serial.printf("cfg.timeouts:%d cfg.ssid:%s cfg.pass:%s\n", cfg.timeouts, cfg.ssid, cfg.pass);
}

void saveConfig()
{
    EEPROM.begin(sizeof(TConfig));
    EEPROM.put(0, cfg);
    EEPROM.commit();
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

void initWiFi()
{
    uint32_t interval = cfg.timeouts * 1000;
    while (true)
    {
        char dot = '.';
        WiFi.begin(cfg.ssid, cfg.pass);
        Serial.printf("Try to connect to %s", cfg.ssid);

        uint32_t beg = millis();
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print(dot);
            if (millis() - beg > interval)
            {
                Serial.println("timeout");
                // startPortal();
                break;
            }
        }

        if (WiFi.status() == WL_CONNECTED)
        {
            Serial.println("connected");
            break;
        }
    }
    Serial.println("IP address: " + WiFi.localIP().toString());
}

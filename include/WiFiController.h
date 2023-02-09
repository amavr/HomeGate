#ifndef $WiFiController
#define $WiFiController

#include <Arduino.h>
#include <EEPROM.h>

// подключение к WiFi
#include <ESP8266WiFi.h>

// сайт настроек для поключения к WiFi
#include "ConfigSite.h"

struct TWiFiParams
{
    uint16_t connectPeriod = 30; // длительность попытки подключения к WiFi в секундах
    uint16_t siteLifeTime = 240; // длительность работы сайта для введения параметров подключения в секундах
    char ssid[20] = "x";
    char pass[20] = "x";
};

class WiFiController
{
public:
    WiFiController(uint16_t SizeEEPROM);
    void connect(bool isFirtsTime);
    void disconnect();
    void reset();
    void tick();

private:
    uint16_t eepromSize;
    TWiFiParams cfg;
    void loadWiFiParams();
    void saveWiFiParams();
    bool runSite();
};

WiFiController::WiFiController(uint16_t SizeEEPROM = 256)
{
    eepromSize = SizeEEPROM;
    Serial.printf("SizeEEPROM:%d\n", SizeEEPROM);
    EEPROM.begin(eepromSize);
}

void WiFiController::connect(bool isFirstTime)
{
    if (isFirstTime)
    {
        reset();
    }

    while (true)
    {
        bool changed = false;
        loadWiFiParams();
        WiFi.begin(cfg.ssid, cfg.pass);
        uint32_t started = millis();
        uint32_t period = cfg.connectPeriod * 1000;
        Serial.printf("Connecting to %s", cfg.ssid);
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print('.');
            if (millis() - started > period)
            {
                Serial.println("timeout");
                Serial.print("run AP to change config. Waiting changes...");
                changed = runSite();
                Serial.println(changed ? "has change" : "no change");
                if (changed)
                {
                    saveWiFiParams();
                    Serial.printf("saved ssid:%s passw:%s\n", cfg.ssid, cfg.pass);
                }
                // если не было изменений параметров
                // все равно проверять подключение,
                // может роутер выключался
                WiFi.disconnect(true, true);
                break;
            }
        }
    }
    Serial.println("done");
    Serial.println(WiFi.localIP());
}

void WiFiController::disconnect()
{
    WiFi.disconnect();
}

void WiFiController::reset()
{
    for (int i = 0; i < (int)EEPROM.length(); i++)
    {
        EEPROM.put(i, 0);
    }
    EEPROM.put(0, cfg);
    EEPROM.commit();
}

void WiFiController::tick()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        connect(false);
    }
}

void WiFiController::loadWiFiParams()
{
    EEPROM.get(0, cfg);
    Serial.printf("load ssid:%s pass:%s\n", cfg.ssid, cfg.pass);
}

void WiFiController::saveWiFiParams()
{
    EEPROM.put(0, cfg);
    EEPROM.commit();
}

bool WiFiController::runSite()
{
    ConfigSite site(cfg.ssid, cfg.pass, 240);
    site.run();
    bool changed = (strcmp(cfg.ssid, _ssid) != 0) || (strcmp(cfg.pass, _pass) != 0);
    strcpy(cfg.ssid, _ssid);
    strcpy(cfg.pass, _pass);
    return changed;
}

#endif
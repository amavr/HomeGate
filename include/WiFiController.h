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
    int connectPeriod = 40; // длительность попытки подключения к WiFi в секундах
    int siteLifeTime = 240; // длительность работы сайта для введения параметров подключения в секундах
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
        unsigned long started = millis();
        uint32_t period = cfg.connectPeriod * 1000;
        Serial.printf("Connecting to %s", cfg.ssid);
        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
            Serial.print('.');
            if (millis() - started > period)
            {
                started = millis();
                Serial.println("timeout");
                Serial.println("Start AP to change config.");
                changed = runSite();
                Serial.println(changed ? "Stop AP with changes" : "Stop AP without changes");
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
        if (WiFi.status() == WL_CONNECTED)
        {
            break;
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
    strcpy(siteCfg.ssid, cfg.ssid);
    strcpy(siteCfg.pass, cfg.pass);
    siteCfg.lifeTimeSec = cfg.siteLifeTime;

    run();

    if (changed())
    {
        strcpy(cfg.ssid, siteCfg.ssid);
        strcpy(cfg.pass, siteCfg.pass);
    }
    return changed();
}

#endif
#pragma once

#include <Arduino.h>
#include <EEPROM.h>

// подключение к WiFi
#include <ESP8266WiFi.h>

// организация страницы настроек для поключения к WiFi
#include <DNSServer.h>
#include <ESP8266WebServer.h>

#define SP_AP_NAME "ESP Config"   // название точки
#define SP_AP_IP 192, 168, 100, 1 // IP точки

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
    void connect();
    void disconnect();
    void onUpload();
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
    this->eepromSize = SizeEEPROM;
    Serial.printf("SizeEEPROM:%d\n", SizeEEPROM);
    EEPROM.begin(this->eepromSize);
}

void WiFiController::connect()
{
    while (true)
    {
        bool changed = false;
        loadWiFiParams();
        WiFi.begin(this->cfg.ssid, this->cfg.pass);
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
                changed = runSite();
                if (changed)
                {
                    WiFi.disconnect(true, true);
                    saveWiFiParams();
                    Serial.printf("saved ssid:%s passw:%s\n", this->cfg.ssid, this->cfg.pass);
                    break;
                    // WiFi.begin(this->cfg.ssid, this->cfg.pass);
                }
                // Serial.printf("Connecting to %s", cfg.ssid);
                // started = millis();
            }
        }
        if(changed){
            continue;
        }

        Serial.println("done");
        break;
        // выход из цикла только при успешном подключении
        // поэтому параметры с которыми WiFi подключен сохраняются
        // if (changed)
        // {
        //     saveWiFiParams();
        // }
    }
}

void WiFiController::disconnect()
{
    WiFi.disconnect();
}

void WiFiController::onUpload()
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
        connect();
    }
}

void WiFiController::loadWiFiParams()
{
    EEPROM.get(0, this->cfg);
    Serial.printf("load ssid:%s pass:%s\n", this->cfg.ssid, this->cfg.pass);
}

void WiFiController::saveWiFiParams()
{
    EEPROM.put(0, this->cfg);
    EEPROM.commit();
}

bool WiFiController::runSite()
{
    bool changed = true;

    // strcpy(cfg.ssid, String("gvv").c_str());
    // strcpy(cfg.pass, String("09090909").c_str());

    strcpy(this->cfg.ssid, String("amavr").c_str());
    strcpy(this->cfg.pass, String("oooooooo").c_str());

    return changed;
}

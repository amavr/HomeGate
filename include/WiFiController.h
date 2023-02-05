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
    void connect();
    void disconnect();
    void onUpload();

private:
    TWiFiParams cfg;
    void loadWiFiParams();
    void saveWiFiParams();
    bool runSite();
};

void WiFiController::connect()
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
            changed = runSite();
            if (changed)
            {
                WiFi.disconnect(true, true);
                WiFi.begin(cfg.ssid, cfg.pass);
            }
            Serial.printf("Connecting to %s", cfg.ssid);
            started = millis();
        }
    }
    Serial.println("done");
    // выход из цикла только при успешном подключении
    // поэтому параметры с которыми WiFi подключен сохраняются
    if (changed)
    {
        saveWiFiParams();
    }
}

void WiFiController::disconnect()
{
    WiFi.disconnect();
}

void WiFiController::onUpload()
{
    EEPROM.begin(sizeof(TWiFiParams));
    for (int i = 0; i < (int)EEPROM.length(); i++)
    {
        EEPROM.put(i, 0);
    }
    EEPROM.put(0, cfg);
    EEPROM.commit();
}

void WiFiController::loadWiFiParams()
{
    EEPROM.get(0, cfg);
}

void WiFiController::saveWiFiParams()
{
    EEPROM.begin(sizeof(TWiFiParams));
    EEPROM.put(0, cfg);
    EEPROM.commit();
}

bool WiFiController::runSite()
{
    bool changed = true;

    // strcpy(cfg.ssid, String("gvv").c_str());
    // strcpy(cfg.pass, String("09090909").c_str());

    strcpy(cfg.ssid, String("amavr").c_str());
    strcpy(cfg.pass, String("oooooooo").c_str());

    return changed;
}

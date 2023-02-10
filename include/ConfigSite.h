/*
    Создано на базе SimplePortal
    GitHub: https://github.com/GyverLibs/SimplePortal
*/
#ifndef $ConfigSite
#define $ConfigSite

// #include <Arduino.h>

struct SiteConfig
{
    int lifeTimeSec = 240;
    char name[20] = "ESP Config";
    char ssid[20] = "x";
    char pass[20] = "x";
};
extern SiteConfig siteCfg;



void run();
void start();
void stop();
bool tick();
void handleConnect();
bool changed();

#endif
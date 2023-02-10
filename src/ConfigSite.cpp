/*
    Создано на базе SimplePortal
    GitHub: https://github.com/GyverLibs/SimplePortal
*/

#include "ConfigSite.h"

#include <Arduino.h>

#include <DNSServer.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#else
#include <WiFi.h>
#include <WebServer.h>
#endif

static DNSServer dnsServer;
#ifdef ESP8266
static ESP8266WebServer webServer(80);
#else
static WebServer webServer(80);
#endif

const char html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
<meta name="viewport" content="width=device-width, initial-scale=1">
</head><body>
<style type="text/css">
    input[type="text"] {margin-bottom:8px;font-size:20px;}
    input[type="submit"] {width:180px; height:60px;margin-bottom:8px;font-size:20px;}
</style>
<center>
<h3>WiFi settings</h3>
<form action="/connect" method="POST">
    <input type="text" name="ssid" placeholder="SSID">
    <input type="text" name="pass" placeholder="Pass">
    <br/><br/>
    <input type="submit" value="Submit">
</form>
</center>
</body></html>)rawliteral";

static IPAddress addrAP(192, 168, 100, 1);

SiteConfig siteCfg;

bool isStarted = false;
bool isChanged = false;

// обработчик сабмита формы с новыми параметрами подключения к WiFi
void handleConnect()
{
    strcpy(siteCfg.ssid, webServer.arg("ssid").c_str());
    strcpy(siteCfg.pass, webServer.arg("pass").c_str());
    isStarted = false;
    isChanged = true;
}

// запуск сервера со страницей
void start()
{
    WiFi.softAPdisconnect();
    WiFi.disconnect();

    dnsServer.start(53, "*", addrAP);

    WiFi.mode(WIFI_AP);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(addrAP, addrAP, subnet);
    WiFi.softAP("ESP Config");

    webServer.onNotFound([]() {
        webServer.send(200, "text/html", html);
    });
    // _webServer.on("/generate_204", HTTP_GET, handleRoot);
    webServer.on("/connect", HTTP_POST, handleConnect);

    webServer.begin();
    isStarted = true;
}

void stop()
{
    WiFi.softAPdisconnect();
    webServer.stop();
    dnsServer.stop();
    isStarted = false;
}

bool tick()
{
    // тикает только когда запущен WEB сервер
    if (isStarted)
    {
        dnsServer.processNextRequest();
        webServer.handleClient();
        yield();
        // завершение работы сайта, после получения сабмита
        if (isChanged)
        {
            stop();
            return false;
        }
        return true;
    }
    return false;
}

void run()
{
    uint32_t beg = millis();
    start();

    uint32_t timeout = siteCfg.lifeTimeSec * 1000;

    while (tick())
    {
        if (millis() - beg > timeout)
        {
            break;
        }
        yield();
    }
    stop();
}

bool changed()
{
    return isChanged;
}
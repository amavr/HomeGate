/*
    Создано на базе SimplePortal
    GitHub: https://github.com/GyverLibs/SimplePortal
*/
#ifndef $ConfigSite
#define $ConfigSite

#include <Arduino.h>

#define NAME_AP "ESP Config" // название точки

#include <FS.h>

#include <DNSServer.h>
#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#else
#include <WiFi.h>
#include <WebServer.h>
#endif

static DNSServer _dnsServer;
#ifdef ESP8266
static ESP8266WebServer _webServer(80);
#else
static WebServer _webServer(80);
#endif

static IPAddress _ipAP(192, 168, 100, 1);

#define SP_ERROR 0
#define SP_SUBMIT 1
#define SP_SWITCH_AP 2
#define SP_SWITCH_LOCAL 3
#define SP_EXIT 4
#define SP_TIMEOUT 5

// static DNSServer _dnsServer;
// #ifdef ESP8266
// static ESP8266WebServer _webServer(80);
// #else
// static WebServer _webServer(80);
// #endif

class ConfigSite
{
public:
    ConfigSite(const char *ssid, const char *pass, uint16_t lifeTimeSec);
    void run();
    void start();
    bool tick();
    uint8_t portalStatus();

private:
    void stop();

    static void handleConnect();
    static void handleAP();
    static void handleLocal();
    static void handleExit();

    static String getContentType(String filename);
    static void handleRoot();
    static bool handleFileRead(String path);

    char _ssidAP[32];
    uint16_t _lifeTime;

    bool _started = false;
};

static uint8_t _status = 0;
static uint8_t _mode = WIFI_AP;
static char _ssid[20];
static char _pass[20];

ConfigSite::ConfigSite(const char *ssid, const char *pass, uint16_t lifeTimeSec)
{
    strcpy(_ssid, ssid);
    strcpy(_pass, pass);
    _lifeTime = lifeTimeSec * 1000;

    SPIFFS.begin(); // Start the SPI Flash Files System
}

void ConfigSite::handleConnect()
{
    strcpy(_ssid, _webServer.arg("ssid").c_str());
    strcpy(_pass, _webServer.arg("pass").c_str());
    _mode = WIFI_STA;
    _status = 1;
}

void ConfigSite::handleAP()
{
    _mode = WIFI_AP;
    _status = 2;
}

void ConfigSite::handleLocal()
{
    _mode = WIFI_STA;
    _status = 3;
}

void ConfigSite::handleExit()
{
    _status = 4;
}

String ConfigSite::getContentType(String filename)
{ // convert the file extension to the MIME type
    if (filename.endsWith(".htm"))
        return "text/html";
    else if (filename.endsWith(".html"))
        return "text/html";
    else if (filename.endsWith(".css"))
        return "text/css";
    else if (filename.endsWith(".js"))
        return "application/javascript";
    else if (filename.endsWith(".png"))
        return "image/png";
    else if (filename.endsWith(".gif"))
        return "image/gif";
    else if (filename.endsWith(".jpg"))
        return "image/jpeg";
    else if (filename.endsWith(".ico"))
        return "image/x-icon";
    else if (filename.endsWith(".xml"))
        return "text/xml";
    else if (filename.endsWith(".pdf"))
        return "application/x-pdf";
    else if (filename.endsWith(".zip"))
        return "application/x-zip";
    else if (filename.endsWith(".gz"))
        return "application/x-gzip";
    return "text/plain";
}

void ConfigSite::handleRoot()
{
    handleFileRead("/");
}

bool ConfigSite::handleFileRead(String path)
{ // send the right file to the client (if it exists)
    Serial.println("handleFileRead: " + path);
    if (path.endsWith("/"))
        path += "index.html";                  // If a folder is requested, send the index file
    String contentType = getContentType(path); // Get the MIME type
    String pathWithGz = path + ".gz";
    if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path))
    {                                                           // If the file exists, either as a compressed archive, or normal
        if (SPIFFS.exists(pathWithGz))                          // If there's a compressed version available
            path += ".gz";                                      // Use the compressed version
        File file = SPIFFS.open(path, "r");                     // Open the file
        size_t sent = _webServer.streamFile(file, contentType); // Send it to the client
        file.close();                                           // Close the file again
        Serial.println(String("\tSent file: ") + path);
        return true;
    }
    Serial.println(String("\tFile Not Found: ") + path);
    return false; // If the file doesn't exist, return false
}

void ConfigSite::start()
{
    WiFi.softAPdisconnect();
    WiFi.disconnect();

    strcpy(_ssidAP, NAME_AP);

    _dnsServer.start(53, "*", _ipAP);

    WiFi.mode(WIFI_AP);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(_ipAP, _ipAP, subnet);
    WiFi.softAP(_ssidAP);

    _webServer.onNotFound([]() {                                  // If the client requests any URI
        if (!handleFileRead(_webServer.uri()))                    // send it if it exists
            _webServer.send(404, "text/plain", "404: Not Found"); // otherwise, respond with a 404 (Not Found) error
    });
    _webServer.on("/generate_204", HTTP_GET, handleRoot);
    _webServer.on("/connect", HTTP_POST, handleConnect);
    _webServer.on("/ap", HTTP_POST, handleAP);
    _webServer.on("/local", HTTP_POST, handleLocal);
    _webServer.on("/exit", HTTP_POST, handleExit);

    _webServer.begin();
    _started = true;
    _status = 0;
}

void ConfigSite::stop()
{
    WiFi.softAPdisconnect();
    _webServer.stop();
    _dnsServer.stop();
    _started = false;
}

bool ConfigSite::tick()
{
    if (_started)
    {
        _dnsServer.processNextRequest();
        _webServer.handleClient();
        yield();
        if (_status)
        {
            stop();
            return 1;
        }
    }
    return 0;
}

void ConfigSite::run()
{
    uint32_t beg = millis();
    start();

    while (!tick())
    {
        if (millis() - beg > _lifeTime)
        {
            _status = 5;
            break;
        }
        yield();
    }
    stop();
}

// статус: 1 connect, 2 ap, 3 local, 4 exit, 5 timeout
uint8_t ConfigSite::portalStatus()
{
    return _status;
}

#endif
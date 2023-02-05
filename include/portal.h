/*
    Простой менеджер WiFi для esp8266 для задания логина-пароля WiFi и режима работы
    GitHub: https://github.com/GyverLibs/SimplePortal
    
    AlexGyver, alex@alexgyver.ru
    https://alexgyver.ru/
    MIT License

    Версии:
    v1.0
    v1.1 - совместимость с ESP32
*/

#include <Arduino.h>

// #define SP_AP_NAME "ESP Config"     // название точки
// #define SP_AP_IP 192,168,100,1        // IP точки

// #include <DNSServer.h>
// #include <ESP8266WiFi.h>
// #include <ESP8266WebServer.h>


// #define SP_ERROR 0
// #define SP_SUBMIT 1
// #define SP_SWITCH_AP 2
// #define SP_SWITCH_LOCAL 3
// #define SP_EXIT 4
// #define SP_TIMEOUT 5



void portalRun(uint32_t prd);   // блокирующий вызов
byte portalStatus();    // статус: 1 connect, 2 ap, 3 local, 4 exit, 5 timeout


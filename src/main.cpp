#include <Arduino.h>

uint16_t sizeEEPROM = 512;

#include <WiFiController.h>
WiFiController ctrl(0); // 0 - начальный адрес в памяти для WiFi настроек

#include "UdpInformer.h"
UdpInformer informer(sizeEEPROM); 

void setup()
{
    Serial.begin(115200);
    delay(500);

    EEPROM.begin(sizeEEPROM);

    // Признак первого запуска
    bool isFirstTime = EEPROM[128] != 0x22;

    Serial.printf("isFirstTime: %d\n", isFirstTime);

    ctrl.connect(isFirstTime);
    informer.start(2222, isFirstTime);

    // после инициализации подключения и старта информера
    // EEPROM проинициализирована, поэтому сброс флага первого запуска
    if(isFirstTime){
        EEPROM[128] = 0x22;
        EEPROM.commit();
    }
}

int n = 0;

void loop()
{
    // проверка WiFi и если требуется - подключение
    ctrl.tick();
    // опрос приема UDP пакетов
    informer.tick();


    delay(10);
    n++;
    if (n > 500)
    {
        n = 0;
        // informer.broadcast("wake up!");
    }
}

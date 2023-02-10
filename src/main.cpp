#include <Arduino.h>

uint16_t sizeEEPROM = 512;

#include "WiFiController.h"
WiFiController ctrl(0); // 0 - начальный адрес в памяти для WiFi настроек

#include "UdpInformer.h"
UdpInformer informer(sizeEEPROM); 

char incomingPacket[255];
char replyPacekt[] = "OK";

#include "DicList.h"
DicList dic;

void printDicItem(const char *id, const char *data, int index)
{
    Serial.printf("%s:%s\n", id, data);
}

void testDic()
{
    dic.set("one", "First value");
    dic.set("two", "Second value");
    dic.set("three", "Third value");
    dic.set("four", "Four value");
    Serial.printf("one: %s\n", dic.get("one"));
    Serial.printf("three: %s\n", dic.get("three"));
    dic.set("one", "Other value");
    Serial.printf("one: %s\n", dic.get("one"));
    dic.forEach(&printDicItem);
}

void testInformer()
{
    // char sour1[20] = "  a  hello b  1  ";
    // char sour2[20] = "";
    // char sour3[20] = "     ";
    // char dest[20];

    // informer.normalize(sour1, dest);
    // Serial.printf("sour1.normalize: [%s]\n", dest);
    // informer.normalize(sour2, dest);
    // Serial.printf("sour2.normalize: [%s]\n", dest);
    // informer.normalize(sour3, dest);
    // Serial.printf("sour3.normalize: [%s]\n", dest);
}

void setup()
{
    Serial.begin(115200);
    delay(500);

    // testDic();
    // testInformer();

    EEPROM.begin(sizeEEPROM);

    // Признак первого запуска
    bool isFirstTime = EEPROM[128] != 0x22;

    ctrl.connect(isFirstTime);
    informer.start(2222, isFirstTime);

    // после инициализации подключения и старта информера
    // EEPROM проинициализирована, поэтому сброс флага первого запуска
    if(isFirstTime){
        // EEPROM[128] = 0x22;
    }

//    dc:4f:22:2c:32:c6
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

#include <Arduino.h>

uint16_t sizeEEPROM = 512;

#include <cppQueue.h>
cppQueue q(sizeof(String *), 5, FIFO); // Instantiate queue

#include <WiFiController.h>
WiFiController ctrl;

#include <EspNowConnector.h>
EspNowConnector conn;

#include "UdpInformer.h"
UdpInformer informer;

#include <FastBot.h>
FastBot bot;

struct TBotCfg
{
    char token[60] = ""; // 60 - размер токена с запасом
    char chatId[16] = "";
} botCfg;

void loadBotCfg()
{
    uint16_t addr = ctrl.useEEPROMSize() + 2;
    EEPROM.get(addr, botCfg);
}

void saveBotCfg()
{
    uint16_t addr = ctrl.useEEPROMSize() + 2;
    EEPROM.put(addr, botCfg);
    EEPROM.commit();
}

// сброс настроек: сохранение корректных, но пустых настроек
void reset()
{
    uint8_t label = 0x00;
    EEPROM.get(0, label);
    if (label == 0x22)
    {
        EEPROM.put(0, 0xff);
        EEPROM.commit();
    }
    saveBotCfg();
}

void onTlgToken(const char *token)
{
    Serial.printf("Tlg token: %s\n", token);
    strcpy(botCfg.token, token);
    saveBotCfg();
}

void onTlgGroup(const char *group)
{
    Serial.printf("Tlg group: %s\n", group);
    strcpy(botCfg.chatId, group);
    saveBotCfg();
}

// обработчик сообщений
void onTlgMsg(FB_msg &msg)
{
    //   выводим имя юзера и текст сообщения
    //   Serial.print(msg.username);
    //   Serial.print(", ");
    Serial.println(msg.text);

    // выводим всю информацию о сообщении
    // Serial.println(msg.toString());
}

void OnEspNowSent(uint8_t *mac, uint8_t status)
{
    Serial.printf("%s\n", status == 0 ? "success" : "fail");
}

String text;
void OnEspNowMsg(uint8_t *mac, uint8_t *data, uint8_t len)
{
    char addr[18];
    ATools::macToChars(mac, addr);

    // char *msg = (char *)data;
    // text = String(msg);
    // q.push(&text);

    char *msg = (char *)malloc(len);
    memcpy(msg, data, len);
    q.push(&msg);
    Serial.printf("ADDR:%s\tMSG:%s\n", addr, text.c_str());
    // bot.sendMessage(text);
}

void setup()
{
    Serial.begin(115200);
    delay(500);

    EEPROM.begin(sizeEEPROM);

    // reset();

    // Признак первого запуска
    bool isFirstTime = EEPROM[0] != 0x22;
    Serial.printf("isFirstTime: %d\n", isFirstTime);
    if (isFirstTime)
        reset();

    // подключение к WiFi
    ctrl.connect(isFirstTime, WIFI_AP_STA);

    Serial.printf("WiFi channel: %d\n", WiFi.channel());

    // после инициализации подключения и старта информера
    // EEPROM проинициализирована, поэтому сброс флага первого запуска
    if (isFirstTime)
    {
        EEPROM[0] = 0x22;
        EEPROM.commit();
    }

    // инициализация стека ESP-NOW
    conn.start();
    // сообщение по подписке, возможно только алерт
    conn.setReceiveCallback(OnEspNowMsg);
    conn.setSendCallback(OnEspNowSent);

    // инициализация телеграм-бота
    loadBotCfg();
    Serial.printf("botCfg: {%s, %s}\n", botCfg.chatId, botCfg.token);
    bot.setToken(botCfg.token);
    bot.setChatID(botCfg.chatId);
    bot.attach(onTlgMsg);
    bot.sendMessage("started");

    // запуск UDP регистратора инфраструктуры
    bool res = informer.start(2222);
    // настройка бота происходит через UDP
    // UDP назначение токена бота
    informer.setTokenCallback(onTlgToken);
    // UDP назначение ID чата
    informer.setGroupCallback(onTlgGroup);

    Serial.printf("UDPInformer started: %s\n", res ? "true" : "false");
}

int sub_alert_timeout = 30000;

void loop()
{
    // проверка WiFi и если требуется - подключение
    ctrl.tick();
    // опрос приема UDP пакетов
    informer.tick();
    // проверка новых сообщений бота
    bot.tick();

    int i = 0;
    while (!q.isEmpty())
    {
        char *msg;
        q.pop(&msg);
        Serial.printf("%d recv: %s\n", ++i, msg);
        bot.sendMessage(msg);
        free(msg);

        // q.pop(&text);
        // Serial.printf("%d recv: %s\n", ++i, text.c_str());
        // bot.sendMessage(text.c_str());
        // text.clear();

        Serial.printf("heap: %d\n", ESP.getFreeHeap());
    }

    delay(10);
}

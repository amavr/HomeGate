#include <Arduino.h>

uint16_t sizeEEPROM = 512;

#include <WiFiController.h>
WiFiController ctrl;

#include "UdpInformer.h"
UdpInformer informer(sizeEEPROM);

#include <FastBot.h>
FastBot bot;

struct TBotCfg
{
    // размер с запасом
    char token[60] = "";
    char chatId[16] = "-833347396";
} botCfg;

void loadBotCfg()
{
    uint16_t addr = ctrl.useEEPROMSize();
    EEPROM.get(addr, botCfg);
}

void saveBotCfg()
{
    uint16_t addr = ctrl.useEEPROMSize();
    EEPROM.put(addr, botCfg);
    EEPROM.commit();
}

// сброс настроек
void reset()
{
    EEPROM[128] = 0xff;
    EEPROM.commit();
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
    //   Serial.println(msg.text);

    //   выводим всю информацию о сообщении
    Serial.println(msg.toString());
}

void onAlert(const char *text)
{
    Serial.printf("Alert: %s\n", text);
}

void setup()
{
    Serial.begin(115200);
    delay(500);

    EEPROM.begin(sizeEEPROM);

    // reset();

    // Признак первого запуска
    bool isFirstTime = EEPROM[128] != 0x22;
    Serial.printf("isFirstTime: %d\n", isFirstTime);

    // подключение к WiFi
    ctrl.connect(isFirstTime);

    // запуск UDP регистратора инфраструктуры
    informer.setTokenCallback(onTlgToken);
    informer.setGroupCallback(onTlgGroup);
    informer.setAlertCallback(onAlert);
    bool res = informer.start(2222);
    Serial.printf("UDPInformer started:%s\n", res ? "Y" : "N");

    // после инициализации подключения и старта информера
    // EEPROM проинициализирована, поэтому сброс флага первого запуска
    if (isFirstTime)
    {
        EEPROM[128] = 0x22;
        EEPROM.commit();
    }

    loadBotCfg();
    Serial.printf("botCfg: {%s, %s}\n", botCfg.chatId, botCfg.token);
    bot.setToken(botCfg.token);
    bot.setChatID(botCfg.chatId);
    bot.attach(onTlgMsg);
    bot.sendMessage("started");
}

void loop()
{
    // проверка WiFi и если требуется - подключение
    ctrl.tick();
    // опрос приема UDP пакетов
    informer.tick();
    // проверка новых сообщений бота
    bot.tick();

    delay(10);
}

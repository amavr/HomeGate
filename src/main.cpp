#include <Arduino.h>

uint16_t sizeEEPROM = 512;

#include <WiFiController.h>
WiFiController ctrl;

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

void OnEspNowMsg(uint8_t *mac, uint8_t *data, uint8_t len)
{
    char addr[18];
    ATools::macToChars(mac, addr);
    Serial.printf("ADDR:%s\tMSG:%s\n", addr, (char*)data);
}

void setup()
{
    Serial.begin(115200);
    delay(500);

    EEPROM.begin(sizeEEPROM);

    // Признак первого запуска
    bool isFirstTime = EEPROM[0] != 0x22;
    Serial.printf("isFirstTime: %d\n", isFirstTime);
    if (isFirstTime)
        reset();

    // подключение к WiFi
    ctrl.connect(isFirstTime);

    // запуск UDP регистратора инфраструктуры
    informer.setTokenCallback(onTlgToken);
    informer.setGroupCallback(onTlgGroup);
    informer.setAlertCallback(onAlert);
    informer.setEspMsgCallback(OnEspNowMsg);
    
    bool res = informer.start(2222);
    Serial.printf("UDPInformer started:%s\n", res ? "Y" : "N");

    // после инициализации подключения и старта информера
    // EEPROM проинициализирована, поэтому сброс флага первого запуска
    if (isFirstTime)
    {
        EEPROM[0] = 0x22;
        EEPROM.commit();
    }

    loadBotCfg();
    Serial.printf("botCfg: {%s, %s}\n", botCfg.chatId, botCfg.token);
    bot.setToken(botCfg.token);
    bot.setChatID(botCfg.chatId);
    bot.attach(onTlgMsg);
    bot.sendMessage("started");
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

    delay(10);
}

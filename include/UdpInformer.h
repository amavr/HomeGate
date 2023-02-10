#pragma once

#include <ESP8266WiFi.h>
// #include <ESPAsyncUDP.h>
#include <WiFiUdp.h>

#include "DicList.h"

// в этом адресе хранится кол-во элементов словаря
// сами элементы начинаются со 130 адреса
const uint16_t BEG_EEPROM_DIC_DATA = 129;

char packetBuffer[255];     // buffer to hold incoming packet
char ReplyBuffer[] = "ack"; // a string to send back

// структура хранения элемента словаря в EEPROM
struct EEDicItem
{
    char role[20];
    char addr[12];
};

static int stupid_pos = 0;
void cbStupidSaveDicItem(const char *id, const char *data, int index)
{
    EEDicItem item;
    strcpy(item.role, id);
    strcpy(item.addr, data);
    EEPROM.put(stupid_pos, item);
    stupid_pos += sizeof(item);
}

// предоставление mac-адресов основных узлов остальным устройствам
// через UDP для будущего общения через ESP-NOW
class UdpInformer
{
private:
    // AsyncUDP udp;
    WiFiUDP udp;
    DicList *dic;
    uint16_t beg_eeprom;
    uint16_t eeprom_dic_size;

    void reset()
    {
        for (uint16_t i = beg_eeprom; i < eeprom_dic_size; i++)
        {
            EEPROM[i] = 0x00;
        }
        EEPROM.commit();
    }

    void loadDic()
    {
        EEDicItem item;
        uint8_t count = EEPROM[BEG_EEPROM_DIC_DATA];
        uint16_t save_pos = BEG_EEPROM_DIC_DATA + 1;

        for (int i = 0; i < count; i++)
        {
            EEPROM.get(save_pos, item);
            save_pos += sizeof(item);
            dic->set(item.role, item.addr);
        }
    }

    // нещадящее (пока) сохранение всех элементов коллекции
    // надо сделать запись только измененных элементов
    // костыль - сохранение через внешнюю функцию!!!
    void saveDic()
    {
        uint8_t count = (uint8_t)dic->Count();
        EEPROM[BEG_EEPROM_DIC_DATA] = count;
        stupid_pos = BEG_EEPROM_DIC_DATA + 1;
        dic->forEach(&cbStupidSaveDicItem);
        EEPROM.commit();
    }


    bool isCmd(const char *cmd, char *data)
    {
        size_t cmd_len = strlen(cmd);
        size_t data_len = strlen(data);
        // команда длиннее, чем анализируемый текст
        // т.е. явно не та команда 
        if(data_len < cmd_len)
        {
            return false;
        }

        // длина команды не менее 3 символов?
        if (cmd_len >= 3)
        {
            // команда совпала?
            if(strncmp(cmd, data, cmd_len) == 0)
            {
                // забрать параметры
                // нет параметров?
                if(cmd_len == data_len)
                {
                    data[0] = '\0';
                }
                else
                {
                    size_t prm_len = data_len - cmd_len;
                    strncpy(data, data + cmd_len, prm_len);
                    data[prm_len] = '\0';
                }

                return true;
            }
        }

        return false;
    }

    void onReceive(int size)
    {
        IPAddress remoteIp = udp.remoteIP();
        uint16_t port = udp.localPort();

        char buf[255];

        int len = udp.read(buf, 255);
        buf[len] = '\0';

        char chars[255];
        normalize(buf, chars);
        Serial.println(chars);

        if(isCmd("set role ", chars))
        {
            Serial.printf("set role: [%s]\n", chars);
        }

        if(isCmd("get role ", chars))
        {
            Serial.printf("get role: [%s]\n", chars);
        }

        char ans[] = "ack";
        udp.beginPacket(udp.remoteIP(), udp.localPort());
        udp.write(ans);
        udp.endPacket();
    }

public:
    UdpInformer(uint16_t sizeEEPROM)
    {
        dic = new DicList;
        eeprom_dic_size = sizeEEPROM;
    }

    // нормализация пробелов + trim
    void normalize(const char *sour, char *dest)
    {
        int sour_len = strlen(sour);
        if (sour_len == 0)
        {
            dest[0] = '\0';
            return;
        }

        int di = 0;
        bool prev_space = true;
        for (int si = 0; si < sour_len; si++)
        {
            // обработка пробелов
            if (sour[si] == ' ')
            {
                // ранее было не начало строки и не пробел?
                if (!prev_space)
                {
                    // значимый пробел (после непробельного символа)
                    dest[di++] = sour[si];
                    // но след.пробел надо пропустить
                    prev_space = true;
                }
            }
            // пришел не пробел
            else
            {
                dest[di++] = sour[si];
                prev_space = false;
            }
        }

        dest[di] = '\0';
        // на конце может оставаться значимый пробел
        while(di > 0 && dest[--di] == ' ')
        {
            dest[di] = '\0';
        }        
    }

    bool start(int port, bool isFirstTime)
    {
        if (false)
        {
            if (isFirstTime)
            {
                reset();
            }
            loadDic();
        }
        return udp.begin(port) == 1;
    }

    void tick()
    {
        int packetSize = udp.parsePacket();
        if (packetSize)
        {
            onReceive(packetSize);
        }
    }

    void broadcast(const char *text)
    {
        IPAddress addr = WiFi.localIP();
        addr[3] = 0xff;
        udp.beginPacket(addr, udp.localPort());
        udp.print(text);
        udp.endPacket();
    }

    void addResource(const char *id, const char *addr)
    {
        dic->set(id, addr);
    }

    const char *getResourceAddr(const char *resourceId)
    {
        return dic->get(resourceId);
    }
};
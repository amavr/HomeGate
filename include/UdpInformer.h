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

    // нормализация 
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
            // пропуск двойных пробелов
            if (sour[si] == ' ')
            {
                // ранее было начало строки или пробел?
                if (prev_space)
                {
                    continue;
                }
                else
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
        // компенсация последнего инкремента
        di--;
        if (di < 0)
        {
            dest[0] = '\0';
            return;
        }

        int dest_len = strlen(dest);
        // последний символ - пробел?
        if (dest_len > 0 && dest[dest_len - 1] == ' ')
        {
            // перенести конец строки влево
            di--;
        }
        // установить конец строки
        dest[di] = '\0';
    }

public:
    UdpInformer(uint16_t sizeEEPROM)
    {
        dic = new DicList;
        eeprom_dic_size = sizeEEPROM;
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
            IPAddress remoteIp = udp.remoteIP();

            Serial.print(remoteIp);
            Serial.print(":");
            Serial.print(udp.remotePort());

            Serial.print(" sent [");
            Serial.print(packetSize);
            Serial.print("] ");

            // read the packet into packetBufffer
            int len = udp.read(packetBuffer, 255);
            if (len > 0)
            {
                packetBuffer[len] = 0;
            }
            Serial.println(packetBuffer);

            // send a reply, to the IP address and port that sent us the packet we received
            // udp.beginPacket(udp.remoteIP(), udp.remotePort());
            udp.beginPacket(udp.remoteIP(), udp.localPort());
            udp.write(ReplyBuffer);
            udp.endPacket();
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
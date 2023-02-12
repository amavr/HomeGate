#pragma once

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <GParser.h>

#include "DicList.h"

// предоставление mac-адресов основных узлов остальным устройствам
// через UDP для будущего общения через ESP-NOW
class UdpInformer
{
private:
    WiFiUDP udp;
    DicList *dic;

    // Callbacks
    // Telegram Token changed
    void (*cbReceiveToken)(const char *token);
    // Telegram Group changed
    void (*cbReceiveGroup)(const char *group);
    // Send alert
    void (*cbAlert)(const char *token);

    void onToken(const char *token)
    {
        if (cbReceiveToken != NULL)
            cbReceiveToken(token);
    }

    void onGroup(const char *group)
    {
        if (cbReceiveGroup != NULL)
            cbReceiveGroup(group);
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
        while (di > 0 && dest[--di] == ' ')
        {
            dest[di] = '\0';
        }
    }

    bool isCmd(
        const char *cmd,  // команда с которой происходит сравнение
        const char *data, // принятый текст
        char *rest        // выделенные здесь параметры команды
    )
    {
        size_t cmd_len = strlen(cmd);
        size_t data_len = strlen(data);
        // команда длиннее, чем анализируемый текст
        // т.е. явно не та команда
        if (data_len < cmd_len)
        {
            return false;
        }

        // длина команды не менее 3 символов?
        if (cmd_len >= 3)
        {
            // команда совпала?
            if (strncmp(cmd, data, cmd_len) == 0)
            {
                // забрать параметры
                // нет параметров?
                if (cmd_len == data_len)
                {
                    rest[0] = '\0';
                }
                else
                {
                    // get role broker
                    // 6 = 15 - 9
                    size_t prm_len = data_len - cmd_len;
                    //
                    strncpy(rest, data + cmd_len, prm_len);
                    rest[prm_len] = '\0';
                }

                return true;
            }
        }

        return false;
    }

    void response(const char *answer, IPAddress host, uint16_t port)
    {
        udp.beginPacket(host, port);
        udp.write(answer);
        udp.endPacket();
    }

    void onReceive(int size)
    {
        IPAddress remoteIp = udp.remoteIP();
        uint16_t port = udp.localPort();

        char buf[255];

        int len = udp.read(buf, 255);
        buf[len] = '\0';

        char text[255];
        normalize(buf, text);
        Serial.println(text);

        if (isCmd("set role ", text, buf))
        {
            GParser data(buf, ' ');
            int count = data.split();
            if (count == 2)
            {
                dic->set(data[0], data[1]);
                response(data[1], remoteIp, port);

                // если регистрируется брокер,
                // то надо подписаться на тему уведомлений
                if (strcmp(data[0], "broker") == 0)
                {
                }
            }
            else
            {
                sprintf(buf, "use: set role <name> <macaddr>");
                response(buf, remoteIp, port);
            }
        }

        if (isCmd("get role ", text, buf))
        {
            GParser data(buf, ' ');
            int count = data.split();
            if (count == 1)
            {
                const char *val = dic->get(buf);
                if (val == NULL)
                {
                    response("\0", remoteIp, port);
                }
                else
                {
                    response(val, remoteIp, port);
                }
            }
            else
            {
                sprintf(buf, "use: get role <name>");
                response(buf, remoteIp, port);
            }
        }

        if (isCmd("set tlg-token ", text, buf))
        {
            GParser data(buf, ' ');
            int count = data.split();
            if (count == 1 && strlen(text) > 0)
            {
                onToken(buf);
                response(buf, remoteIp, port);
            }
            else
            {
                sprintf(buf, "use: set tlg-token <token>");
                response(buf, remoteIp, port);
            }
        }

        if (isCmd("set tlg-group ", text, buf))
        {
            GParser data(buf, ' ');
            int count = data.split();
            if (count == 1 && strlen(text) > 0)
            {
                onGroup(buf);
                response(buf, remoteIp, port);
            }
            else
            {
                sprintf(buf, "use: set tlg-group <group>");
                response(buf, remoteIp, port);
            }
        }
    }

public:
    UdpInformer(uint16_t sizeEEPROM)
    {
        dic = new DicList;
    }

    // установка колбэков
    // смена токена телеграма
    void setTokenCallback(void (*userDefinedCallback)(const char *token))
    {
        cbReceiveToken = userDefinedCallback;
    }

    // смена группы телеграма
    void setGroupCallback(void (*userDefinedCallback)(const char *token))
    {
        cbReceiveGroup = userDefinedCallback;
    }

    // передача уведомления
    void setAlertCallback(void (*userDefinedCallback)(const char *token))
    {
        cbAlert = userDefinedCallback;
    }

    bool start(int port)
    {
        bool started = udp.begin(port) == 1;
        broadcast("wakeup");
        return started;
    }

    void tick()
    {
        int packetSize = udp.parsePacket();
        if (packetSize)
        {
            onReceive(packetSize);
        }
    }

    void alert(const char *text)
    {
        if (cbAlert != NULL)
            cbAlert(text);
    }

    void broadcast(const char *text)
    {
        IPAddress addr = WiFi.localIP();
        addr[3] = 0xff;
        udp.beginPacket(addr, udp.localPort());
        udp.print(text);
        udp.endPacket();
    }
};
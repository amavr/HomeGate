#pragma once

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif

#include <WiFiUdp.h>
#include <GParser.h>
#include <DicList.h>
#include <ATools.h>
#include <EspNowConnector.h>

#include <DicFile.h>

// предоставление mac-адресов основных узлов остальным устройствам
// через UDP для будущего общения через ESP-NOW
class UdpInformer
{
private:
    WiFiUDP udp;
    EspNowConnector conn;

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
        ATools::normalize(buf, text);
        Serial.println(text);

        if (ATools::isCmd("set role ", text, buf))
        {
            GParser data(buf, ' ');
            int count = data.split();
            if (count == 2)
            {
                Serial.printf("[%s][%s]\n", data[0], data[1]);
                dic->set(data[0], data[1]);
                response(data[1], remoteIp, port);

                DicFile::save("/dic.txt", dic);

                // если регистрируется брокер,
                // то надо подписаться на тему уведомлений
                if (strcmp(data[0], "broker") == 0)
                {
                    // сопряжение с брокером
                    conn.pair(data[1]);
                    // отправка происходите через ESP-NOW
                    conn.send(data[1], "sub alert");
                }
            }
            else
            {
                sprintf(buf, "use: set role <name> <macaddr>");
                response(buf, remoteIp, port);
            }
        }
        else if (ATools::isCmd("get role ", text, buf))
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
        else if (ATools::isCmd("set tlg-token ", text, buf))
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
        else if (ATools::isCmd("set tlg-group ", text, buf))
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

    uint32_t last_time = 0;

public:
    DicList *dic;

    UdpInformer()
    {
        dic = new DicList;
        // DicFile::remove("/dic.txt");
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

    void setEspMsgCallback(esp_now_recv_cb_t onMsg)
    {
        conn.setReceiveCallback(onMsg);
    }

    bool start(int port)
    {
        Serial.println("Resources loading");
        DicFile::load("/dic.txt", dic);
        dic->set("gate", WiFi.macAddress().c_str());
        return udp.begin(port) == 1;
    }

    void tick()
    {
        // if (millis() - last_time > 60000 || millis() < last_time)
        // {
        //     const char *brokerMac = dic->get("broker");
        //     if (brokerMac != NULL)
        //     {
        //         conn.pair(brokerMac);
        //         conn.send(brokerMac, "sub alert");
        //     }

        //     last_time = millis();
        // }

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
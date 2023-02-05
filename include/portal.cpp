#include "portal.h"
// static DNSServer _SP_dnsServer;
// static ESP8266WebServer _SP_server(80);

struct PortalCfg {
  char SSID[32] = "";
  char pass[32] = "";
//   uint8_t mode = WIFI_AP;    // (1 WIFI_STA, 2 WIFI_AP)
} portalCfg;
// extern PortalCfg portalCfg;


void portalRun(uint32_t prd)
{
    Serial.printf("portalRun(%d)\n", prd);
}

byte portalStatus()
{
    return 0;
}
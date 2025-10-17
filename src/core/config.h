#ifndef CONFIG_H
#define CONFIG_H

#include "../../secrets.h"

namespace Config {
  struct WiFiConfig {
    const char* ssid = WIFI_SSID;
    const char* password = WIFI_PASSWORD;
  };
  
  struct MQTTConfig {
    const char* server = MQTT_SERVER;
    const uint16_t port = MQTT_PORT;
    char clientId[24];
    const uint16_t bufferSize = 512;
  };
  
  struct ScanConfig {
    char networkPrefix[20] = "192.168.0.";
    uint8_t startIp = 1;
    uint8_t endIp = 254;
    uint16_t startPort = 1;
    uint16_t endPort = 1024;
    uint16_t timeoutMs = 100;
    uint32_t intervalMs = 300000;
    uint8_t workerThreads = 12;
    uint8_t queueSize = 200;
    bool enableMQTT = false;
    bool enableBannerGrab = true;
    bool skipHostCheck = true;
  };
  
  extern WiFiConfig wifi;
  extern MQTTConfig mqtt;
  extern ScanConfig scan;
}

#endif

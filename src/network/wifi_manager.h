#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "../core/interfaces.h"
#include "../core/config.h"

class WiFiManager : public IConnectionManager {
private:
  const Config::WiFiConfig& config;
  uint8_t maxRetries;
  
public:
  WiFiManager(const Config::WiFiConfig& cfg, uint8_t retries = 20) 
    : config(cfg), maxRetries(retries) {}
  
  bool connect() override {
    if (WiFi.status() == WL_CONNECTED) return true;
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(config.ssid, config.password);
    Serial.printf("\n[WiFi] Connecting to %s", config.ssid);
    
    uint8_t attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < maxRetries) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
      Serial.printf("\n[WiFi] Connected: %s (RSSI: %d dBm)\n", 
        WiFi.localIP().toString().c_str(), WiFi.RSSI());
      return true;
    }
    
    Serial.println("\n[WiFi] Connection failed!");
    return false;
  }
  
  bool isConnected() override {
    return WiFi.status() == WL_CONNECTED;
  }
  
  void disconnect() override {
    WiFi.disconnect();
  }
  
  bool reconnect() {
    Serial.println("[WiFi] Reconnecting...");
    WiFi.mode(WIFI_STA);
    delay(100);
    return connect();
  }
  
  int8_t getSignalStrength() const {
    return WiFi.RSSI();
  }
};

#endif

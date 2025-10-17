#ifndef WIFI_SCANNER_H
#define WIFI_SCANNER_H

#include <WiFi.h>
#include <vector>
#include "../core/data_structures.h"

class WiFiScanner {
private:
  std::vector<WiFiNetwork> networks;
  SemaphoreHandle_t mutex;
  uint32_t lastScan;
  
public:
  WiFiScanner() {
    mutex = xSemaphoreCreateMutex();
    lastScan = 0;
  }
  
  ~WiFiScanner() {
    if (mutex) vSemaphoreDelete(mutex);
  }
  
  void scan() {
    int n = WiFi.scanNetworks();
    
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      networks.clear();
      
      for (int i = 0; i < n; i++) {
        WiFiNetwork net;
        strncpy(net.ssid, WiFi.SSID(i).c_str(), sizeof(net.ssid) - 1);
        memcpy(net.bssid, WiFi.BSSID(i), 6);
        net.rssi = WiFi.RSSI(i);
        net.channel = WiFi.channel(i);
        net.encryption = WiFi.encryptionType(i);
        net.isHidden = (WiFi.SSID(i).length() == 0);
        net.lastSeen = millis();
        networks.push_back(net);
      }
      
      lastScan = millis();
      xSemaphoreGive(mutex);
    }
    
    WiFi.scanDelete();
    Serial.printf("[WiFi] Scan complete: %d networks found\n", n);
  }
  
  String toJson() const {
    DynamicJsonDocument doc(4096);
    JsonArray arr = doc.to<JsonArray>();
    
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      for (const auto& net : networks) {
        JsonObject obj = arr.createNestedObject();
        obj["ssid"] = net.isHidden ? "<Hidden>" : net.ssid;
        
        char bssidStr[18];
        snprintf(bssidStr, sizeof(bssidStr), "%02X:%02X:%02X:%02X:%02X:%02X",
          net.bssid[0], net.bssid[1], net.bssid[2], 
          net.bssid[3], net.bssid[4], net.bssid[5]);
        obj["bssid"] = bssidStr;
        
        obj["rssi"] = net.rssi;
        obj["channel"] = net.channel;
        
        const char* encType = "Unknown";
        switch(net.encryption) {
          case WIFI_AUTH_OPEN: encType = "Open"; break;
          case WIFI_AUTH_WEP: encType = "WEP"; break;
          case WIFI_AUTH_WPA_PSK: encType = "WPA"; break;
          case WIFI_AUTH_WPA2_PSK: encType = "WPA2"; break;
          case WIFI_AUTH_WPA_WPA2_PSK: encType = "WPA/WPA2"; break;
          case WIFI_AUTH_WPA2_ENTERPRISE: encType = "WPA2-Enterprise"; break;
          case WIFI_AUTH_WPA3_PSK: encType = "WPA3"; break;
        }
        obj["encryption"] = encType;
        obj["isOpen"] = (net.encryption == WIFI_AUTH_OPEN);
        obj["isWeak"] = (net.encryption == WIFI_AUTH_WEP || net.encryption == WIFI_AUTH_OPEN);
      }
      xSemaphoreGive(mutex);
    }
    
    String output;
    serializeJson(doc, output);
    return output;
  }
  
  size_t count() const {
    return networks.size();
  }
};

#endif

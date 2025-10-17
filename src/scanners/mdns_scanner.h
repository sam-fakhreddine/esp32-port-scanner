#ifndef MDNS_SCANNER_H
#define MDNS_SCANNER_H

#include <ESPmDNS.h>
#include <vector>
#include <ArduinoJson.h>
#include "../core/data_structures.h"

class MDNSScanner {
private:
  std::vector<MDNSService> services;
  SemaphoreHandle_t mutex;
  
  const char* commonServices[15] = {
    "_http", "_printer", "_ipp", "_airplay", "_googlecast",
    "_spotify-connect", "_homekit", "_hap", "_smb", "_afpovertcp",
    "_ssh", "_sftp-ssh", "_workstation", "_device-info", "_raop"
  };
  
public:
  MDNSScanner() {
    mutex = xSemaphoreCreateMutex();
  }
  
  ~MDNSScanner() {
    if (mutex) vSemaphoreDelete(mutex);
  }
  
  void scan() {
    Serial.println("[mDNS] Starting scan...");
    
    if (!MDNS.begin("esp32scanner")) {
      Serial.println("[mDNS] Init failed");
      return;
    }
    
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      services.clear();
      
      for (int i = 0; i < 15; i++) {
        Serial.printf("[mDNS] Querying %s._tcp...\n", commonServices[i]);
        int n = MDNS.queryService(commonServices[i], "tcp");
        Serial.printf("[mDNS] Found %d instances\n", n);
        
        for (int j = 0; j < n; j++) {
          MDNSService svc;
          strncpy(svc.hostname, MDNS.hostname(j).c_str(), sizeof(svc.hostname) - 1);
          strncpy(svc.service, commonServices[i], sizeof(svc.service) - 1);
          svc.port = MDNS.port(j);
          svc.ip = MDNS.address(j);
          services.push_back(svc);
          Serial.printf("[mDNS] %s at %s:%d\n", svc.hostname, svc.ip.toString().c_str(), svc.port);
        }
      }
      
      xSemaphoreGive(mutex);
    }
    
    Serial.printf("[mDNS] Scan complete: %d services found\n", services.size());
  }
  
  String toJson() const {
    DynamicJsonDocument doc(4096);
    JsonArray arr = doc.to<JsonArray>();
    
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      for (const auto& svc : services) {
        JsonObject obj = arr.createNestedObject();
        obj["hostname"] = svc.hostname;
        obj["service"] = svc.service;
        obj["port"] = svc.port;
        obj["ip"] = svc.ip.toString();
        
        const char* icon = "🔍";
        const char* desc = "Service";
        
        if (strstr(svc.service, "printer") || strstr(svc.service, "ipp")) {
          icon = "🖨️";
          desc = "Printer";
        } else if (strstr(svc.service, "airplay") || strstr(svc.service, "raop")) {
          icon = "📺";
          desc = "AirPlay";
        } else if (strstr(svc.service, "googlecast")) {
          icon = "📺";
          desc = "Chromecast";
        } else if (strstr(svc.service, "http")) {
          icon = "🌐";
          desc = "Web Server";
        } else if (strstr(svc.service, "ssh") || strstr(svc.service, "sftp")) {
          icon = "💻";
          desc = "SSH";
        } else if (strstr(svc.service, "smb") || strstr(svc.service, "afp")) {
          icon = "📁";
          desc = "File Share";
        } else if (strstr(svc.service, "spotify")) {
          icon = "🎵";
          desc = "Spotify";
        } else if (strstr(svc.service, "homekit") || strstr(svc.service, "hap")) {
          icon = "🏠";
          desc = "HomeKit";
        }
        
        obj["icon"] = icon;
        obj["description"] = desc;
      }
      xSemaphoreGive(mutex);
    }
    
    String output;
    serializeJson(doc, output);
    return output;
  }
  
  size_t count() const {
    return services.size();
  }
};

#endif

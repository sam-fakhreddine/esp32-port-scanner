#ifndef SMB_SCANNER_H
#define SMB_SCANNER_H

#include <WiFi.h>
#include <vector>
#include <ArduinoJson.h>

struct SMBShare {
  IPAddress ip;
  uint16_t port;
  bool nullSessionAllowed;
  String hostname;
  uint32_t discovered;
};

class SMBScanner {
private:
  std::vector<SMBShare> shares;
  SemaphoreHandle_t mutex;
  
  bool testNullSession(const char* ip, uint16_t port) {
    WiFiClient client;
    if (!client.connect(ip, port, 2000)) {
      return false;
    }
    
    // SMB Negotiate Protocol Request
    uint8_t negotiate[] = {
      0x00, 0x00, 0x00, 0x85, // NetBIOS Session
      0xFF, 0x53, 0x4D, 0x42, // SMB Header "\xFFSMB"
      0x72, // Negotiate Protocol
      0x00, 0x00, 0x00, 0x00, // Status
      0x18, 0x53, 0xC8, 0x00, // Flags
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0xFF, 0xFF, 0xFF, 0xFE, // TID
      0x00, 0x00, 0x00, 0x00, // PID
      0x00, 0x00, 0x00, 0x00, // UID
      0x00, 0x00, 0x00, 0x00, // MID
      0x00, // Word Count
      0x62, 0x00, // Byte Count
      0x02, 0x50, 0x43, 0x20, 0x4E, 0x45, 0x54, 0x57, 0x4F, 0x52, 0x4B,
      0x20, 0x50, 0x52, 0x4F, 0x47, 0x52, 0x41, 0x4D, 0x20, 0x31, 0x2E,
      0x30, 0x00, 0x02, 0x4C, 0x41, 0x4E, 0x4D, 0x41, 0x4E, 0x31, 0x2E,
      0x30, 0x00, 0x02, 0x57, 0x69, 0x6E, 0x64, 0x6F, 0x77, 0x73, 0x20,
      0x66, 0x6F, 0x72, 0x20, 0x57, 0x6F, 0x72, 0x6B, 0x67, 0x72, 0x6F,
      0x75, 0x70, 0x73, 0x20, 0x33, 0x2E, 0x31, 0x61, 0x00, 0x02, 0x4C,
      0x4D, 0x31, 0x2E, 0x32, 0x58, 0x30, 0x30, 0x32, 0x00, 0x02, 0x4C,
      0x41, 0x4E, 0x4D, 0x41, 0x4E, 0x32, 0x2E, 0x31, 0x00, 0x02, 0x4E,
      0x54, 0x20, 0x4C, 0x4D, 0x20, 0x30, 0x2E, 0x31, 0x32, 0x00
    };
    
    client.write(negotiate, sizeof(negotiate));
    delay(500);
    
    if (client.available() > 0) {
      uint8_t response[256];
      int len = client.read(response, sizeof(response));
      client.stop();
      
      // Check if SMB response received
      if (len > 8 && response[4] == 0xFF && response[5] == 0x53 && 
          response[6] == 0x4D && response[7] == 0x42) {
        // Check status code (bytes 9-12)
        uint32_t status = response[9] | (response[10] << 8) | 
                         (response[11] << 16) | (response[12] << 24);
        return (status == 0); // STATUS_SUCCESS
      }
    }
    
    client.stop();
    return false;
  }
  
public:
  SMBScanner() {
    mutex = xSemaphoreCreateMutex();
  }
  
  ~SMBScanner() {
    if (mutex) vSemaphoreDelete(mutex);
  }
  
  void scan(const std::vector<uint8_t>& arpIPs) {
    Serial.println("[SMB] Scanning ARP hosts for open shares...");
    
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      shares.clear();
      
      IPAddress localIP = WiFi.localIP();
      uint16_t smbPorts[] = {445, 139};
      
      for (uint8_t ip : arpIPs) {
        char ipStr[16];
        snprintf(ipStr, sizeof(ipStr), "%d.%d.%d.%d", 
                 localIP[0], localIP[1], localIP[2], ip);
        
        for (uint16_t port : smbPorts) {
          WiFiClient client;
          if (client.connect(ipStr, port, 1000)) {
            Serial.printf("[SMB] %s:%d - SMB port open\n", ipStr, port);
            
            bool nullSession = testNullSession(ipStr, port);
            
            SMBShare share;
            share.ip = IPAddress(localIP[0], localIP[1], localIP[2], ip);
            share.port = port;
            share.nullSessionAllowed = nullSession;
            share.hostname = "Unknown";
            share.discovered = millis();
            shares.push_back(share);
            
            if (nullSession) {
              Serial.printf("[SMB] ⚠️  %s:%d - NULL SESSION ALLOWED!\n", ipStr, port);
            }
            
            client.stop();
            break;
          }
        }
      }
      
      xSemaphoreGive(mutex);
    }
    
    Serial.printf("[SMB] Found %d SMB hosts (%d vulnerable)\n", 
                  shares.size(), 
                  std::count_if(shares.begin(), shares.end(), 
                               [](const SMBShare& s){ return s.nullSessionAllowed; }));
  }
  
  String toJson() const {
    DynamicJsonDocument doc(4096);
    JsonArray arr = doc.to<JsonArray>();
    
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      for (const auto& share : shares) {
        JsonObject obj = arr.createNestedObject();
        obj["ip"] = share.ip.toString();
        obj["port"] = share.port;
        obj["nullSession"] = share.nullSessionAllowed;
        obj["hostname"] = share.hostname;
        obj["discovered"] = share.discovered;
        obj["risk"] = share.nullSessionAllowed ? "CRITICAL" : "INFO";
      }
      xSemaphoreGive(mutex);
    }
    
    String output;
    serializeJson(doc, output);
    return output;
  }
  
  size_t count() const {
    return shares.size();
  }
  
  size_t vulnerableCount() const {
    int count = 0;
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      count = std::count_if(shares.begin(), shares.end(), 
                           [](const SMBShare& s){ return s.nullSessionAllowed; });
      xSemaphoreGive(mutex);
    }
    return count;
  }
};

#endif

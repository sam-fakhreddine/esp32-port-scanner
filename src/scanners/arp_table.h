#ifndef ARP_TABLE_H
#define ARP_TABLE_H

#include <WiFi.h>
#include <lwip/etharp.h>
#include <lwip/netif.h>
#include <lwip/tcpip.h>
#include <ArduinoJson.h>
#include <vector>

struct ARPEntry {
  IPAddress ip;
  uint8_t mac[6];
  uint32_t lastSeen;
};

class ARPTable {
private:
  std::vector<ARPEntry> entries;
  SemaphoreHandle_t mutex;
  
public:
  ARPTable() {
    mutex = xSemaphoreCreateMutex();
  }
  
  ~ARPTable() {
    if (mutex) vSemaphoreDelete(mutex);
  }
  
  void scan() {
    Serial.println("[ARP] Sending ARP requests...");
    
    struct netif* netif = netif_default;
    if (!netif) {
      Serial.println("[ARP] No network interface");
      return;
    }
    
    // Send ARP requests in batches and collect results
    IPAddress localIP = WiFi.localIP();
    Serial.printf("[ARP] Table size: %d\n", ARP_TABLE_SIZE);
    
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      entries.clear();
      
      // Scan in batches to work around ARP table size limit
      for (uint8_t batch = 0; batch < 26; batch++) {
        uint8_t start = batch * 10 + 1;
        uint8_t end = min(start + 9, 254);
        
        LOCK_TCPIP_CORE();
        for (uint8_t i = start; i <= end; i++) {
          ip4_addr_t target;
          IP4_ADDR(&target, localIP[0], localIP[1], localIP[2], i);
          etharp_request(netif, &target);
        }
        UNLOCK_TCPIP_CORE();
        
        delay(100);
        
        // Read entries from this batch
        for (int i = 0; i < ARP_TABLE_SIZE; i++) {
          ip4_addr_t* ip_addr;
          struct netif* ret_netif;
          struct eth_addr* eth_addr;
          
          if (etharp_get_entry(i, &ip_addr, &ret_netif, &eth_addr) == 1) {
            ARPEntry entry;
            entry.ip = IPAddress(ip4_addr_get_u32(ip_addr));
            
            // Check if already in list
            bool found = false;
            for (const auto& e : entries) {
              if (e.ip == entry.ip) {
                found = true;
                break;
              }
            }
            
            if (!found) {
              memcpy(entry.mac, eth_addr->addr, 6);
              entry.lastSeen = millis();
              entries.push_back(entry);
              
              Serial.printf("[ARP] %s -> %02X:%02X:%02X:%02X:%02X:%02X\n",
                entry.ip.toString().c_str(),
                entry.mac[0], entry.mac[1], entry.mac[2],
                entry.mac[3], entry.mac[4], entry.mac[5]);
            }
          }
        }
      }
      
      xSemaphoreGive(mutex);
    }
    
    Serial.printf("[ARP] Found %d entries\n", entries.size());
  }
  
  String toJson() const {
    DynamicJsonDocument doc(4096);
    JsonArray arr = doc.to<JsonArray>();
    
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      for (const auto& entry : entries) {
        JsonObject obj = arr.createNestedObject();
        obj["ip"] = entry.ip.toString();
        
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
          entry.mac[0], entry.mac[1], entry.mac[2],
          entry.mac[3], entry.mac[4], entry.mac[5]);
        obj["mac"] = macStr;
        
        // Identify vendor from OUI
        uint32_t oui = (entry.mac[0] << 16) | (entry.mac[1] << 8) | entry.mac[2];
        const char* vendor = "Unknown";
        
        if (oui == 0x001122 || oui == 0xB8E856) vendor = "Apple";
        else if (oui == 0x00E04C || oui == 0x7C2F80) vendor = "Realtek";
        else if (oui == 0x00D0C9 || oui == 0x001B63) vendor = "Intel";
        else if (oui == 0x00C0CA || oui == 0x001E58) vendor = "Cisco";
        else if (oui == 0x001CF0 || oui == 0x00259E) vendor = "D-Link";
        else if (oui == 0x001EA7 || oui == 0x00241D) vendor = "TP-Link";
        else if (oui == 0x00E0FC || oui == 0x001DD8) vendor = "Netgear";
        else if (oui == 0x00037F || oui == 0x001DD9) vendor = "Atheros";
        else if (oui == 0x00E098 || oui == 0x001E2A) vendor = "Broadcom";
        
        obj["vendor"] = vendor;
        obj["lastSeen"] = entry.lastSeen;
      }
      xSemaphoreGive(mutex);
    }
    
    String output;
    serializeJson(doc, output);
    return output;
  }
  
  size_t count() const {
    return entries.size();
  }
  
  std::vector<uint8_t> getActiveIPs() const {
    std::vector<uint8_t> ips;
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      for (const auto& entry : entries) {
        ips.push_back(entry.ip[3]);
      }
      xSemaphoreGive(mutex);
    }
    return ips;
  }
};

#endif

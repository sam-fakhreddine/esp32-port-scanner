#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <vector>
#include <Arduino.h>

struct ScanTask {
  uint8_t ip;
  uint16_t portStart;
  uint16_t portEnd;
  
  ScanTask(uint8_t i = 0, uint16_t ps = 0, uint16_t pe = 0) 
    : ip(i), portStart(ps), portEnd(pe) {}
};

struct ScanResult {
  uint8_t ip;
  uint16_t port;
  uint32_t timestamp;
  
  ScanResult(uint8_t i, uint16_t p) 
    : ip(i), port(p), timestamp(millis()) {}
};

struct RiskDetail {
  const char* port;
  const char* service;
  const char* severity;
  uint8_t points;
};

struct RiskFinding {
  String port;
  String service;
  String severity;
  uint8_t points;
};

struct EndpointData {
  uint8_t ip;
  std::vector<uint16_t> openPorts;
  char macAddress[18];
  char hostname[64];
  uint32_t firstSeen;
  uint32_t lastSeen;
  uint16_t scanCount;
  uint16_t avgResponseTime;
  char deviceType[32];
  uint8_t riskScore;
  std::vector<String> serviceBanners;
  std::vector<RiskFinding> riskFindings;
  
  EndpointData(uint8_t i) : ip(i), firstSeen(millis()), lastSeen(millis()), 
                            scanCount(1), avgResponseTime(0), riskScore(0) {
    strcpy(macAddress, "Unknown");
    strcpy(hostname, "Unknown");
    strcpy(deviceType, "Unknown");
  }
};

struct ScanStats {
  uint32_t totalScans;
  uint32_t totalIpsScanned;
  uint32_t totalPortsChecked;
  uint32_t totalOpenPorts;
  uint32_t uniqueHosts;
  uint32_t lastScanDuration;
  uint32_t avgScanDuration;
  
  ScanStats() : totalScans(0), totalIpsScanned(0), totalPortsChecked(0), 
                totalOpenPorts(0), uniqueHosts(0), lastScanDuration(0), avgScanDuration(0) {}
};

struct ScanProgress {
  char currentIP[16];
  uint16_t currentPort;
  uint32_t ipsScanned;
  uint32_t totalIPs;
  uint32_t portsScanned;
  uint32_t totalPorts;
  uint8_t percentComplete;
  
  ScanProgress() : currentPort(0), ipsScanned(0), totalIPs(0), portsScanned(0), totalPorts(0), percentComplete(0) {
    strcpy(currentIP, "");
  }
};

struct WiFiNetwork {
  char ssid[33];
  uint8_t bssid[6];
  int8_t rssi;
  uint8_t channel;
  uint8_t encryption;
  bool isHidden;
  uint32_t lastSeen;
  
  WiFiNetwork() : rssi(0), channel(0), encryption(0), isHidden(false), lastSeen(0) {
    strcpy(ssid, "");
    memset(bssid, 0, 6);
  }
};

struct BluetoothDevice {
  char name[32];
  uint8_t address[6];
  int8_t rssi;
  uint32_t firstSeen;
  uint32_t lastSeen;
  uint8_t addressType;
  uint16_t appearance;
  int8_t txPower;
  char manufacturer[64];
  uint16_t serviceUUIDs[8];
  uint8_t serviceCount;
  uint8_t advType;
  
  BluetoothDevice() : rssi(0), firstSeen(0), lastSeen(0), addressType(0), 
                      appearance(0), txPower(0), serviceCount(0), advType(0) {
    strcpy(name, "");
    strcpy(manufacturer, "");
    memset(address, 0, 6);
    memset(serviceUUIDs, 0, sizeof(serviceUUIDs));
  }
};

struct MDNSService {
  char hostname[64];
  char service[32];
  uint16_t port;
  IPAddress ip;
  
  MDNSService() : port(0), ip(0, 0, 0, 0) {
    strcpy(hostname, "");
    strcpy(service, "");
  }
};

#endif

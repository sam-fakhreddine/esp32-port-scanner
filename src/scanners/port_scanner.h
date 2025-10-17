#ifndef PORT_SCANNER_H
#define PORT_SCANNER_H

#include <WiFi.h>
#include <vector>
#include <algorithm>
#include "../core/interfaces.h"
#include "../core/data_structures.h"

class PortScanner : public IPortScanner {
public:
  bool scanPort(const char* ip, uint16_t port, uint16_t timeout) override {
    WiFiClient client;
    client.setTimeout(timeout);
    bool isOpen = client.connect(ip, port);
    if (isOpen) client.stop();
    return isOpen;
  }
  
  bool scanPortWithBanner(const char* ip, uint16_t port, uint16_t timeout, String& banner) override {
    WiFiClient client;
    client.setTimeout(timeout);
    if (!client.connect(ip, port)) return false;
    
    if (port == 80 || port == 8080) {
      client.print("HEAD / HTTP/1.0\r\n\r\n");
      delay(100);
      if (client.available()) {
        banner = client.readStringUntil('\n');
        if (banner.length() > 100) banner = banner.substring(0, 100);
      }
    } else {
      delay(50);
      if (client.available()) {
        banner = client.readStringUntil('\n');
        if (banner.length() > 100) banner = banner.substring(0, 100);
      }
    }
    
    client.stop();
    return true;
  }
  
  const char* identifyDevice(const std::vector<uint16_t>& ports) {
    bool hasHTTP = std::find(ports.begin(), ports.end(), 80) != ports.end();
    bool hasHTTPS = std::find(ports.begin(), ports.end(), 443) != ports.end();
    bool hasSSH = std::find(ports.begin(), ports.end(), 22) != ports.end();
    bool hasTelnet = std::find(ports.begin(), ports.end(), 23) != ports.end();
    bool hasRDP = std::find(ports.begin(), ports.end(), 3389) != ports.end();
    bool hasSMB = std::find(ports.begin(), ports.end(), 445) != ports.end();
    bool hasMQTT = std::find(ports.begin(), ports.end(), 1883) != ports.end();
    bool hasUPnP = std::find(ports.begin(), ports.end(), 1900) != ports.end();
    
    if (hasRDP && hasSMB) return "Windows PC";
    if (hasSSH && hasHTTP && ports.size() < 5) return "IoT Device";
    if (hasMQTT) return "IoT/MQTT Device";
    if (hasUPnP) return "Router/Gateway";
    if (hasTelnet && !hasSSH) return "Legacy Device";
    if (hasHTTP && hasHTTPS && ports.size() > 10) return "Server";
    if (hasSSH && ports.size() > 5) return "Linux Server";
    if (hasHTTP || hasHTTPS) return "Web Device";
    
    return "Unknown";
  }
  
  std::vector<RiskDetail> getRiskDetails(const std::vector<uint16_t>& ports) {
    std::vector<RiskDetail> details;
    
    if (std::find(ports.begin(), ports.end(), 23) != ports.end())
      details.push_back({"23", "Telnet", "Critical", 30});
    if (std::find(ports.begin(), ports.end(), 21) != ports.end())
      details.push_back({"21", "FTP", "High", 20});
    if (std::find(ports.begin(), ports.end(), 3389) != ports.end())
      details.push_back({"3389", "RDP", "High", 15});
    if (std::find(ports.begin(), ports.end(), 445) != ports.end())
      details.push_back({"445", "SMB", "High", 15});
    if (std::find(ports.begin(), ports.end(), 22) != ports.end())
      details.push_back({"22", "SSH", "Medium", 10});
    if (std::find(ports.begin(), ports.end(), 3306) != ports.end())
      details.push_back({"3306", "MySQL", "Medium", 10});
    if (std::find(ports.begin(), ports.end(), 5432) != ports.end())
      details.push_back({"5432", "PostgreSQL", "Medium", 10});
    if (std::find(ports.begin(), ports.end(), 80) != ports.end())
      details.push_back({"80", "HTTP", "Low", 5});
    if (std::find(ports.begin(), ports.end(), 443) != ports.end())
      details.push_back({"443", "HTTPS", "Low", 3});
    
    uint8_t portCountRisk = ports.size() * 2;
    if (portCountRisk > 0) {
      char buf[32];
      snprintf(buf, sizeof(buf), "%d open ports", (int)ports.size());
      details.push_back({"*", buf, "Info", portCountRisk});
    }
    
    return details;
  }
  
  uint8_t calculateRiskScore(const std::vector<uint16_t>& ports) {
    uint8_t risk = 0;
    
    if (std::find(ports.begin(), ports.end(), 23) != ports.end()) risk += 30;
    if (std::find(ports.begin(), ports.end(), 21) != ports.end()) risk += 20;
    if (std::find(ports.begin(), ports.end(), 3389) != ports.end()) risk += 15;
    if (std::find(ports.begin(), ports.end(), 445) != ports.end()) risk += 15;
    if (std::find(ports.begin(), ports.end(), 22) != ports.end()) risk += 10;
    if (std::find(ports.begin(), ports.end(), 3306) != ports.end()) risk += 10;
    if (std::find(ports.begin(), ports.end(), 5432) != ports.end()) risk += 10;
    if (std::find(ports.begin(), ports.end(), 80) != ports.end()) risk += 5;
    if (std::find(ports.begin(), ports.end(), 443) != ports.end()) risk += 3;
    
    risk += ports.size() * 2;
    
    return min(risk, (uint8_t)100);
  }
};

#endif

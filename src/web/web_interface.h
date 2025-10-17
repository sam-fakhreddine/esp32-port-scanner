#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <WebServer.h>
#include <ArduinoJson.h>
#include "../core/scan_orchestrator.h"
#include "../core/result_store.h"
#include "../core/config.h"
#include "web_content.h"

class WebInterface {
private:
  WebServer server;
  ScanOrchestrator& orchestrator;
  ResultStore& resultStore;
  WiFiScanner& wifiScanner;
  BluetoothScanner& btScanner;
  MDNSScanner& mdnsScanner;
  ARPTable& arpTable;
  SMBScanner& smbScanner;
  Config::ScanConfig& config;
  BLERebootScanner& bleRebootScanner;
  
public:
  WebInterface(ScanOrchestrator& orch, ResultStore& store, WiFiScanner& wifi, BluetoothScanner& bt, MDNSScanner& mdns, ARPTable& arp, SMBScanner& smb, Config::ScanConfig& cfg, BLERebootScanner& bleReboot)
    : server(80), orchestrator(orch), resultStore(store), wifiScanner(wifi), btScanner(bt), mdnsScanner(mdns), arpTable(arp), smbScanner(smb), config(cfg), bleRebootScanner(bleReboot) {
    
    server.on("/", HTTP_GET, [this]() {
      String html = buildHTML();
      server.send(200, "text/html", html);
    });
    
    server.on("/scan", HTTP_POST, [this]() {
      if (btScanner.isScanning()) {
        server.send(409, "application/json", "{\"error\":\"Bluetooth scan in progress\"}");
        return;
      }
      orchestrator.startScan();
      server.send(200, "application/json", "{\"status\":\"started\"}");
    });
    
    server.on("/pause", HTTP_POST, [this]() {
      orchestrator.pauseScan();
      server.send(200, "application/json", "{\"status\":\"paused\"}");
    });
    
    server.on("/resume", HTTP_POST, [this]() {
      orchestrator.resumeScan();
      server.send(200, "application/json", "{\"status\":\"resumed\"}");
    });
    
    server.on("/status", HTTP_GET, [this]() {
      DynamicJsonDocument doc(512);
      doc["scanning"] = orchestrator.getIsScanning();
      doc["paused"] = orchestrator.getIsPaused();
      doc["results"] = resultStore.count();
      doc["freeHeap"] = ESP.getFreeHeap();
      doc["uptime"] = millis() / 1000;
      
      if (orchestrator.getIsScanning()) {
        ScanProgress p = orchestrator.getProgress();
        doc["currentIP"] = p.currentIP;
        doc["currentPort"] = p.currentPort;
        doc["ipsScanned"] = p.ipsScanned;
        doc["totalIPs"] = p.totalIPs;
        doc["portsScanned"] = p.portsScanned;
        doc["totalPorts"] = p.totalPorts;
        doc["percentComplete"] = p.percentComplete;
      }
      
      String output;
      serializeJson(doc, output);
      server.send(200, "application/json", output);
    });
    
    server.on("/results", HTTP_GET, [this]() {
      String json = resultStore.toJson(config.networkPrefix);
      server.send(200, "application/json", json);
    });
    
    server.on("/config", HTTP_GET, [this]() {
      DynamicJsonDocument doc(512);
      doc["prefix"] = config.networkPrefix;
      doc["startIp"] = config.startIp;
      doc["endIp"] = config.endIp;
      doc["startPort"] = config.startPort;
      doc["endPort"] = config.endPort;
      doc["timeout"] = config.timeoutMs;
      doc["interval"] = config.intervalMs / 1000;
      doc["workers"] = config.workerThreads;
      doc["enableMQTT"] = config.enableMQTT;
      doc["enableBannerGrab"] = config.enableBannerGrab;
      doc["skipHostCheck"] = config.skipHostCheck;
      String output;
      serializeJson(doc, output);
      server.send(200, "application/json", output);
    });
    
    server.on("/config", HTTP_POST, [this]() {
      if (server.hasArg("prefix")) {
        strncpy(config.networkPrefix, server.arg("prefix").c_str(), sizeof(config.networkPrefix) - 1);
      }
      if (server.hasArg("startIp")) config.startIp = server.arg("startIp").toInt();
      if (server.hasArg("endIp")) config.endIp = server.arg("endIp").toInt();
      if (server.hasArg("startPort")) config.startPort = server.arg("startPort").toInt();
      if (server.hasArg("endPort")) config.endPort = server.arg("endPort").toInt();
      if (server.hasArg("timeout")) config.timeoutMs = server.arg("timeout").toInt();
      if (server.hasArg("interval")) config.intervalMs = server.arg("interval").toInt() * 1000;
      if (server.hasArg("workers")) {
        uint8_t w = server.arg("workers").toInt();
        if (w >= 1 && w <= 32) config.workerThreads = w;
      }
      if (server.hasArg("enableMQTT")) config.enableMQTT = server.arg("enableMQTT") == "true";
      if (server.hasArg("enableBannerGrab")) config.enableBannerGrab = server.arg("enableBannerGrab") == "true";
      if (server.hasArg("skipHostCheck")) config.skipHostCheck = server.arg("skipHostCheck") == "true";
      server.send(200, "application/json", "{\"status\":\"saved\"}");
    });
    
    server.on("/clear", HTTP_POST, [this]() {
      resultStore.clear();
      server.send(200, "application/json", "{\"status\":\"cleared\"}");
    });
    
    server.on("/stats", HTTP_GET, [this]() {
      String json = resultStore.getStatsJson();
      server.send(200, "application/json", json);
    });
    
    server.on("/endpoints", HTTP_GET, [this]() {
      String json = resultStore.getEndpointsJson(config.networkPrefix);
      server.send(200, "application/json", json);
    });
    
    server.on("/wifi/scan", HTTP_POST, [this]() {
      wifiScanner.scan();
      server.send(200, "application/json", "{\"status\":\"complete\"}");
    });
    
    server.on("/wifi/networks", HTTP_GET, [this]() {
      String json = wifiScanner.toJson();
      server.send(200, "application/json", json);
    });
    
    server.on("/bluetooth/scan", HTTP_POST, [this]() {
      bleRebootScanner.requestScan();
      server.send(200, "application/json", "{\"status\":\"rebooting\"}");
    });
    
    server.on("/bluetooth/devices", HTTP_GET, [this]() {
      String json = btScanner.toJson();
      server.send(200, "application/json", json);
    });
    
    server.on("/bluetooth/clear", HTTP_POST, [this]() {
      btScanner.clear();
      server.send(200, "application/json", "{\"status\":\"cleared\"}");
    });
    
    server.on("/mdns/scan", HTTP_POST, [this]() {
      mdnsScanner.scan();
      server.send(200, "application/json", "{\"status\":\"complete\"}");
    });
    
    server.on("/mdns/services", HTTP_GET, [this]() {
      String json = mdnsScanner.toJson();
      server.send(200, "application/json", json);
    });
    
    server.on("/arp/scan", HTTP_POST, [this]() {
      arpTable.scan();
      server.send(200, "application/json", "{\"status\":\"complete\"}");
    });
    
    server.on("/arp/table", HTTP_GET, [this]() {
      String json = arpTable.toJson();
      server.send(200, "application/json", json);
    });
    
    server.on("/smb/scan", HTTP_POST, [this]() {
      arpTable.scan();
      std::vector<uint8_t> arpIPs = arpTable.getActiveIPs();
      smbScanner.scan(arpIPs);
      server.send(200, "application/json", "{\"status\":\"complete\"}");
    });
    
    server.on("/smb/shares", HTTP_GET, [this]() {
      String json = smbScanner.toJson();
      server.send(200, "application/json", json);
    });
  }
  
  void begin() {
    server.begin();
    Serial.println("[WEB] Server started on port 80");
  }
  
  void handleClient() {
    server.handleClient();
  }
};

#endif

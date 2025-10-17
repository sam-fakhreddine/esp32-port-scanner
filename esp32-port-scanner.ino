#include <WiFi.h>
#include <WebServer.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <vector>

#include "src/core/config.h"
#include "src/core/interfaces.h"
#include "src/core/data_structures.h"
#include "src/network/wifi_manager.h"
#include "src/network/mqtt_manager.h"
#include "src/scanners/port_scanner.h"
#include "src/scanners/wifi_scanner.h"
#include "src/scanners/bluetooth_scanner.h"
#include "src/scanners/ble_reboot_scan.h"
#include "src/scanners/mdns_scanner.h"
#include "src/scanners/arp_table.h"
#include "src/scanners/smb_scanner.h"
#include "src/core/result_store.h"
#include "src/core/scan_orchestrator.h"
#include "src/web/web_interface.h"

Config::WiFiConfig Config::wifi;
Config::MQTTConfig Config::mqtt;
Config::ScanConfig Config::scan;

WiFiManager wifiManager(Config::wifi);
MQTTManager mqttManager(Config::mqtt);
PortScanner portScanner;
WiFiScanner wifiScanner;
BluetoothScanner btScanner;
BLERebootScanner bleRebootScanner(btScanner);
MDNSScanner mdnsScanner;
ARPTable arpTable;
SMBScanner smbScanner;
ResultStore resultStore;
ScanOrchestrator orchestrator(portScanner, mqttManager, resultStore, Config::scan);
WebInterface webInterface(orchestrator, resultStore, wifiScanner, btScanner, mdnsScanner, arpTable, smbScanner, Config::scan, bleRebootScanner);

uint32_t lastAutoScan = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n");
  Serial.println("  ██████╗  ██████╗  ██████╗ ");
  Serial.println("  ██╔══██╗██╔═══██╗██╔═══██╗");
  Serial.println("  ██║  ██╗██╔   ██╗██╔   ██╗");
  Serial.println("  ██████╔╝██║   ██║██║   ██║");
  Serial.println("  ██║  ██╗██║   ██║██║   ██║");
  Serial.println("  ██╚══██╗██║   ██║██║   ██║");
  Serial.println("  ██████╔╝╚██████╔╝╚██████╔╝");
  Serial.println("  ╚═════╝  ╚═════╝  ╚═════╝ ");
  Serial.println("\n");
  Serial.println("  ╔═══════════════════════════════════════╗");
  Serial.println("  ║   IoT Security Intelligence v4.0      ║");
  Serial.println("  ║   Port Scanner + Device Fingerprint   ║");
  Serial.println("  ║            👻 Spooky Mode 👻            ║");
  Serial.println("  ╚═══════════════════════════════════════╝\n");
  Serial.printf("[MEM] Free heap at start: %d bytes\n", ESP.getFreeHeap());
  
  // Check if BLE scan was requested
  if (bleRebootScanner.checkAndScan()) {
    Serial.println("[BLE] Scan complete, continuing startup...");
  }
  
  if (!wifiManager.connect()) {
    Serial.println("[ERROR] WiFi connection failed");
    ESP.restart();
  }
  
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("[NTP] Syncing time...");
  delay(2000);
  
  mqttManager.connect();
  webInterface.begin();
  
  time_t now = time(nullptr);
  if (now > 1000000000) {
    Serial.printf("[NTP] Time synced: %s", ctime(&now));
  } else {
    Serial.println("[NTP] Time sync failed, using uptime");
  }
  
  Serial.printf("[READY] Web interface: http://%s\n", WiFi.localIP().toString().c_str());
  Serial.printf("[MEM] Free heap: %d bytes\n", ESP.getFreeHeap());
  Serial.println("[INFO] BLE will initialize on first scan");
}

void loop() {
  // Reconnect WiFi if disconnected (e.g., after BLE scan)
  if (!wifiManager.isConnected() && !btScanner.isScanning()) {
    wifiManager.reconnect();
  }
  
  webInterface.handleClient();
  
  if (mqttManager.isConnected()) {
    mqttManager.loop();
  } else if (wifiManager.isConnected()) {
    mqttManager.connect();
  }
  
  if (millis() - lastAutoScan > Config::scan.intervalMs) {
    if (!orchestrator.getIsScanning() && !btScanner.isScanning()) {
      orchestrator.startScan();
    }
    lastAutoScan = millis();
  }
  
  if (ESP.getFreeHeap() < 1000) {
    Serial.println("[WARNING] Low memory, restarting...");
    ESP.restart();
  }
  
  delay(10);
}

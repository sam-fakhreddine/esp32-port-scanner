#ifndef BLE_REBOOT_SCAN_H
#define BLE_REBOOT_SCAN_H

#include <Preferences.h>
#include "bluetooth_scanner.h"

class BLERebootScanner {
private:
  Preferences prefs;
  BluetoothScanner& btScanner;
  
public:
  BLERebootScanner(BluetoothScanner& bt) : btScanner(bt) {}
  
  void requestScan() {
    prefs.begin("ble", false);
    prefs.putBool("scan", true);
    prefs.end();
    Serial.println("[BLE] Scan scheduled, rebooting...");
    delay(100);
    ESP.restart();
  }
  
  bool checkAndScan() {
    prefs.begin("ble", false);
    bool shouldScan = prefs.getBool("scan", false);
    prefs.putBool("scan", false);
    prefs.end();
    
    if (shouldScan) {
      Serial.println("[BLE] Executing scheduled scan...");
      btScanner.scan(30);
      return true;
    }
    return false;
  }
};

#endif

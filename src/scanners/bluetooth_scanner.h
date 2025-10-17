#ifndef BLUETOOTH_SCANNER_H
#define BLUETOOTH_SCANNER_H

#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <WiFi.h>
#include <vector>
#include <ArduinoJson.h>
#include "../core/data_structures.h"
#include "../core/time_utils.h"

class BluetoothScanner {
private:
  std::vector<BluetoothDevice> devices;
  SemaphoreHandle_t mutex;
  volatile bool scanning;
  volatile int activeCallbacks;
  BLEScan* pBLEScan;
  bool bleInitialized;
  BLEAdvertisedDeviceCallbacks* pCallbacks;
  
  class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    BluetoothScanner* scanner;
  public:
    MyAdvertisedDeviceCallbacks(BluetoothScanner* s) : scanner(s) {}
    
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      if (!scanner || !scanner->mutex || !scanner->scanning) return;
      scanner->activeCallbacks++;
      if (xSemaphoreTake(scanner->mutex, 10)) {
        try {
          bool found = false;
          uint8_t* addr = (uint8_t*)advertisedDevice.getAddress().getNative();
          if (!addr) {
            xSemaphoreGive(scanner->mutex);
            scanner->activeCallbacks--;
            return;
          }
          
          for (auto& dev : scanner->devices) {
            if (memcmp(dev.address, addr, 6) == 0) {
              dev.rssi = advertisedDevice.getRSSI();
              dev.lastSeen = getTimestamp();
              found = true;
              break;
            }
          }
          
          if (!found && scanner->devices.size() < 50) {
            BluetoothDevice dev;
            if (advertisedDevice.haveName()) {
              strncpy(dev.name, advertisedDevice.getName().c_str(), sizeof(dev.name) - 1);
            } else {
              strcpy(dev.name, "<Unknown>");
            }
            memcpy(dev.address, addr, 6);
            dev.rssi = advertisedDevice.getRSSI();
            dev.firstSeen = getTimestamp();
            dev.lastSeen = getTimestamp();
            dev.addressType = advertisedDevice.getAddressType();
            
            if (advertisedDevice.haveAppearance()) {
              dev.appearance = advertisedDevice.getAppearance();
            }
            if (advertisedDevice.haveTXPower()) {
              dev.txPower = advertisedDevice.getTXPower();
            }
            if (advertisedDevice.haveManufacturerData()) {
              std::string mfg = advertisedDevice.getManufacturerData().c_str();
              if (mfg.length() >= 2) {
                uint16_t companyId = (uint8_t)mfg[1] << 8 | (uint8_t)mfg[0];
                snprintf(dev.manufacturer, sizeof(dev.manufacturer), "0x%04X", companyId);
              }
            }
            if (advertisedDevice.haveServiceUUID()) {
              BLEUUID uuid = advertisedDevice.getServiceUUID();
              if (dev.serviceCount < 8) {
                dev.serviceUUIDs[dev.serviceCount++] = uuid.getNative()->uuid.uuid16;
              }
            }
            
            scanner->devices.push_back(dev);
          }
        } catch (...) {
        }
        xSemaphoreGive(scanner->mutex);
      }
      scanner->activeCallbacks--;
    }
  };
  
public:
  BluetoothScanner() {
    mutex = xSemaphoreCreateMutex();
    scanning = false;
    activeCallbacks = 0;
    pBLEScan = nullptr;
    bleInitialized = false;
    pCallbacks = nullptr;
  }
  
  void init() {
    if (bleInitialized) return;
    
    Serial.println("[BLE] Initializing...");
    try {
      BLEDevice::init("");
      delay(100);
      pBLEScan = BLEDevice::getScan();
      if (pBLEScan) {
        if (!pCallbacks) {
          pCallbacks = new MyAdvertisedDeviceCallbacks(this);
        }
        pBLEScan->setAdvertisedDeviceCallbacks(pCallbacks);
        pBLEScan->setActiveScan(false);
        pBLEScan->setInterval(1349);
        pBLEScan->setWindow(449);
        bleInitialized = true;
        Serial.println("[BLE] Initialized");
      }
    } catch (...) {
      Serial.println("[BLE] Init failed");
      pBLEScan = nullptr;
      bleInitialized = false;
    }
  }
  
  void deinit() {
    if (!bleInitialized) return;
    Serial.println("[BLE] Deinit start");
    try {
      if (pBLEScan) {
        Serial.println("[BLE] Stopping scan");
        pBLEScan->stop();
        delay(200);
      }
      Serial.println("[BLE] Calling BLEDevice::deinit");
      BLEDevice::deinit(false);
      bleInitialized = false;
      pBLEScan = nullptr;
      Serial.println("[BLE] Deinitialized");
    } catch (...) {
      Serial.println("[BLE] Deinit failed");
      bleInitialized = false;
      pBLEScan = nullptr;
    }
  }
  
  ~BluetoothScanner() {
    deinit();
    pCallbacks = nullptr;
    if (mutex) {
      vSemaphoreDelete(mutex);
    }
  }
  
  void scan(uint8_t duration = 5) {
    if (ESP.getFreeHeap() < 50000) {
      Serial.println("[BLE] Low memory");
      return;
    }
    
    scanning = true;
    bool wifiWasConnected = WiFi.status() == WL_CONNECTED;
    
    Serial.println("[BLE] Disabling WiFi...");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    delay(500);
    
    uint32_t scanStart = millis();
    uint32_t timeout = (duration + 5) * 1000;
    
    try {
      init();
      if (!pBLEScan) {
        Serial.println("[BLE] Init failed");
      } else {
        Serial.printf("[BLE] Scanning %ds...\n", duration);
        
        BLEScanResults* results = pBLEScan->start(duration, false);
        
        if (millis() - scanStart < timeout && results) {
          Serial.printf("[BLE] Found %d devices\n", results->getCount());
        }
        
        scanning = false;
        
        if (pBLEScan) {
          pBLEScan->stop();
        }
        
        uint32_t waitStart = millis();
        while (activeCallbacks > 0 && millis() - waitStart < 2000) {
          delay(10);
        }
        Serial.printf("[BLE] Callbacks done: %d remaining\n", activeCallbacks);
        
        if (pBLEScan) {
          pBLEScan->clearResults();
        }
      }
    } catch (...) {
      Serial.println("[BLE] Scan error");
      scanning = false;
    }
    
    delay(300);
    deinit();
    delay(200);
    
    if (wifiWasConnected) {
      Serial.println("[BLE] Re-enabling WiFi...");
      WiFi.mode(WIFI_STA);
      delay(100);
    }
    
    Serial.printf("[BLE] Complete: %d devices\n", devices.size());
  }
  
  String toJson() const {
    DynamicJsonDocument doc(4096);
    JsonArray arr = doc.to<JsonArray>();
    
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      for (const auto& dev : devices) {
        JsonObject obj = arr.createNestedObject();
        obj["name"] = dev.name;
        
        char addrStr[18];
        snprintf(addrStr, sizeof(addrStr), "%02X:%02X:%02X:%02X:%02X:%02X",
          dev.address[0], dev.address[1], dev.address[2],
          dev.address[3], dev.address[4], dev.address[5]);
        obj["address"] = addrStr;
        
        obj["rssi"] = dev.rssi;
        obj["firstSeen"] = dev.firstSeen;
        obj["lastSeen"] = dev.lastSeen;
        obj["isRandom"] = (dev.addressType == BLE_ADDR_TYPE_RANDOM);
        
        int distance = 0;
        if (dev.rssi >= -50) distance = 1;
        else if (dev.rssi >= -70) distance = 5;
        else if (dev.rssi >= -85) distance = 10;
        else distance = 20;
        obj["distance"] = distance;
        
        if (dev.appearance > 0) obj["appearance"] = dev.appearance;
        if (dev.txPower != 0) obj["txPower"] = dev.txPower;
        if (strlen(dev.manufacturer) > 0) obj["manufacturer"] = dev.manufacturer;
        if (dev.serviceCount > 0) {
          JsonArray services = obj.createNestedArray("services");
          for (uint8_t i = 0; i < dev.serviceCount; i++) {
            char uuid[8];
            snprintf(uuid, sizeof(uuid), "0x%04X", dev.serviceUUIDs[i]);
            services.add(uuid);
          }
        }
      }
      xSemaphoreGive(mutex);
    }
    
    String output;
    serializeJson(doc, output);
    return output;
  }
  
  size_t count() const {
    return devices.size();
  }
  
  void clear() {
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      devices.clear();
      xSemaphoreGive(mutex);
    }
  }
  
  bool isScanning() const {
    return scanning;
  }
};

#endif

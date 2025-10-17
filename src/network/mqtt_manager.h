#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <WiFi.h>
#include <PubSubClient.h>
#include "../core/interfaces.h"
#include "../core/config.h"

class MQTTManager : public IConnectionManager, public IPublisher {
private:
  WiFiClient espClient;
  PubSubClient client;
  const Config::MQTTConfig& config;
  SemaphoreHandle_t mutex;
  uint8_t maxRetries;
  
  void generateClientId() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    snprintf(const_cast<char*>(config.clientId), 24, "ESP32_%02X%02X%02X", 
      mac[3], mac[4], mac[5]);
  }
  
public:
  MQTTManager(const Config::MQTTConfig& cfg, uint8_t retries = 3) 
    : client(espClient), config(cfg), maxRetries(retries) {
    client.setServer(config.server, config.port);
    client.setBufferSize(config.bufferSize);
    mutex = xSemaphoreCreateMutex();
    generateClientId();
  }
  
  ~MQTTManager() {
    if (mutex) vSemaphoreDelete(mutex);
  }
  
  bool connect() override {
    if (client.connected()) return true;
    
    Serial.printf("[MQTT] Connecting to %s:%d as %s\n", 
      config.server, config.port, config.clientId);
    
    uint8_t attempts = 0;
    while (!client.connected() && attempts < maxRetries) {
      if (client.connect(config.clientId)) {
        Serial.println("[MQTT] Connected!");
        return true;
      }
      Serial.printf("[MQTT] Failed (rc=%d), retry %d/%d\n", 
        client.state(), attempts + 1, maxRetries);
      delay(2000);
      attempts++;
    }
    
    return false;
  }
  
  bool isConnected() override {
    return client.connected();
  }
  
  void disconnect() override {
    client.disconnect();
  }
  
  bool publish(const char* topic, const char* payload) override {
    if (!client.connected()) return false;
    
    bool result = false;
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      result = client.publish(topic, payload);
      xSemaphoreGive(mutex);
    }
    return result;
  }
  
  void loop() {
    client.loop();
  }
};

#endif

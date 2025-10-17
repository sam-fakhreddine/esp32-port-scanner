#include "scan_orchestrator.h"
#include "../scanners/arp_scanner.h"
#include "../scanners/arp_table.h"
#include "../scanners/dns_resolver.h"

extern ARPTable arpTable;

ScanOrchestrator::ScanOrchestrator(IPortScanner& s, IPublisher& p, ResultStore& r, const Config::ScanConfig& cfg)
  : scanner(s), publisher(p), resultStore(r), config(cfg), isScanning(false), isPaused(false) {
  
  progressMutex = xSemaphoreCreateMutex();
  taskQueue = xQueueCreate(config.queueSize, sizeof(ScanTask));
  workerHandles = new TaskHandle_t[config.workerThreads];
  
  for (uint8_t i = 0; i < config.workerThreads; i++) {
    char taskName[16];
    snprintf(taskName, sizeof(taskName), "Worker%d", i);
    xTaskCreatePinnedToCore(workerTask, taskName, 4096, this, 1, &workerHandles[i], i % 2);
  }
}

ScanOrchestrator::~ScanOrchestrator() {
  if (taskQueue) vQueueDelete(taskQueue);
  if (progressMutex) vSemaphoreDelete(progressMutex);
  delete[] workerHandles;
}

ScanProgress ScanOrchestrator::getProgress() const {
  ScanProgress p;
  if (xSemaphoreTake(progressMutex, portMAX_DELAY)) {
    p = progress;
    xSemaphoreGive(progressMutex);
  }
  return p;
}

void ScanOrchestrator::workerTask(void* parameter) {
  auto* self = static_cast<ScanOrchestrator*>(parameter);
  ScanTask task;
  
  while (true) {
    if (xQueueReceive(self->taskQueue, &task, portMAX_DELAY)) {
      char ipStr[16];
      snprintf(ipStr, sizeof(ipStr), "%s%d", self->config.networkPrefix, task.ip);
      
      uint32_t startTime = millis();
      std::vector<uint16_t> foundPorts;
      
      for (uint16_t port = task.portStart; port <= task.portEnd; port++) {
        while (self->isPaused) {
          delay(100);
        }
        
        if (xSemaphoreTake(self->progressMutex, 10)) {
          strncpy(self->progress.currentIP, ipStr, sizeof(self->progress.currentIP) - 1);
          self->progress.currentPort = port;
          self->progress.portsScanned++;
          xSemaphoreGive(self->progressMutex);
        }
        
        String banner;
        bool isOpen = false;
        
        if (self->config.enableBannerGrab && (port == 80 || port == 443 || port == 22 || port == 21 || port == 23)) {
          isOpen = self->scanner.scanPortWithBanner(ipStr, port, self->config.timeoutMs, banner);
        } else {
          isOpen = self->scanner.scanPort(ipStr, port, self->config.timeoutMs);
        }
        
        if (isOpen) {
          foundPorts.push_back(port);
          self->resultStore.add(task.ip, port, banner);
          
          if (self->config.enableMQTT) {
            char topic[32], payload[128];
            snprintf(topic, sizeof(topic), "portscan/%s%d", self->config.networkPrefix, task.ip);
            if (banner.length() > 0) {
              snprintf(payload, sizeof(payload), "{\"port\":%d,\"state\":\"open\",\"banner\":\"%s\"}", 
                        port, banner.substring(0, 50).c_str());
            } else {
              snprintf(payload, sizeof(payload), "{\"port\":%d,\"state\":\"open\"}", port);
            }
            self->publisher.publish(topic, payload);
          }
          
          Serial.printf("[SCAN] %s:%d OPEN", ipStr, port);
          if (banner.length() > 0) {
            Serial.printf(" [%s]", banner.substring(0, 40).c_str());
          }
          Serial.println();
        }
      }
      
      if (foundPorts.size() > 0) {
        uint16_t responseTime = (millis() - startTime) / foundPorts.size();
        PortScanner* ps = static_cast<PortScanner*>(&self->scanner);
        const char* deviceType = ps->identifyDevice(foundPorts);
        uint8_t riskScore = ps->calculateRiskScore(foundPorts);
        auto riskDetails = ps->getRiskDetails(foundPorts);
        self->resultStore.updateEndpointMetadata(task.ip, deviceType, riskScore, responseTime, riskDetails);
        
        Serial.printf("[INFO] %s: %s (Risk: %d%%, Ports: %d)\n", 
                     ipStr, deviceType, riskScore, foundPorts.size());
      }
    }
  }
}

bool ScanOrchestrator::isHostAlive(uint8_t ip) {
  if (config.skipHostCheck) return true;
  
  char ipStr[16];
  snprintf(ipStr, sizeof(ipStr), "%s%d", config.networkPrefix, ip);
  
  // Try ARP first (fast, Layer 2)
  if (ARPScanner::isHostAlive(ipStr)) return true;
  
  // Fallback to port check if ARP fails
  const uint16_t testPorts[] = {80, 443, 22};
  for (uint16_t port : testPorts) {
    if (scanner.scanPort(ipStr, port, 30)) return true;
  }
  return false;
}

void ScanOrchestrator::startScan() {
  if (isScanning) return;
  
  xTaskCreate([](void* param) {
    auto* self = static_cast<ScanOrchestrator*>(param);
    self->executeScan();
    vTaskDelete(NULL);
  }, "ScanTask", 8192, this, 1, NULL);
}

void ScanOrchestrator::executeScan() {
  isScanning = true;
  
  if (config.enableMQTT) {
    publisher.publish("portscan/status", "{\"status\":\"started\"}");
  }
  Serial.printf("[SCAN] Starting: %s%d-%d, ports %d-%d\n", 
    config.networkPrefix, config.startIp, config.endIp, config.startPort, config.endPort);
  
  uint32_t startTime = millis();
  uint32_t ipsScanned = 0;
  uint32_t portsChecked = 0;
  
  // Get live hosts from ARP table first
  Serial.println("[SCAN] Performing ARP sweep...");
  arpTable.scan();
  std::vector<uint8_t> arpIPs = arpTable.getActiveIPs();
  Serial.printf("[SCAN] ARP found %d live hosts\n", arpIPs.size());
  
  uint32_t totalIPs = arpIPs.size();
  uint32_t totalPorts = (config.endPort - config.startPort + 1) * totalIPs;
  
  if (xSemaphoreTake(progressMutex, portMAX_DELAY)) {
    progress.totalIPs = totalIPs;
    progress.totalPorts = totalPorts;
    progress.ipsScanned = 0;
    progress.portsScanned = 0;
    progress.percentComplete = 0;
    xSemaphoreGive(progressMutex);
  }
  
  uint16_t portsPerTask = (config.endPort - config.startPort + 1) / config.workerThreads;
  if (portsPerTask == 0) portsPerTask = 1;
  
  std::vector<uint8_t> knownIPs;
  std::vector<uint8_t> unknownIPs;
  
  // Prioritize ARP-discovered hosts
  for (uint8_t ip : arpIPs) {
    if (ip >= config.startIp && ip <= config.endIp) {
      if (resultStore.hasEndpoint(ip)) {
        knownIPs.push_back(ip);
      } else {
        unknownIPs.push_back(ip);
      }
    }
  }
  
  // Add remaining IPs from range (if not already in ARP)
  for (uint8_t ip = config.startIp; ip <= config.endIp; ip++) {
    bool inARP = std::find(arpIPs.begin(), arpIPs.end(), ip) != arpIPs.end();
    if (!inARP) {
      if (resultStore.hasEndpoint(ip)) {
        knownIPs.push_back(ip);
      } else {
        unknownIPs.push_back(ip);
      }
    }
  }
  
  Serial.printf("[SCAN] Strategy: %d new IPs first, %d known IPs later\n", unknownIPs.size(), knownIPs.size());
  
  // Scan unknown IPs first
  for (uint8_t ip : unknownIPs) {
    char ipStr[16];
    snprintf(ipStr, sizeof(ipStr), "%s%d", config.networkPrefix, ip);
    
    if (xSemaphoreTake(progressMutex, 10)) {
      strncpy(progress.currentIP, ipStr, sizeof(progress.currentIP) - 1);
      progress.currentPort = 0;
      xSemaphoreGive(progressMutex);
    }
    
    if (isHostAlive(ip)) {
      IPAddress ipAddr;
      ipAddr.fromString(ipStr);
      String hostname = DNSResolver::getHostnameByIP(ipAddr);
      if (hostname.length() > 0) {
        resultStore.updateHostname(ip, hostname.c_str());
        Serial.printf("[DNS] %s -> %s\n", ipStr, hostname.c_str());
      }
      
      ipsScanned++;
      if (xSemaphoreTake(progressMutex, 10)) {
        progress.ipsScanned = ipsScanned;
        progress.percentComplete = (ipsScanned * 100) / totalIPs;
        xSemaphoreGive(progressMutex);
      }
      for (uint16_t portStart = config.startPort; portStart <= config.endPort; portStart += portsPerTask) {
        uint16_t portEnd = min((uint16_t)(portStart + portsPerTask - 1), config.endPort);
        ScanTask task(ip, portStart, portEnd);
        xQueueSend(taskQueue, &task, portMAX_DELAY);
        portsChecked += (portEnd - portStart + 1);
      }
    }
  }
  
  // Now scan known IPs (rescan)
  for (uint8_t ip : knownIPs) {
    char ipStr[16];
    snprintf(ipStr, sizeof(ipStr), "%s%d", config.networkPrefix, ip);
    
    if (xSemaphoreTake(progressMutex, 10)) {
      strncpy(progress.currentIP, ipStr, sizeof(progress.currentIP) - 1);
      progress.currentPort = 0;
      xSemaphoreGive(progressMutex);
    }
    
    if (isHostAlive(ip)) {
      IPAddress ipAddr;
      ipAddr.fromString(ipStr);
      String hostname = DNSResolver::getHostnameByIP(ipAddr);
      if (hostname.length() > 0) {
        resultStore.updateHostname(ip, hostname.c_str());
        Serial.printf("[DNS] %s -> %s\n", ipStr, hostname.c_str());
      }
      
      ipsScanned++;
      if (xSemaphoreTake(progressMutex, 10)) {
        progress.ipsScanned = ipsScanned;
        progress.percentComplete = (ipsScanned * 100) / totalIPs;
        xSemaphoreGive(progressMutex);
      }
      for (uint16_t portStart = config.startPort; portStart <= config.endPort; portStart += portsPerTask) {
        uint16_t portEnd = min((uint16_t)(portStart + portsPerTask - 1), config.endPort);
        ScanTask task(ip, portStart, portEnd);
        xQueueSend(taskQueue, &task, portMAX_DELAY);
        portsChecked += (portEnd - portStart + 1);
      }
    }
  }
  
  // Wait for workers to finish with timeout
  uint32_t waitStart = millis();
  while (uxQueueMessagesWaiting(taskQueue) > 0) {
    delay(100);
    if (millis() - waitStart > 300000) {  // 5 minute timeout
      Serial.println("[SCAN] Timeout waiting for workers, completing anyway");
      break;
    }
  }
  
  // Give workers time to finish current tasks
  delay(1000);
  
  uint32_t duration = (millis() - startTime) / 1000;
  resultStore.updateScanStats(ipsScanned, portsChecked, duration);
  
  char statusMsg[128];
  snprintf(statusMsg, sizeof(statusMsg), 
    "{\"status\":\"complete\",\"duration\":%lu,\"results\":%zu}", 
    duration, resultStore.count());
  
  if (config.enableMQTT) {
    publisher.publish("portscan/status", statusMsg);
  }
  Serial.printf("[SCAN] Complete: %lu seconds, %zu open ports, %lu IPs, %lu ports checked\n", 
    duration, resultStore.count(), ipsScanned, portsChecked);
  Serial.printf("[SCAN] Queue remaining: %d tasks\n", uxQueueMessagesWaiting(taskQueue));
  
  isScanning = false;
  isPaused = false;
}

void ScanOrchestrator::pauseScan() {
  if (isScanning && !isPaused) {
    isPaused = true;
    Serial.println("[SCAN] Paused");
  }
}

void ScanOrchestrator::resumeScan() {
  if (isScanning && isPaused) {
    isPaused = false;
    Serial.println("[SCAN] Resumed");
  }
}

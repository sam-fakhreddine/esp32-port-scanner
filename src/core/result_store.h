#ifndef RESULT_STORE_H
#define RESULT_STORE_H

#include <vector>
#include <algorithm>
#include <ArduinoJson.h>
#include "data_structures.h"
#include "time_utils.h"

class ResultStore {
private:
  std::vector<ScanResult> results;
  std::vector<EndpointData> endpoints;
  ScanStats stats;
  SemaphoreHandle_t mutex;
  const size_t maxResults = 100;
  
public:
  ResultStore() {
    mutex = xSemaphoreCreateMutex();
    results.reserve(maxResults);
  }
  
  ~ResultStore() {
    if (mutex) vSemaphoreDelete(mutex);
  }
  
  void add(uint8_t ip, uint16_t port, const String& banner = "") {
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      if (results.size() >= maxResults) {
        results.erase(results.begin());
      }
      results.emplace_back(ip, port);
      stats.totalOpenPorts++;
      
      bool found = false;
      for (auto& ep : endpoints) {
        if (ep.ip == ip) {
          ep.lastSeen = getTimestamp();
          if (std::find(ep.openPorts.begin(), ep.openPorts.end(), port) == ep.openPorts.end()) {
            ep.openPorts.push_back(port);
            if (banner.length() > 0) {
              ep.serviceBanners.push_back(String(port) + ": " + banner);
            }
          }
          found = true;
          break;
        }
      }
      if (!found) {
        EndpointData ep(ip);
        ep.openPorts.push_back(port);
        if (banner.length() > 0) {
          ep.serviceBanners.push_back(String(port) + ": " + banner);
        }
        endpoints.push_back(ep);
        stats.uniqueHosts++;
      }
      
      xSemaphoreGive(mutex);
    }
  }
  
  void updateEndpointMetadata(uint8_t ip, const char* deviceType, uint8_t riskScore, uint16_t responseTime, const std::vector<RiskDetail>& riskDetails) {
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      for (auto& ep : endpoints) {
        if (ep.ip == ip) {
          strncpy(ep.deviceType, deviceType, sizeof(ep.deviceType) - 1);
          ep.riskScore = riskScore;
          ep.avgResponseTime = responseTime;
          ep.riskFindings.clear();
          for (const auto& rd : riskDetails) {
            RiskFinding rf;
            rf.port = String(rd.port);
            rf.service = String(rd.service);
            rf.severity = String(rd.severity);
            rf.points = rd.points;
            ep.riskFindings.push_back(rf);
          }
          break;
        }
      }
      xSemaphoreGive(mutex);
    }
  }
  
  void updateHostname(uint8_t ip, const char* hostname) {
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      bool found = false;
      for (auto& ep : endpoints) {
        if (ep.ip == ip) {
          strncpy(ep.hostname, hostname, sizeof(ep.hostname) - 1);
          found = true;
          break;
        }
      }
      if (!found) {
        EndpointData ep(ip);
        strncpy(ep.hostname, hostname, sizeof(ep.hostname) - 1);
        endpoints.push_back(ep);
        stats.uniqueHosts++;
      }
      xSemaphoreGive(mutex);
    }
  }
  
  void clear() {
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      results.clear();
      xSemaphoreGive(mutex);
    }
  }
  
  void updateScanStats(uint32_t ipsScanned, uint32_t portsChecked, uint32_t duration) {
    if (xSemaphoreTake(mutex, portMAX_DELAY)) {
      stats.totalScans++;
      stats.totalIpsScanned += ipsScanned;
      stats.totalPortsChecked += portsChecked;
      stats.lastScanDuration = duration;
      stats.avgScanDuration = (stats.avgScanDuration * (stats.totalScans - 1) + duration) / stats.totalScans;
      xSemaphoreGive(mutex);
    }
  }
  
  size_t count() const { return results.size(); }
  
  bool hasEndpoint(uint8_t ip) const {
    if (xSemaphoreTake(mutex, 100)) {
      for (const auto& ep : endpoints) {
        if (ep.ip == ip) {
          xSemaphoreGive(mutex);
          return true;
        }
      }
      xSemaphoreGive(mutex);
    }
    return false;
  }
  
  String toJson(const char* networkPrefix) const;
  String getStatsJson() const;
  String getEndpointsJson(const char* networkPrefix) const;
};

#endif

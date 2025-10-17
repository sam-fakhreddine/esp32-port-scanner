#include "result_store.h"

String ResultStore::toJson(const char* networkPrefix) const {
  DynamicJsonDocument doc(2048);
  JsonArray arr = doc.to<JsonArray>();
  
  if (xSemaphoreTake(mutex, portMAX_DELAY)) {
    for (const auto& r : results) {
      JsonObject obj = arr.createNestedObject();
      char ipStr[16];
      snprintf(ipStr, sizeof(ipStr), "%s%d", networkPrefix, r.ip);
      obj["ip"] = ipStr;
      obj["port"] = r.port;
      obj["timestamp"] = r.timestamp;
    }
    xSemaphoreGive(mutex);
  }
  
  String output;
  serializeJson(doc, output);
  return output;
}

String ResultStore::getStatsJson() const {
  DynamicJsonDocument doc(512);
  if (xSemaphoreTake(mutex, portMAX_DELAY)) {
    doc["totalScans"] = stats.totalScans;
    doc["totalIpsScanned"] = stats.totalIpsScanned;
    doc["totalPortsChecked"] = stats.totalPortsChecked;
    doc["totalOpenPorts"] = stats.totalOpenPorts;
    doc["uniqueHosts"] = stats.uniqueHosts;
    doc["lastScanDuration"] = stats.lastScanDuration;
    doc["avgScanDuration"] = stats.avgScanDuration;
    xSemaphoreGive(mutex);
  }
  String output;
  serializeJson(doc, output);
  return output;
}

String ResultStore::getEndpointsJson(const char* networkPrefix) const {
  DynamicJsonDocument doc(8192);
  JsonArray arr = doc.to<JsonArray>();
  
  if (xSemaphoreTake(mutex, portMAX_DELAY)) {
    for (const auto& ep : endpoints) {
      JsonObject obj = arr.createNestedObject();
      char ipStr[16];
      snprintf(ipStr, sizeof(ipStr), "%s%d", networkPrefix, ep.ip);
      obj["ip"] = ipStr;
      obj["mac"] = ep.macAddress;
      obj["hostname"] = ep.hostname;
      obj["deviceType"] = ep.deviceType;
      obj["riskScore"] = ep.riskScore;
      obj["avgResponseTime"] = ep.avgResponseTime;
      obj["portCount"] = ep.openPorts.size();
      JsonArray ports = obj.createNestedArray("ports");
      for (uint16_t p : ep.openPorts) {
        ports.add(p);
      }
      JsonArray banners = obj.createNestedArray("banners");
      for (const auto& b : ep.serviceBanners) {
        String sanitized;
        sanitized.reserve(b.length());
        for (size_t i = 0; i < b.length(); i++) {
          char c = b[i];
          if (c >= 32 && c <= 126 && c != '"' && c != '\\') {
            sanitized += c;
          } else if (c == '"') {
            sanitized += "'";
          } else {
            sanitized += ' ';
          }
        }
        banners.add(sanitized);
      }
      JsonArray risks = obj.createNestedArray("riskFindings");
      for (const auto& rf : ep.riskFindings) {
        JsonObject riskObj = risks.createNestedObject();
        riskObj["port"] = rf.port;
        riskObj["service"] = rf.service;
        riskObj["severity"] = rf.severity;
        riskObj["points"] = rf.points;
      }
      obj["firstSeen"] = ep.firstSeen;
      obj["lastSeen"] = ep.lastSeen;
      obj["scanCount"] = ep.scanCount;
    }
    xSemaphoreGive(mutex);
  }
  
  String output;
  serializeJson(doc, output);
  return output;
}

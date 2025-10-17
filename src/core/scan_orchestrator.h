#ifndef SCAN_ORCHESTRATOR_H
#define SCAN_ORCHESTRATOR_H

#include "interfaces.h"
#include "../scanners/port_scanner.h"
#include "result_store.h"
#include "config.h"

class ScanOrchestrator {
private:
  IPortScanner& scanner;
  IPublisher& publisher;
  ResultStore& resultStore;
  QueueHandle_t taskQueue;
  TaskHandle_t* workerHandles;
  const Config::ScanConfig& config;
  volatile bool isScanning;
  volatile bool isPaused;
  ScanProgress progress;
  SemaphoreHandle_t progressMutex;
  
  static void workerTask(void* parameter);
  bool isHostAlive(uint8_t ip);
  
public:
  ScanOrchestrator(IPortScanner& s, IPublisher& p, ResultStore& r, const Config::ScanConfig& cfg);
  ~ScanOrchestrator();
  
  void startScan();
  void executeScan();
  void pauseScan();
  void resumeScan();
  bool getIsScanning() const { return isScanning; }
  bool getIsPaused() const { return isPaused; }
  ScanProgress getProgress() const;
};

#endif

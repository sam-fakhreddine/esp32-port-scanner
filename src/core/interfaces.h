#ifndef INTERFACES_H
#define INTERFACES_H

#include <Arduino.h>

class IConnectionManager {
public:
  virtual bool connect() = 0;
  virtual bool isConnected() = 0;
  virtual void disconnect() = 0;
  virtual ~IConnectionManager() = default;
};

class IPublisher {
public:
  virtual bool publish(const char* topic, const char* payload) = 0;
  virtual ~IPublisher() = default;
};

class IPortScanner {
public:
  virtual bool scanPort(const char* ip, uint16_t port, uint16_t timeout) = 0;
  virtual bool scanPortWithBanner(const char* ip, uint16_t port, uint16_t timeout, String& banner) = 0;
  virtual ~IPortScanner() = default;
};

#endif

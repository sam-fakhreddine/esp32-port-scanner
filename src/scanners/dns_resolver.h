#ifndef DNS_RESOLVER_H
#define DNS_RESOLVER_H

#include <WiFi.h>
#include <WiFiUdp.h>

class DNSResolver {
public:
  static String getHostnameByIP(IPAddress ip) {
    WiFiUDP udp;
    uint8_t netbios[] = {
      0x80, 0x94, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
      0x00, 0x00, 0x00, 0x00, 0x20, 0x43, 0x4B, 0x41,
      0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
      0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
      0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41, 0x41,
      0x41, 0x41, 0x41, 0x41, 0x41, 0x00, 0x00, 0x21,
      0x00, 0x01
    };
    
    udp.begin(0);
    udp.beginPacket(ip, 137);
    udp.write(netbios, sizeof(netbios));
    udp.endPacket();
    
    uint32_t start = millis();
    while (millis() - start < 200) {
      int size = udp.parsePacket();
      if (size > 0) {
        uint8_t buf[256];
        udp.read(buf, size);
        if (size > 57) {
          char name[16];
          int idx = 0;
          for (int i = 57; i < size && idx < 15; i++) {
            if (buf[i] >= 32 && buf[i] <= 126 && buf[i] != ' ') {
              name[idx++] = buf[i];
            } else if (buf[i] == ' ' || buf[i] == 0) {
              break;
            }
          }
          name[idx] = 0;
          udp.stop();
          if (idx > 0) return String(name);
        }
      }
      delay(10);
    }
    udp.stop();
    return "";
  }
};

#endif

#ifndef ARP_SCANNER_H
#define ARP_SCANNER_H

#include <WiFi.h>
#include <lwip/etharp.h>
#include <lwip/netif.h>

class ARPScanner {
public:
  static bool isHostAlive(const char* ipStr) {
    IPAddress ip;
    if (!ip.fromString(ipStr)) return false;
    
    ip_addr_t addr;
    IP_ADDR4(&addr, ip[0], ip[1], ip[2], ip[3]);
    
    struct netif* netif = netif_default;
    if (!netif) return false;
    
    const ip4_addr_t* ip4_addr = ip_2_ip4(&addr);
    struct eth_addr* eth_ret = nullptr;
    const ip4_addr_t* ip_ret = nullptr;
    
    // Check ARP cache
    if (etharp_find_addr(netif, ip4_addr, &eth_ret, &ip_ret) >= 0) {
      return true;  // Found in ARP cache
    }
    
    // Send ARP request and wait briefly
    etharp_request(netif, ip4_addr);
    delay(10);
    
    // Check again
    return etharp_find_addr(netif, ip4_addr, &eth_ret, &ip_ret) >= 0;
  }
};

#endif

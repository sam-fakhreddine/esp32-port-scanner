# ESP32 Port Scanner - Architecture

## 📁 File Structure

```
esp32-port-scanner/
├── esp32-port-scanner.ino      # Main entry point
├── secrets.h                    # WiFi/MQTT credentials (gitignored)
├── secrets.h.example            # Template for credentials
├── .gitignore                   # Exclude secrets from git
├── src/
│   ├── core/
│   │   ├── config.h             # Configuration namespace
│   │   ├── interfaces.h         # Abstract interfaces
│   │   ├── data_structures.h    # Data models
│   │   ├── time_utils.h         # Time utilities
│   │   ├── result_store.h/.cpp  # Data storage + analytics
│   │   └── scan_orchestrator.h/.cpp # Scan coordination
│   ├── network/
│   │   ├── wifi_manager.h       # WiFi connection management
│   │   └── mqtt_manager.h       # MQTT publishing
│   ├── scanners/
│   │   ├── port_scanner.h       # Port scanning + fingerprinting
│   │   ├── wifi_scanner.h       # WiFi network scanner
│   │   ├── bluetooth_scanner.h  # BLE device scanner
│   │   ├── ble_reboot_scan.h    # BLE reboot handler
│   │   ├── arp_scanner.h        # ARP utilities
│   │   ├── arp_table.h          # ARP table scanner
│   │   ├── mdns_scanner.h       # mDNS service discovery
│   │   ├── smb_scanner.h        # SMB share scanner
│   │   └── dns_resolver.h       # NetBIOS name resolution
│   └── web/
│       ├── web_interface.h      # HTTP API handlers
│       ├── web_content.h        # HTML builder
│       ├── html_index.h         # HTML structure
│       ├── html_styles.h        # CSS styles
│       └── html_scripts.h       # JavaScript code
└── docs/
    └── ARCHITECTURE.md          # This file
```

## 🎯 SOLID Principles Applied

### Single Responsibility Principle (SRP)
Each class has ONE job:
- `WiFiManager` - WiFi connections only
- `MQTTManager` - MQTT operations only
- `PortScanner` - Port scanning + device fingerprinting
- `ResultStore` - Data storage + JSON serialization
- `ScanOrchestrator` - Scan coordination + task distribution
- `WebInterface` - HTTP request handling

### Open/Closed Principle (OCP)
- Interfaces allow extension without modification
- New scanners can implement `IPortScanner`
- New publishers can implement `IPublisher`

### Liskov Substitution Principle (LSP)
- Any `IConnectionManager` implementation is interchangeable
- Any `IPublisher` can replace another
- Any `IPortScanner` works with `ScanOrchestrator`

### Interface Segregation Principle (ISP)
- Small, focused interfaces
- `IConnectionManager` - connect/disconnect only
- `IPublisher` - publish only
- `IPortScanner` - scan operations only

### Dependency Inversion Principle (DIP)
- High-level modules depend on abstractions (interfaces)
- `ScanOrchestrator` depends on `IPortScanner`, not concrete `PortScanner`
- Easy to mock for testing

## 🔄 DRY (Don't Repeat Yourself)

### Eliminated Repetition:
1. **Config centralized** - One place for all settings
2. **Data structures shared** - No duplicate struct definitions
3. **Interfaces reused** - Common contracts across classes
4. **HTML separated** - CSS/JS/HTML in own files
5. **JSON serialization** - Centralized in `ResultStore`

## 📊 Component Dependencies

```
esp32-port-scanner.ino
    ├── src/core/config.h (includes secrets.h)
    ├── src/core/interfaces.h
    ├── src/core/data_structures.h
    ├── src/network/wifi_manager.h
    ├── src/network/mqtt_manager.h
    ├── src/scanners/port_scanner.h
    ├── src/scanners/wifi_scanner.h
    ├── src/scanners/bluetooth_scanner.h
    ├── src/scanners/ble_reboot_scan.h
    ├── src/scanners/mdns_scanner.h
    ├── src/scanners/arp_table.h
    ├── src/scanners/smb_scanner.h
    ├── src/core/result_store.h
    ├── src/core/scan_orchestrator.h
    └── src/web/web_interface.h
            └── src/web/web_content.h
                    ├── src/web/html_index.h
                    ├── src/web/html_styles.h
                    └── src/web/html_scripts.h
```

## 🚀 Benefits

### Maintainability
- **Easy to find code** - Each file has clear purpose
- **Easy to modify** - Change one file without affecting others
- **Easy to test** - Mock interfaces for unit tests

### Readability
- **Main file is 60 lines** - Down from 800+
- **Clear separation** - HTML/CSS/JS in own files
- **Self-documenting** - File names describe contents

### Extensibility
- **Add new scanners** - Implement `IPortScanner`
- **Add new publishers** - Implement `IPublisher`
- **Add new storage** - Replace `ResultStore`
- **Add new UI** - Replace `WebInterface`

## 🔧 How to Modify

### Change WiFi Credentials
Edit `secrets.h` (never commit this file):
```cpp
#define WIFI_SSID "YOUR_SSID"
#define WIFI_PASSWORD "YOUR_PASSWORD"
#define MQTT_SERVER "192.168.0.210"
#define MQTT_PORT 1883
```

### Modify HTML/CSS/JavaScript
Edit separate files:
- `src/web/html_index.h` - HTML structure
- `src/web/html_styles.h` - CSS styling
- `src/web/html_scripts.h` - JavaScript logic

### Add New Device Fingerprint
Edit `src/scanners/port_scanner.h` → `identifyDevice()` method

### Change Risk Scoring
Edit `src/scanners/port_scanner.h` → `calculateRiskScore()` method

### Add New API Endpoint
Edit `src/web/web_interface.h` → Add new `server.on()` handler

### Add New Scanner
1. Create new file in `src/scanners/`
2. Include in main `.ino` file
3. Add web endpoints in `src/web/web_interface.h`
4. Add UI tab in `src/web/html_index.h`

## 📦 Compilation

Arduino IDE automatically compiles:
- `.ino` file (main)
- `.cpp` files (implementations)
- `.h` files (headers - included automatically)

No makefile needed!

## 🎓 Learning Resources

- **SOLID Principles**: https://en.wikipedia.org/wiki/SOLID
- **DRY Principle**: https://en.wikipedia.org/wiki/Don%27t_repeat_yourself
- **C++ Header Files**: https://www.learncpp.com/cpp-tutorial/header-files/

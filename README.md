# ESP32 Port Scanner v3.0 - IoT Security Intelligence

**⚠️ EDUCATIONAL PURPOSE ONLY** - This project demonstrates IoT security vulnerabilities. Only scan networks you own or have explicit permission to test.

Professional network port scanner for ESP32 with advanced security intelligence, device fingerprinting, and risk assessment capabilities.

## 🔗 Links

- **GitHub Repository**: https://github.com/sam-fakhreddine/esp32-port-scanner
- **Live Presentation**: https://sam-fakhreddine.github.io/esp32-port-scanner/

## 🚨 Security Research Purpose

This tool demonstrates how easily IoT devices can:
- Scan and profile network devices
- Identify vulnerable services
- Collect device fingerprints
- Assess security risks
- Operate autonomously on home networks

**Key Insight**: A $10 ESP32 can perform sophisticated network reconnaissance, highlighting the importance of IoT security.

## 🚀 Features

### Core Scanning
- **Multi-threaded Scanning** - 4 parallel workers for fast scans
- **Service Banner Grabbing** - Collect HTTP headers, SSH versions, service info
- **Device Fingerprinting** - Automatically identify device types (IoT, Windows, Linux, Router)
- **Risk Scoring** - Calculate vulnerability scores (0-100%) based on exposed services

### Intelligence Gathering
- **Endpoint Profiling** - Track unique hosts with detailed metadata
- **Response Time Analysis** - Measure network latency per host
- **Temporal Tracking** - First seen, last seen, scan count
- **Service Detection** - Identify web servers, databases, remote access services

### Web Interface
- **Modern Dark UI** - Professional dashboard with real-time updates
- **Analytics Tab** - Scan statistics and metrics
- **Endpoints Tab** - Detailed host information with risk indicators
- **Configuration Tab** - Live scan parameter updates
- **Results Tab** - Real-time open port discoveries

### Integration
- **MQTT Publishing** - Stream findings to MQTT broker
- **JSON APIs** - RESTful endpoints for automation
- **Auto-scanning** - Configurable periodic scans

## 📊 Data Collection

### Per-Host Intelligence
- **Open Ports** - Complete list of accessible services
- **Device Type** - Windows PC, IoT Device, Router, Server, etc.
- **Risk Score** - 0-100% vulnerability assessment
- **Service Banners** - HTTP headers, SSH versions, service strings
- **Response Times** - Average connection latency
- **Temporal Data** - Discovery timeline and scan frequency

### Risk Scoring Algorithm
- **Critical (30 pts)**: Telnet (unencrypted remote access)
- **High (20 pts)**: FTP (unencrypted file transfer)
- **High (15 pts)**: RDP, SMB (Windows vulnerabilities)
- **Medium (10 pts)**: SSH, MySQL, PostgreSQL (exposed databases)
- **Low (5 pts)**: HTTP, HTTPS (web services)
- **Multiplier**: +2 pts per open port

### Device Fingerprinting
- **Windows PC**: RDP + SMB detected
- **IoT Device**: SSH + HTTP with few ports
- **IoT/MQTT Device**: MQTT broker detected
- **Router/Gateway**: UPnP service detected
- **Legacy Device**: Telnet without SSH
- **Linux Server**: SSH with many ports
- **Server**: HTTP + HTTPS with 10+ ports

## 🛠️ Hardware Requirements

- **ESP32 Development Board** (ESP-WROOM-32 or similar)
- **USB Cable** for programming
- **WiFi Network** access (2.4GHz)

Tested on: ELEGOO ESP-WROOM-32 Development Board

## 📦 Dependencies

Install via Arduino IDE Library Manager:

1. **ESP32 Board Support** (by Espressif Systems)
2. **PubSubClient** (by Nick O'Leary) - MQTT client
3. **ArduinoJson** (by Benoit Blanchon) - JSON parsing

## ⚙️ Configuration

Edit `esp32-port-scanner.ino`:

```cpp
namespace Config {
  struct WiFiConfig {
    const char* ssid = "YOUR_WIFI_SSID";        // Change this
    const char* password = "YOUR_WIFI_PASSWORD"; // Change this
  };
  
  struct MQTTConfig {
    const char* server = "192.168.0.210";       // Your MQTT broker
    const uint16_t port = 1883;                 // MQTT port
  };
  
  struct ScanConfig {
    char networkPrefix[20] = "192.168.0.";      // Your network
    uint8_t startIp = 1;
    uint8_t endIp = 254;
    uint16_t startPort = 1;
    uint16_t endPort = 1024;
    uint16_t timeoutMs = 100;
    uint32_t intervalMs = 300000;  // 5 minutes
  };
}
```

## 📤 Installation

### Step 1: Install Arduino IDE & ESP32 Support

1. Download [Arduino IDE](https://www.arduino.cc/en/software)
2. **File → Preferences** → Add to "Additional Board Manager URLs":
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
3. **Tools → Board → Boards Manager** → Install "esp32 by Espressif Systems"

### Step 2: Install Libraries

**Tools → Manage Libraries**, search and install:
- PubSubClient
- ArduinoJson

### Step 3: Configure & Upload

1. Open `esp32-port-scanner.ino`
2. Update WiFi credentials and MQTT settings
3. Select board: **Tools → Board → ESP32 Dev Module**
4. Select port: **Tools → Port → /dev/cu.usbserial-XXXX**
5. Click **Upload**

## 🎮 Usage

### Web Interface

1. **Find ESP32 IP**: Check Serial Monitor (115200 baud) after boot
2. **Open browser**: Navigate to `http://[ESP32_IP]`
3. **Tabs**:
   - **Scan**: Start manual scans
   - **Analytics**: View scan statistics and metrics
   - **Endpoints**: Detailed host profiles with risk scores
   - **Config**: Adjust scan parameters
   - **Results**: Real-time port discoveries

### Serial Monitor Output

```
[SCAN] Starting: 192.168.0.1-254, ports 1-1024
[SCAN] 192.168.0.105:22 OPEN [SSH-2.0-OpenSSH_8.2]
[SCAN] 192.168.0.105:80 OPEN [HTTP/1.1 200 OK]
[INFO] 192.168.0.105: IoT Device (Risk: 25%, Ports: 2)
[SCAN] Complete: 45 seconds, 2 open ports, 12 IPs, 12288 ports checked
```

### MQTT Topics

Results published to:
- `portscan/status` - Scan lifecycle events
- `portscan/192.168.0.X` - Individual host findings

Example payload:
```json
{
  "port": 80,
  "state": "open",
  "banner": "HTTP/1.1 200 OK"
}
```

## 🏗️ Architecture

### SOLID Design Principles

- **Single Responsibility**: Each class has one job
  - `WiFiManager` - WiFi connections
  - `MQTTManager` - MQTT operations
  - `PortScanner` - Port scanning + fingerprinting
  - `ResultStore` - Data storage + analytics
  - `ScanOrchestrator` - Scan coordination
  - `WebInterface` - HTTP API handling

- **Open/Closed**: Easy to extend via interfaces
- **Liskov Substitution**: Interface implementations are interchangeable
- **Interface Segregation**: Small, focused interfaces
- **Dependency Inversion**: High-level modules depend on abstractions

### Component Flow

```
┌─────────────────────────────────────┐
│      Web Interface (Port 80)        │
│   5 Tabs: Scan/Analytics/Endpoints  │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│      Scan Orchestrator              │
│  • Task distribution                │
│  • Non-blocking execution           │
└──┬─────────────────────────┬────────┘
   │                         │
┌──▼──────────┐    ┌────────▼────────┐
│ Port Scanner│    │  Result Store   │
│ • Scanning  │    │  • Endpoints    │
│ • Banners   │    │  • Statistics   │
│ • Fingerpr. │    │  • Risk Scores  │
│ • Risk Calc │    │  • Analytics    │
└─────────────┘    └─────────────────┘
```

## 📊 API Endpoints

- `GET /` - Web interface
- `POST /scan` - Start scan
- `GET /status` - System status
- `GET /results` - Recent port discoveries
- `GET /endpoints` - Host profiles with intelligence
- `GET /stats` - Scan statistics
- `GET /config` - Current configuration
- `POST /config` - Update configuration
- `POST /clear` - Clear results

## 🔧 Troubleshooting

### Compilation Errors
- Ensure all libraries are installed
- Update ESP32 board package to latest version
- Check Arduino IDE version (2.x recommended)

### WiFi Connection Failed
- Verify SSID/password
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
- Check network allows new devices

### Slow Scans
- Reduce port range (e.g., 1-100 instead of 1-1024)
- Increase timeout (default 100ms)
- Reduce IP range for testing

### Memory Issues
- Auto-restart triggers at <10KB free heap
- Reduce maxResults in ResultStore if needed
- Monitor Serial output for memory warnings

## 📈 Performance

- **Scan Speed**: ~30-60 seconds for 254 IPs × 1024 ports
- **Memory Usage**: ~48KB RAM, ~960KB Flash
- **Concurrent Workers**: 4 threads on dual-core CPU
- **Result Capacity**: 100 recent discoveries (auto-cleanup)
- **Endpoint Tracking**: Unlimited unique hosts

## 🔐 Security & Ethics

### Legal Considerations
- **Only scan networks you own or have permission to test**
- Unauthorized network scanning may be illegal in your jurisdiction
- This tool is for educational and authorized security research only

### Security Notes
- Change default WiFi credentials before deployment
- MQTT connections are unencrypted by default
- Web interface has no authentication (add if deploying publicly)
- Consider implementing HTTPS/TLS for production use

### Responsible Disclosure
If you discover vulnerabilities using this tool:
1. Document findings professionally
2. Notify device owners/manufacturers privately
3. Allow reasonable time for patches
4. Follow coordinated disclosure practices

## 🎯 Use Cases

### Educational
- Learn network protocols and security
- Understand IoT vulnerabilities
- Practice responsible security research

### Home Network Audit
- Discover forgotten devices
- Identify insecure services (Telnet, FTP)
- Assess IoT device security posture

### Security Research
- IoT device fingerprinting
- Service enumeration
- Vulnerability assessment

## 🤝 Contributing

Contributions welcome! Areas for improvement:
- MAC address resolution (ARP)
- Hostname lookup (mDNS/DNS)
- Additional device fingerprints
- Export functionality (CSV/JSON download)
- Authentication system
- HTTPS/TLS support
- AWS IoT Core integration

## 📝 License

MIT License - Free to modify and distribute for educational purposes.

## ⚠️ Disclaimer

This tool is provided for educational and authorized security testing purposes only. The authors are not responsible for misuse or damage caused by this program. Always obtain proper authorization before scanning networks.

## 📧 Support

For issues or questions:
- Open an issue on [GitHub](https://github.com/sam-fakhreddine/esp32-port-scanner/issues)
- Check Serial Monitor output (115200 baud)
- Review Arduino IDE compilation errors
- Verify network connectivity and permissions

## 🎤 Presentation

View the interactive presentation about IoT security vulnerabilities:
- **Live Demo**: https://sam-fakhreddine.github.io/esp32-port-scanner/
- **Source**: `presentation/` directory

---

**Remember**: With great power comes great responsibility. Use this tool ethically and legally.

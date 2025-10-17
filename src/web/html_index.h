#ifndef HTML_INDEX_H
#define HTML_INDEX_H

const char HTML_INDEX[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width,initial-scale=1.0">
  <title>ESP32 Port Scanner</title>
  <style>%CSS%</style>
</head>
<body>
  <div id="scanBanner" class="scan-banner" style="display:none">
    <div class="scan-banner-content">
      <div class="scan-banner-title">🔍 Scanning in Progress</div>
      <div class="scan-banner-details" id="scanDetails">Initializing...</div>
      <div class="scan-progress-bar">
        <div class="scan-progress-fill" id="progressBar" style="width:0%"></div>
      </div>
    </div>
  </div>
  
  <h1>ESP32 Port Scanner v4.0</h1>
  
  <div class="card">
    <div class="status" id="status">Loading...</div>
  </div>
  
  <div class="tabs">
    <div class="tab active" onclick="showTab('scan')">Scan</div>
    <div class="tab" onclick="showTab('analytics')">Analytics</div>
    <div class="tab" onclick="showTab('endpoints')">Endpoints</div>
    <div class="tab" onclick="showTab('wifi')">WiFi</div>
    <div class="tab" onclick="showTab('bluetooth')">Bluetooth</div>
    <div class="tab" onclick="showTab('services')">Services</div>
    <div class="tab" onclick="showTab('arp')">ARP Table</div>
    <div class="tab" onclick="showTab('smb')">SMB Shares</div>
    <div class="tab" onclick="showTab('config')">Config</div>
    <div class="tab" onclick="showTab('results')">Results</div>
  </div>
  
  <div id="scan" class="tab-content active">
    <div class="card">
      <button id="scanBtn" onclick="startScan()">Start Scan Now</button>
      <button id="pauseBtn" class="secondary" onclick="pauseScan()" style="display:none">⏸ Pause Scan</button>
      <button id="resumeBtn" class="secondary" onclick="resumeScan()" style="display:none">▶ Resume Scan</button>
    </div>
  </div>
  
  <div id="analytics" class="tab-content">
    <div class="card">
      <h3>Scan Statistics</h3>
      <div class="stat-grid" id="statsGrid">Loading...</div>
    </div>
  </div>
  
  <div id="endpoints" class="tab-content">
    <div class="card">
      <h3>Discovered Endpoints</h3>
      <div id="endpointsList">Loading...</div>
    </div>
  </div>
  
  <div id="wifi" class="tab-content">
    <div class="card">
      <button onclick="scanWiFi()" class="secondary">📡 Scan WiFi Networks</button>
      <h3>Nearby WiFi Networks</h3>
      <div id="wifiList">Click scan to discover networks</div>
    </div>
  </div>
  
  <div id="bluetooth" class="tab-content">
    <div class="card">
      <button onclick="scanBluetooth()" class="secondary">📱 Scan Bluetooth Devices</button>
      <button onclick="clearBluetooth()" class="secondary">🗑️ Clear History</button>
      <h3>Nearby Bluetooth Devices</h3>
      <div id="bluetoothList">Click scan to discover devices</div>
    </div>
  </div>
  
  <div id="services" class="tab-content">
    <div class="card">
      <button onclick="scanMDNS()" class="secondary">🔍 Discover Services</button>
      <h3>Advertised Services (mDNS)</h3>
      <div id="servicesList">Click scan to discover services</div>
    </div>
  </div>
  
  <div id="arp" class="tab-content">
    <div class="card">
      <button onclick="scanARP()" class="secondary">🔄 Refresh ARP Table</button>
      <h3>ARP Cache</h3>
      <div id="arpList">Click refresh to view ARP table</div>
    </div>
  </div>
  
  <div id="smb" class="tab-content">
    <div class="card">
      <button onclick="scanSMB()" class="secondary">🔍 Scan for SMB Shares</button>
      <h3>SMB/CIFS File Shares</h3>
      <div id="smbList">Click scan to discover shares</div>
    </div>
  </div>
  
  <div id="config" class="tab-content">
    <div class="card">
      <form id="configForm" onsubmit="saveConfig(event)">
        <div class="form-group">
          <label>Network Prefix:</label>
          <input name="prefix" id="prefix" value="192.168.0.">
        </div>
        <div class="form-group">
          <label>Start IP:</label>
          <input name="startIp" id="startIp" type="number" min="1" max="254" value="1">
        </div>
        <div class="form-group">
          <label>End IP:</label>
          <input name="endIp" id="endIp" type="number" min="1" max="254" value="254">
        </div>
        <div class="form-group">
          <label>Start Port:</label>
          <input name="startPort" id="startPort" type="number" min="1" max="65535" value="1">
        </div>
        <div class="form-group">
          <label>End Port:</label>
          <input name="endPort" id="endPort" type="number" min="1" max="65535" value="1024">
        </div>
        <div class="form-group">
          <label>Timeout (ms):</label>
          <input name="timeout" id="timeout" type="number" min="10" max="5000" value="100">
        </div>
        <div class="form-group">
          <label>Scan Interval (seconds):</label>
          <input name="interval" id="interval" type="number" min="10" max="86400" value="300">
        </div>
        <div class="form-group">
          <label>Worker Threads (1-32):</label>
          <input name="workers" id="workers" type="number" min="1" max="32" value="12">
        </div>
        <div class="form-group" style="display:flex;align-items:center;gap:10px">
          <input type="checkbox" name="enableMQTT" id="enableMQTT" value="true" style="width:auto">
          <label style="margin:0">Enable MQTT Publishing</label>
        </div>
        <div class="form-group" style="display:flex;align-items:center;gap:10px">
          <input type="checkbox" name="enableBannerGrab" id="enableBannerGrab" value="true" checked style="width:auto">
          <label style="margin:0">Enable Banner Grabbing</label>
        </div>
        <div class="form-group" style="display:flex;align-items:center;gap:10px">
          <input type="checkbox" name="skipHostCheck" id="skipHostCheck" value="true" style="width:auto">
          <label style="margin:0">Skip Host Alive Check (faster)</label>
        </div>
        <button type="submit">Save Configuration</button>
      </form>
    </div>
  </div>
  
  <div id="results" class="tab-content">
    <div class="card">
      <button onclick="clearResults()" class="secondary">Clear Results</button>
      <div class="results" id="resultsList">Loading...</div>
    </div>
  </div>
  
  <script>%JAVASCRIPT%</script>
</body>
</html>
)rawliteral";

#endif

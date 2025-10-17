#ifndef HTML_SCRIPTS_H
#define HTML_SCRIPTS_H

const char HTML_SCRIPTS[] PROGMEM = R"rawliteral(
function showTab(t) {
  document.querySelectorAll('.tab').forEach(e => e.classList.remove('active'));
  document.querySelectorAll('.tab-content').forEach(e => e.classList.remove('active'));
  event.target.classList.add('active');
  document.getElementById(t).classList.add('active');
  if (t === 'results') updateResults();
  if (t === 'config') loadConfig();
  if (t === 'analytics') loadStats();
  if (t === 'endpoints') loadEndpoints();
  if (t === 'wifi') loadWiFiNetworks();
  if (t === 'bluetooth') loadBluetoothDevices();
  if (t === 'services') loadMDNSServices();
  if (t === 'arp') loadARPTable();
  if (t === 'smb') loadSMBShares();
}

function updateStatus() {
  fetch('/status').then(r => r.json()).then(d => {
    const st = document.getElementById('status');
    st.className = d.scanning ? 'status scanning' : 'status';
    var h = '<div class="info"><span>Status:</span><span>' + (d.scanning ? 'Scanning' : 'Idle') + '</span></div>';
    h += '<div class="info"><span>Results:</span><span>' + d.results + '</span></div>';
    h += '<div class="info"><span>Free Heap:</span><span>' + d.freeHeap + ' bytes</span></div>';
    h += '<div class="info"><span>Uptime:</span><span>' + d.uptime + 's</span></div>';
    st.innerHTML = h;
    document.getElementById('scanBtn').disabled = d.scanning;
    
    const pauseBtn = document.getElementById('pauseBtn');
    const resumeBtn = document.getElementById('resumeBtn');
    if (d.scanning && !d.paused) {
      pauseBtn.style.display = 'block';
      resumeBtn.style.display = 'none';
    } else if (d.scanning && d.paused) {
      pauseBtn.style.display = 'none';
      resumeBtn.style.display = 'block';
    } else {
      pauseBtn.style.display = 'none';
      resumeBtn.style.display = 'none';
    }
    
    const banner = document.getElementById('scanBanner');
    if (d.scanning && d.currentIP) {
      banner.style.display = 'block';
      var details = '';
      if (d.currentPort === 0) {
        details = 'Checking host ' + d.currentIP + ' • ' + d.ipsScanned + '/' + d.totalIPs + ' IPs checked';
      } else {
        details = 'Scanning ' + d.currentIP + ':' + d.currentPort + ' • ' + d.ipsScanned + '/' + d.totalIPs + ' IPs • ' + d.portsScanned + '/' + d.totalPorts + ' ports';
      }
      document.getElementById('scanDetails').textContent = details;
      document.getElementById('progressBar').style.width = d.percentComplete + '%';
      updateScanTable();
    } else {
      banner.style.display = 'none';
    }
  }).catch(e => console.error(e));
}

function loadConfig() {
  fetch('/config').then(r => r.json()).then(d => {
    document.getElementById('prefix').value = d.prefix;
    document.getElementById('startIp').value = d.startIp;
    document.getElementById('endIp').value = d.endIp;
    document.getElementById('startPort').value = d.startPort;
    document.getElementById('endPort').value = d.endPort;
    document.getElementById('timeout').value = d.timeout;
    document.getElementById('interval').value = d.interval;
    document.getElementById('workers').value = d.workers || 12;
    document.getElementById('enableMQTT').checked = d.enableMQTT || false;
    document.getElementById('enableBannerGrab').checked = d.enableBannerGrab !== false;
    document.getElementById('skipHostCheck').checked = d.skipHostCheck || false;
  }).catch(e => console.error(e));
}

function saveConfig(e) {
  e.preventDefault();
  const f = new FormData(e.target);
  fetch('/config', {
    method: 'POST',
    body: new URLSearchParams(f)
  }).then(r => r.json()).then(() => {
    alert('Configuration saved!');
  }).catch(() => alert('Failed to save'));
}

function loadStats() {
  fetch('/stats').then(r => r.json()).then(d => {
    var h = '<div class="stat-box"><div class="stat-value">' + d.totalScans + '</div><div class="stat-label">Total Scans</div></div>';
    h += '<div class="stat-box"><div class="stat-value">' + d.uniqueHosts + '</div><div class="stat-label">Unique Hosts</div></div>';
    h += '<div class="stat-box"><div class="stat-value">' + d.totalOpenPorts + '</div><div class="stat-label">Open Ports Found</div></div>';
    h += '<div class="stat-box"><div class="stat-value">' + d.totalIpsScanned + '</div><div class="stat-label">IPs Scanned</div></div>';
    h += '<div class="stat-box"><div class="stat-value">' + d.totalPortsChecked + '</div><div class="stat-label">Ports Checked</div></div>';
    h += '<div class="stat-box"><div class="stat-value">' + d.lastScanDuration + 's</div><div class="stat-label">Last Scan Duration</div></div>';
    document.getElementById('statsGrid').innerHTML = h;
  }).catch(e => console.error(e));
}

function loadEndpoints() {
  fetch('/endpoints').then(r => r.json()).then(d => {
    const l = document.getElementById('endpointsList');
    if (d.length === 0) {
      l.innerHTML = '<div style="text-align:center;padding:40px;color:#8b949e">No endpoints discovered</div>';
    } else {
      var h = '';
      d.forEach(e => {
        var risk = e.riskScore >= 50 ? 'high-risk' : e.riskScore >= 25 ? 'medium-risk' : 'low-risk';
        var badge = e.riskScore >= 50 ? 'badge-critical' : e.riskScore >= 25 ? 'badge-high' : 'badge-low';
        h += '<div class="endpoint ' + risk + '">';
        h += '<div style="display:flex;justify-content:space-between;align-items:center">';
        h += '<div><strong style="color:#58a6ff;font-size:18px">' + e.ip + '</strong>';
        if (e.hostname && e.hostname !== 'Unknown') {
          h += '<div style="font-size:13px;color:#8b949e;margin-top:2px">' + e.hostname + '</div>';
        }
        h += '<span class="badge ' + badge + '" style="margin-left:10px">Risk: ' + e.riskScore + '%</span></div>';
        h += '<div style="color:#8b949e">' + e.deviceType + '</div></div>';
        h += '<div style="margin-top:10px;color:#8b949e;font-size:13px">Ports (' + e.portCount + '): ' + e.ports.join(', ') + '</div>';
        if (e.banners && e.banners.length > 0) {
          h += '<div style="margin-top:8px;font-size:12px;color:#d29922">Services: ' + e.banners.join(' | ') + '</div>';
        }
        if (e.riskFindings && e.riskFindings.length > 0) {
          h += '<div style="margin-top:10px;padding:10px;background:#0d1117;border-radius:4px">';
          h += '<div style="font-weight:600;margin-bottom:5px;color:#f85149">Risk Breakdown:</div>';
          e.riskFindings.forEach(rf => {
            var sevColor = rf.severity === 'Critical' ? '#f85149' : rf.severity === 'High' ? '#d29922' : rf.severity === 'Medium' ? '#58a6ff' : '#8b949e';
            h += '<div style="display:flex;justify-content:space-between;padding:4px 0;border-bottom:1px solid #21262d">';
            h += '<span style="color:' + sevColor + '">Port ' + rf.port + ' (' + rf.service + ')</span>';
            h += '<span><span style="color:' + sevColor + ';font-weight:600">' + rf.severity + '</span> +' + rf.points + ' pts</span>';
            h += '</div>';
          });
          h += '</div>';
        }
        var seenTime = e.firstSeen > 1000000000 ? new Date(e.firstSeen).toLocaleString() : Math.floor(e.firstSeen / 1000) + 's uptime';
        h += '<div style="margin-top:8px;font-size:12px;color:#484f58">Response: ' + e.avgResponseTime + 'ms | First: ' + seenTime + '</div>';
        h += '</div>';
      });
      l.innerHTML = h;
    }
  }).catch(e => console.error(e));
}

var scanTableData = {};

function updateScanTable() {
  fetch('/endpoints').then(r => r.json()).then(d => {
    const l = document.getElementById('resultsList');
    if (d.length === 0) {
      l.innerHTML = '<div style="text-align:center;padding:40px;color:#8b949e">Scanning...</div>';
    } else {
      var h = '<table style="width:100%;border-collapse:collapse">';
      h += '<thead><tr style="border-bottom:2px solid #21262d"><th style="text-align:left;padding:10px;color:#8b949e">Device</th><th style="text-align:left;padding:10px;color:#8b949e">Ports</th></tr></thead><tbody>';
      d.forEach(e => {
        h += '<tr style="border-bottom:1px solid #21262d">';
        var deviceName = e.ip;
        if (e.hostname && e.hostname !== 'Unknown') {
          deviceName += '<div style="font-size:12px;color:#8b949e;margin-top:2px">' + e.hostname + '</div>';
        }
        h += '<td style="padding:10px"><strong style="color:#58a6ff">' + deviceName + '</strong></td>';
        h += '<td style="padding:10px;color:#8b949e">' + (e.ports.length > 0 ? e.ports.join(', ') : '<span style="color:#484f58">scanning...</span>') + '</td>';
        h += '</tr>';
      });
      h += '</tbody></table>';
      l.innerHTML = h;
    }
  }).catch(e => console.error(e));
}

function updateResults() {
  updateScanTable();
}

function clearResults() {
  if (confirm('Clear all results?')) {
    fetch('/clear', { method: 'POST' }).then(() => updateResults()).catch(e => console.error(e));
  }
}

function startScan() {
  fetch('/scan', { method: 'POST' }).then(r => r.json()).then(() => {
    alert('Scan started!');
    updateStatus();
  }).catch(() => alert('Failed'));
}

function pauseScan() {
  fetch('/pause', { method: 'POST' }).then(r => r.json()).then(() => {
    updateStatus();
  }).catch(() => alert('Failed'));
}

function resumeScan() {
  fetch('/resume', { method: 'POST' }).then(r => r.json()).then(() => {
    updateStatus();
  }).catch(() => alert('Failed'));
}

function scanWiFi() {
  document.getElementById('wifiList').innerHTML = '<div style="text-align:center;padding:40px;color:#8b949e">Scanning...</div>';
  fetch('/wifi/scan', { method: 'POST' }).then(r => r.json()).then(() => {
    loadWiFiNetworks();
  }).catch(() => alert('WiFi scan failed'));
}

function loadWiFiNetworks() {
  fetch('/wifi/networks').then(r => r.json()).then(d => {
    const l = document.getElementById('wifiList');
    if (d.length === 0) {
      l.innerHTML = '<div style="text-align:center;padding:40px;color:#8b949e">No networks found</div>';
    } else {
      var h = '';
      d.forEach(n => {
        var signalClass = n.rssi > -60 ? 'signal-strong' : n.rssi > -75 ? 'signal-medium' : 'signal-weak';
        var bars = n.rssi > -60 ? '████' : n.rssi > -75 ? '███░' : '██░░';
        var networkClass = n.isOpen ? 'open' : n.isWeak ? 'weak' : '';
        var warning = n.isOpen ? ' ⚠️' : n.isWeak ? ' ⚠️' : ' ✓';
        
        h += '<div class="wifi-network ' + networkClass + '">';
        h += '<div><strong style="font-size:16px">' + n.ssid + '</strong>' + warning;
        h += '<div style="font-size:12px;color:#8b949e;margin-top:4px">' + n.bssid + ' • Ch ' + n.channel + ' • ' + n.encryption + '</div></div>';
        h += '<div class="wifi-signal"><span class="signal-bars ' + signalClass + '">' + bars + '</span>';
        h += '<span style="color:#8b949e">' + n.rssi + ' dBm</span></div></div>';
      });
      l.innerHTML = h;
    }
  }).catch(e => console.error(e));
}

function scanBluetooth() {
  if (!confirm('⚠️ Bluetooth scan will disable WiFi for ~30 seconds.\n\nThe web page will be inaccessible during this time.\n\nContinue?')) {
    return;
  }
  document.getElementById('bluetoothList').innerHTML = '<div style="text-align:center;padding:40px;color:#f85149"><strong>⚠️ WiFi Disabled - Scanning Bluetooth</strong><br><br>Scanning for 30 seconds...<br><br>Page will reconnect automatically</div>';
  fetch('/bluetooth/scan', { method: 'POST' }).then(r => r.json()).then(() => {
    setTimeout(() => loadBluetoothDevices(), 35000);
  }).catch(() => {
    document.getElementById('bluetoothList').innerHTML = '<div style="text-align:center;padding:40px;color:#f85149">Scan in progress - page will reconnect soon</div>';
  });
}

function clearBluetooth() {
  if (confirm('Clear Bluetooth device history?')) {
    fetch('/bluetooth/clear', { method: 'POST' }).then(() => {
      loadBluetoothDevices();
    }).catch(e => console.error(e));
  }
}

function loadBluetoothDevices() {
  fetch('/bluetooth/devices').then(r => r.json()).then(d => {
    const l = document.getElementById('bluetoothList');
    if (d.length === 0) {
      l.innerHTML = '<div style="text-align:center;padding:40px;color:#8b949e">No devices found</div>';
    } else {
      var h = '';
      d.forEach(dev => {
        var deviceClass = dev.isRandom ? 'random' : '';
        var icon = '📱';
        var privacy = dev.isRandom ? ' 🔒' : '';
        
        h += '<div class="bt-device ' + deviceClass + '">';
        var displayName = dev.name === '<Unknown>' ? '<span style="color:#484f58;font-style:italic">No Name</span>' : dev.name;
        h += '<div>' + icon + ' <strong style="font-size:16px">' + displayName + '</strong>' + privacy;
        h += '<div style="font-size:12px;color:#8b949e;margin-top:4px">' + dev.address + ' • ~' + dev.distance + 'm away</div>';
        if (dev.manufacturer) {
          h += '<div style="font-size:11px;color:#58a6ff;margin-top:2px">Manufacturer: ' + dev.manufacturer + '</div>';
        }
        if (dev.services && dev.services.length > 0) {
          h += '<div style="font-size:11px;color:#d29922;margin-top:2px">Services: ' + dev.services.join(', ') + '</div>';
        }
        if (dev.appearance) {
          h += '<div style="font-size:11px;color:#8b949e;margin-top:2px">Appearance: 0x' + dev.appearance.toString(16).toUpperCase() + '</div>';
        }
        if (dev.txPower) {
          h += '<div style="font-size:11px;color:#8b949e;margin-top:2px">TX Power: ' + dev.txPower + ' dBm</div>';
        }
        var firstTime = dev.firstSeen > 1000000000 ? new Date(dev.firstSeen).toLocaleTimeString() : Math.floor(dev.firstSeen / 1000) + 's uptime';
        var lastTime = dev.lastSeen > 1000000000 ? new Date(dev.lastSeen).toLocaleTimeString() : Math.floor(dev.lastSeen / 1000) + 's uptime';
        h += '<div style="font-size:11px;color:#484f58;margin-top:2px">First: ' + firstTime + ' | Last: ' + lastTime + '</div></div>';
        h += '<div style="text-align:right"><span style="color:#8b949e">' + dev.rssi + ' dBm</span></div></div>';
      });
      l.innerHTML = h;
    }
  }).catch(e => console.error(e));
}

setInterval(updateStatus, 1000);
function scanMDNS() {
  document.getElementById('servicesList').innerHTML = '<div style="text-align:center;padding:40px;color:#8b949e">Scanning...</div>';
  fetch('/mdns/scan', { method: 'POST' }).then(r => r.json()).then(() => {
    loadMDNSServices();
  }).catch(() => alert('mDNS scan failed'));
}

function loadMDNSServices() {
  fetch('/mdns/services').then(r => r.json()).then(d => {
    const l = document.getElementById('servicesList');
    if (d.length === 0) {
      l.innerHTML = '<div style="text-align:center;padding:40px;color:#8b949e">No services found</div>';
    } else {
      var h = '';
      d.forEach(s => {
        h += '<div class="service-item">';
        h += '<div>' + s.icon + ' <strong style="font-size:16px">' + s.description + '</strong>';
        h += '<div style="font-size:12px;color:#8b949e;margin-top:4px">' + s.hostname + ' • ' + s.service + '</div></div>';
        h += '<div style="text-align:right"><div style="color:#58a6ff">' + s.ip + ':' + s.port + '</div></div></div>';
      });
      l.innerHTML = h;
    }
  }).catch(e => console.error(e));
}

setInterval(() => {
  var active = document.querySelector('.tab-content.active');
  if (!active) return;
  if (active.id === 'results') updateResults();
  if (active.id === 'analytics') loadStats();
  if (active.id === 'endpoints') loadEndpoints();
  if (active.id === 'wifi') loadWiFiNetworks();
  if (active.id === 'bluetooth') loadBluetoothDevices();
  if (active.id === 'services') loadMDNSServices();
  if (active.id === 'arp') loadARPTable();
  if (active.id === 'smb') loadSMBShares();
}, 5000);

function scanARP() {
  document.getElementById('arpList').innerHTML = '<div style="text-align:center;padding:40px;color:#8b949e">Scanning...</div>';
  fetch('/arp/scan', { method: 'POST' }).then(r => r.json()).then(() => {
    loadARPTable();
  }).catch(() => alert('ARP scan failed'));
}

function loadARPTable() {
  fetch('/arp/table').then(r => r.json()).then(d => {
    const l = document.getElementById('arpList');
    if (d.length === 0) {
      l.innerHTML = '<div style="text-align:center;padding:40px;color:#8b949e">No ARP entries</div>';
    } else {
      var h = '';
      d.forEach(a => {
        h += '<div class="service-item">';
        h += '<div>💻 <strong style="font-size:16px">' + a.ip + '</strong>';
        h += '<div style="font-size:12px;color:#8b949e;margin-top:4px">' + a.mac + ' • ' + a.vendor + '</div></div>';
        h += '<div style="text-align:right"><div style="color:#58a6ff">Layer 2</div></div></div>';
      });
      l.innerHTML = h;
    }
  }).catch(e => console.error(e));
}

function scanSMB() {
  document.getElementById('smbList').innerHTML = '<div style="text-align:center;padding:40px;color:#8b949e">Scanning network for SMB shares...</div>';
  fetch('/smb/scan', { method: 'POST' }).then(r => r.json()).then(() => {
    loadSMBShares();
  }).catch(() => alert('SMB scan failed'));
}

function loadSMBShares() {
  fetch('/smb/shares').then(r => r.json()).then(d => {
    const l = document.getElementById('smbList');
    if (d.length === 0) {
      l.innerHTML = '<div style="text-align:center;padding:40px;color:#8b949e">No SMB shares found</div>';
    } else {
      var h = '';
      d.forEach(s => {
        var riskClass = s.nullSession ? 'high-risk' : '';
        var riskBadge = s.nullSession ? '<span class="badge badge-critical" style="margin-left:10px">⚠️ NULL SESSION</span>' : '';
        h += '<div class="endpoint ' + riskClass + '">';
        h += '<div>📁 <strong style="font-size:16px">' + s.ip + ':' + s.port + '</strong>' + riskBadge;
        if (s.hostname && s.hostname !== 'Unknown') {
          h += '<div style="font-size:12px;color:#8b949e;margin-top:4px">' + s.hostname + '</div>';
        }
        if (s.nullSession) {
          h += '<div style="font-size:13px;color:#f85149;margin-top:8px">⚠️ Anonymous access allowed - CRITICAL vulnerability!</div>';
        }
        h += '</div></div>';
      });
      l.innerHTML = h;
    }
  }).catch(e => console.error(e));
}

updateStatus();
)rawliteral";

#endif

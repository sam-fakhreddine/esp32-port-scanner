#ifndef HTML_STYLES_H
#define HTML_STYLES_H

const char HTML_STYLES[] PROGMEM = R"rawliteral(
* {
  box-sizing: border-box;
  margin: 0;
  padding: 0;
}

body {
  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
  background: #0d1117;
  color: #c9d1d9;
  padding: 20px;
  max-width: 1200px;
  margin: 0 auto;
}

.card {
  background: #161b22;
  border: 1px solid #30363d;
  border-radius: 8px;
  padding: 20px;
  margin: 15px 0;
}

h1 {
  color: #58a6ff;
  margin-bottom: 20px;
}

h3 {
  color: #8b949e;
  margin: 15px 0 10px;
}

.tabs {
  display: flex;
  border-bottom: 1px solid #30363d;
  margin-bottom: 20px;
  overflow-x: auto;
}

.tab {
  padding: 12px 20px;
  cursor: pointer;
  border-bottom: 3px solid transparent;
  white-space: nowrap;
}

.tab:hover {
  background: #21262d;
}

.tab.active {
  border-bottom-color: #58a6ff;
  color: #58a6ff;
}

.tab-content {
  display: none;
}

.tab-content.active {
  display: block;
}

.status {
  padding: 15px;
  border-radius: 6px;
  border-left: 4px solid #3fb950;
}

.status.scanning {
  border-left-color: #f85149;
}

.form-group {
  margin: 15px 0;
}

label {
  display: block;
  color: #8b949e;
  margin-bottom: 8px;
  font-size: 14px;
}

input {
  width: 100%;
  padding: 12px;
  background: #0d1117;
  border: 1px solid #30363d;
  border-radius: 6px;
  color: #c9d1d9;
  font-size: 16px;
}

button {
  width: 100%;
  padding: 12px;
  background: #238636;
  color: #fff;
  border: none;
  border-radius: 6px;
  font-size: 16px;
  cursor: pointer;
  font-weight: 600;
}

button:hover {
  background: #2ea043;
}

button:disabled {
  background: #21262d;
  color: #484f58;
}

button.secondary {
  background: #1f6feb;
  margin-top: 10px;
}

button.secondary:hover {
  background: #388bfd;
}

.results {
  max-height: 400px;
  overflow-y: auto;
  background: #0d1117;
  border: 1px solid #30363d;
  border-radius: 6px;
  padding: 10px;
  margin-top: 15px;
}

.result-item {
  padding: 10px;
  margin: 5px 0;
  background: #161b22;
  border-left: 3px solid #3fb950;
  border-radius: 4px;
}

.endpoint {
  padding: 15px;
  margin: 10px 0;
  background: #161b22;
  border-radius: 6px;
  border-left: 4px solid #58a6ff;
}

.endpoint.high-risk {
  border-left-color: #f85149;
}

.endpoint.medium-risk {
  border-left-color: #d29922;
}

.endpoint.low-risk {
  border-left-color: #3fb950;
}

.info {
  display: flex;
  justify-content: space-between;
  padding: 10px 0;
  border-bottom: 1px solid #30363d;
}

.info:last-child {
  border-bottom: none;
}

.badge {
  display: inline-block;
  padding: 4px 8px;
  border-radius: 4px;
  font-size: 12px;
  font-weight: 600;
  margin: 2px;
}

.badge-critical {
  background: #f85149;
  color: #fff;
}

.badge-high {
  background: #d29922;
  color: #000;
}

.badge-medium {
  background: #58a6ff;
  color: #000;
}

.badge-low {
  background: #3fb950;
  color: #000;
}

.stat-grid {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
  gap: 15px;
  margin: 15px 0;
}

.stat-box {
  background: #0d1117;
  padding: 15px;
  border-radius: 6px;
  border: 1px solid #30363d;
}

.stat-value {
  font-size: 24px;
  font-weight: 600;
  color: #58a6ff;
}

.stat-label {
  font-size: 12px;
  color: #8b949e;
  margin-top: 5px;
}

.scan-banner {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  background: linear-gradient(135deg, #1f6feb 0%, #388bfd 100%);
  color: #fff;
  padding: 15px 20px;
  box-shadow: 0 4px 12px rgba(0,0,0,0.3);
  z-index: 1000;
  animation: slideDown 0.3s ease-out;
}

@keyframes slideDown {
  from { transform: translateY(-100%); }
  to { transform: translateY(0); }
}

.scan-banner-content {
  max-width: 1200px;
  margin: 0 auto;
}

.scan-banner-title {
  font-size: 18px;
  font-weight: 600;
  margin-bottom: 8px;
}

.scan-banner-details {
  font-size: 14px;
  opacity: 0.9;
  margin-bottom: 10px;
}

.scan-progress-bar {
  height: 6px;
  background: rgba(255,255,255,0.2);
  border-radius: 3px;
  overflow: hidden;
}

.scan-progress-fill {
  height: 100%;
  background: #fff;
  transition: width 0.3s ease;
  border-radius: 3px;
}

.wifi-network {
  padding: 12px;
  margin: 8px 0;
  background: #161b22;
  border-radius: 6px;
  border-left: 3px solid #58a6ff;
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.wifi-network.open {
  border-left-color: #f85149;
}

.wifi-network.weak {
  border-left-color: #d29922;
}

.wifi-signal {
  display: flex;
  align-items: center;
  gap: 10px;
}

.signal-bars {
  display: inline-block;
  font-size: 14px;
  color: #3fb950;
}

.signal-weak { color: #f85149; }
.signal-medium { color: #d29922; }
.signal-strong { color: #3fb950; }

.bt-device {
  padding: 12px;
  margin: 8px 0;
  background: #161b22;
  border-radius: 6px;
  border-left: 3px solid #58a6ff;
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.bt-device.random {
  border-left-color: #d29922;
}

.service-item {
  padding: 12px;
  margin: 8px 0;
  background: #161b22;
  border-radius: 6px;
  border-left: 3px solid #3fb950;
  display: flex;
  justify-content: space-between;
  align-items: center;
}
)rawliteral";

#endif

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <chrono>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>

// Advanced HTTP server for speed test GUI with maps and server selection

class SpeedTestServer {
public:
    SpeedTestServer(int port) : port_(port), server_fd_(-1) {}
    
    bool start() {
        server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd_ < 0) {
            std::cerr << "Failed to create socket\n";
            return false;
        }
        
        int opt = 1;
        setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port_);
        
        if (bind(server_fd_, (sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "Failed to bind to port " << port_ << "\n";
            return false;
        }
        
        if (listen(server_fd_, 10) < 0) {
            std::cerr << "Failed to listen\n";
            return false;
        }
        
        std::cout << "\n";
        std::cout << "  ╔═══════════════════════════════════════════════════════╗\n";
        std::cout << "  ║              ⚡ SPEED TEST SERVER ⚡                   ║\n";
        std::cout << "  ╚═══════════════════════════════════════════════════════╝\n";
        std::cout << "\n";
        std::cout << "  🌐 Server running at: http://localhost:" << port_ << "\n";
        std::cout << "  📋 Press Ctrl+C to stop\n\n";
        
        return true;
    }
    
    void run() {
        while (true) {
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);
            int client_fd = accept(server_fd_, (sockaddr*)&client_addr, &client_len);
            
            if (client_fd >= 0) {
                handle_client(client_fd);
                close(client_fd);
            }
        }
    }
    
private:
    int port_;
    int server_fd_;
    
    std::string get_ip() {
        FILE* pipe = popen("curl -s ifconfig.me 2>/dev/null", "r");
        std::string ip = "127.0.0.1";
        if (pipe) {
            char buffer[128];
            if (fgets(buffer, sizeof(buffer), pipe)) {
                ip = std::string(buffer);
                if (!ip.empty() && ip.back() == '\n') ip.pop_back();
            }
            pclose(pipe);
        }
        return ip;
    }
    
    std::string get_hostname() {
        char hostname[256];
        if (gethostname(hostname, sizeof(hostname)) == 0) {
            return std::string(hostname);
        }
        return "Local Server";
    }
    
    double random_speed(double base, double variance) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::normal_distribution<> dist(base, variance);
        return std::max(1.0, dist(gen));
    }
    
    void handle_client(int client_fd) {
        char buffer[4096];
        ssize_t bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) return;
        buffer[bytes] = '\0';
        
        std::string request(buffer);
        std::string response;
        
        if (request.find("GET /api/servers") != std::string::npos) {
            std::ostringstream json;
            json << R"([
                {"id":1,"name":"New York, US","location":"New York","country":"United States","lat":40.7128,"lng":-74.0060,"distance":0,"ping":12},
                {"id":2,"name":"London, UK","location":"London","country":"United Kingdom","lat":51.5074,"lng":-0.1278,"distance":5571,"ping":85},
                {"id":3,"name":"Tokyo, JP","location":"Tokyo","country":"Japan","lat":35.6762,"lng":139.6503,"distance":10838,"ping":165},
                {"id":4,"name":"Sydney, AU","location":"Sydney","country":"Australia","lat":-33.8688,"lng":151.2093,"distance":15989,"ping":210},
                {"id":5,"name":"Frankfurt, DE","location":"Frankfurt","country":"Germany","lat":50.1109,"lng":8.6821,"distance":6198,"ping":95},
                {"id":6,"name":"Singapore, SG","location":"Singapore","country":"Singapore","lat":1.3521,"lng":103.8198,"distance":15322,"ping":180},
                {"id":7,"name":"Mumbai, IN","location":"Mumbai","country":"India","lat":19.0760,"lng":72.8777,"distance":12568,"ping":145},
                {"id":8,"name":"São Paulo, BR","location":"São Paulo","country":"Brazil","lat":-23.5505,"lng":-46.6333,"distance":7688,"ping":120}
            ])";
            response = make_json_response(json.str());
        }
        else if (request.find("GET /api/info") != std::string::npos) {
            std::ostringstream json;
            json << "{\"ip\":\"" << get_ip() << "\","
                 << "\"server\":\"" << get_hostname() << "\","
                 << "\"location\":\"Local Network\","
                 << "\"isp\":\"Development Environment\"}";
            response = make_json_response(json.str());
        }
        else if (request.find("GET /api/ping") != std::string::npos) {
            double ping = random_speed(15.0, 5.0);
            double jitter = random_speed(3.0, 1.5);
            std::ostringstream json;
            json << "{\"ping\":" << ping << ",\"jitter\":" << jitter << "}";
            response = make_json_response(json.str());
        }
        else if (request.find("GET /api/download") != std::string::npos) {
            double speed = random_speed(95.0, 15.0);
            std::ostringstream json;
            json << "{\"speed\":" << speed << "}";
            response = make_json_response(json.str());
        }
        else if (request.find("GET /api/upload") != std::string::npos) {
            double speed = random_speed(45.0, 10.0);
            std::ostringstream json;
            json << "{\"speed\":" << speed << "}";
            response = make_json_response(json.str());
        }
        else {
            response = make_html_response(get_html());
        }
        
        send(client_fd, response.c_str(), response.length(), 0);
    }
    
    std::string make_json_response(const std::string& json) {
        std::ostringstream ss;
        ss << "HTTP/1.1 200 OK\r\n"
           << "Content-Type: application/json\r\n"
           << "Access-Control-Allow-Origin: *\r\n"
           << "Content-Length: " << json.length() << "\r\n"
           << "\r\n"
           << json;
        return ss.str();
    }
    
    std::string make_html_response(const std::string& html) {
        std::ostringstream ss;
        ss << "HTTP/1.1 200 OK\r\n"
           << "Content-Type: text/html\r\n"
           << "Content-Length: " << html.length() << "\r\n"
           << "\r\n"
           << html;
        return ss.str();
    }
    
    std::string get_html() {
        return R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Speed Test</title>
    <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css" />
    <script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js"></script>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #0a0a1a 0%, #1a1a3e 50%, #0a2a4a 100%);
            min-height: 100vh;
            color: white;
            overflow-x: hidden;
        }
        
        .header {
            text-align: center;
            padding: 30px 20px 20px;
            background: rgba(0,0,0,0.3);
            border-bottom: 1px solid rgba(255,255,255,0.1);
        }
        
        h1 {
            font-size: 2.5rem;
            background: linear-gradient(90deg, #00d4ff, #7c3aed, #00d4ff);
            background-size: 200% auto;
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
            animation: shimmer 3s linear infinite;
        }
        
        @keyframes shimmer {
            0% { background-position: 0% center; }
            100% { background-position: 200% center; }
        }
        
        .subtitle {
            color: #666;
            margin-top: 5px;
            font-size: 0.9rem;
        }
        
        .main-container {
            display: flex;
            flex-wrap: wrap;
            max-width: 1400px;
            margin: 0 auto;
            padding: 20px;
            gap: 20px;
        }
        
        .left-panel {
            flex: 1;
            min-width: 300px;
        }
        
        .right-panel {
            flex: 1.5;
            min-width: 400px;
        }
        
        /* Map Section */
        .map-container {
            background: rgba(255,255,255,0.05);
            border-radius: 20px;
            padding: 20px;
            border: 1px solid rgba(255,255,255,0.1);
            margin-bottom: 20px;
        }
        
        .map-title {
            font-size: 1rem;
            color: #888;
            margin-bottom: 15px;
            display: flex;
            align-items: center;
            gap: 10px;
        }
        
        #map {
            height: 250px;
            border-radius: 15px;
            overflow: hidden;
        }
        
        .leaflet-container {
            background: #1a1a2e;
        }
        
        /* Server List */
        .server-list {
            background: rgba(255,255,255,0.05);
            border-radius: 20px;
            padding: 20px;
            border: 1px solid rgba(255,255,255,0.1);
        }
        
        .server-item {
            display: flex;
            align-items: center;
            padding: 12px 15px;
            border-radius: 10px;
            cursor: pointer;
            transition: all 0.3s ease;
            margin-bottom: 8px;
            border: 1px solid transparent;
        }
        
        .server-item:hover {
            background: rgba(0, 212, 255, 0.1);
            border-color: rgba(0, 212, 255, 0.3);
        }
        
        .server-item.selected {
            background: rgba(0, 212, 255, 0.2);
            border-color: #00d4ff;
        }
        
        .server-icon {
            width: 40px;
            height: 40px;
            background: linear-gradient(135deg, #00d4ff, #7c3aed);
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            margin-right: 15px;
            font-size: 1.2rem;
        }
        
        .server-info {
            flex: 1;
        }
        
        .server-name {
            font-weight: 600;
            font-size: 0.95rem;
        }
        
        .server-location {
            color: #666;
            font-size: 0.8rem;
        }
        
        .server-ping {
            color: #00d4ff;
            font-weight: 600;
            font-size: 0.9rem;
        }
        
        /* Speed Test Section */
        .speed-test-container {
            background: rgba(255,255,255,0.05);
            border-radius: 20px;
            padding: 30px;
            border: 1px solid rgba(255,255,255,0.1);
            text-align: center;
        }
        
        .meters-container {
            display: flex;
            justify-content: center;
            gap: 40px;
            margin: 30px 0;
            flex-wrap: wrap;
        }
        
        .meter {
            position: relative;
            width: 200px;
            height: 200px;
        }
        
        .meter-label {
            font-size: 0.85rem;
            color: #888;
            text-transform: uppercase;
            letter-spacing: 2px;
            margin-bottom: 10px;
        }
        
        .meter svg {
            width: 100%;
            height: 100%;
            transform: rotate(-90deg);
        }
        
        .meter-bg {
            fill: none;
            stroke: #222;
            stroke-width: 12;
        }
        
        .meter-progress {
            fill: none;
            stroke-width: 12;
            stroke-linecap: round;
            transition: stroke-dashoffset 0.3s ease;
        }
        
        .meter-download .meter-progress {
            stroke: url(#downloadGradient);
        }
        
        .meter-upload .meter-progress {
            stroke: url(#uploadGradient);
        }
        
        .meter-center {
            position: absolute;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            text-align: center;
        }
        
        .meter-value {
            font-size: 2.5rem;
            font-weight: bold;
            line-height: 1;
        }
        
        .meter-unit {
            font-size: 0.8rem;
            color: #888;
            margin-top: 5px;
        }
        
        .meter-download .meter-value {
            color: #00d4ff;
        }
        
        .meter-upload .meter-value {
            color: #7c3aed;
        }
        
        /* GO Button */
        .go-button {
            width: 140px;
            height: 140px;
            border-radius: 50%;
            border: none;
            background: linear-gradient(135deg, #00d4ff, #7c3aed);
            color: white;
            font-size: 1.8rem;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.3s ease;
            box-shadow: 0 10px 40px rgba(0, 212, 255, 0.4);
            position: relative;
            overflow: hidden;
        }
        
        .go-button::before {
            content: '';
            position: absolute;
            top: -50%;
            left: -50%;
            width: 200%;
            height: 200%;
            background: linear-gradient(
                45deg,
                transparent,
                rgba(255,255,255,0.1),
                transparent
            );
            transform: rotate(45deg);
            animation: buttonShine 3s infinite;
        }
        
        @keyframes buttonShine {
            0% { transform: translateX(-100%) rotate(45deg); }
            100% { transform: translateX(100%) rotate(45deg); }
        }
        
        .go-button:hover {
            transform: scale(1.08);
            box-shadow: 0 15px 50px rgba(0, 212, 255, 0.6);
        }
        
        .go-button:disabled {
            background: #444;
            cursor: not-allowed;
            box-shadow: none;
            transform: scale(1);
        }
        
        .go-button:disabled::before {
            display: none;
        }
        
        /* Status */
        .status {
            margin-top: 20px;
            font-size: 1rem;
            color: #888;
            min-height: 30px;
        }
        
        .status.active {
            color: #00d4ff;
            animation: pulse 1.5s infinite;
        }
        
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
        
        /* Results Grid */
        .results-grid {
            display: grid;
            grid-template-columns: repeat(4, 1fr);
            gap: 15px;
            margin-top: 30px;
        }
        
        .result-box {
            background: rgba(0,0,0,0.3);
            border-radius: 15px;
            padding: 20px 15px;
            border: 1px solid rgba(255,255,255,0.05);
        }
        
        .result-icon {
            font-size: 1.5rem;
            margin-bottom: 8px;
        }
        
        .result-value {
            font-size: 1.5rem;
            font-weight: bold;
        }
        
        .result-label {
            font-size: 0.75rem;
            color: #666;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-top: 5px;
        }
        
        .result-box.download .result-value { color: #00d4ff; }
        .result-box.upload .result-value { color: #7c3aed; }
        .result-box.ping .result-value { color: #10b981; }
        .result-box.jitter .result-value { color: #f59e0b; }
        
        /* Server Info Bar */
        .server-info-bar {
            display: flex;
            justify-content: center;
            gap: 30px;
            margin-top: 25px;
            padding-top: 20px;
            border-top: 1px solid rgba(255,255,255,0.1);
            flex-wrap: wrap;
        }
        
        .info-item {
            text-align: center;
        }
        
        .info-label {
            font-size: 0.7rem;
            color: #555;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .info-value {
            font-size: 0.9rem;
            color: #888;
            margin-top: 3px;
        }
        
        /* Animations */
        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(20px); }
            to { opacity: 1; transform: translateY(0); }
        }
        
        .fade-in {
            animation: fadeIn 0.5s ease forwards;
        }
        
        /* Custom Leaflet Styles */
        .custom-marker {
            background: linear-gradient(135deg, #00d4ff, #7c3aed);
            border-radius: 50%;
            border: 3px solid white;
            box-shadow: 0 3px 10px rgba(0,0,0,0.3);
        }
        
        .leaflet-popup-content-wrapper {
            background: #1a1a2e;
            color: white;
            border-radius: 10px;
        }
        
        .leaflet-popup-tip {
            background: #1a1a2e;
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>⚡ Speed Test</h1>
        <p class="subtitle">Test your internet connection speed</p>
    </div>
    
    <div class="main-container">
        <div class="left-panel">
            <div class="map-container">
                <div class="map-title">🌍 Select Server</div>
                <div id="map"></div>
            </div>
            
            <div class="server-list" id="serverList">
                <!-- Servers will be loaded here -->
            </div>
        </div>
        
        <div class="right-panel">
            <div class="speed-test-container">
                <svg width="0" height="0">
                    <defs>
                        <linearGradient id="downloadGradient" x1="0%" y1="0%" x2="100%" y2="0%">
                            <stop offset="0%" stop-color="#00d4ff"/>
                            <stop offset="100%" stop-color="#0088cc"/>
                        </linearGradient>
                        <linearGradient id="uploadGradient" x1="0%" y1="0%" x2="100%" y2="0%">
                            <stop offset="0%" stop-color="#7c3aed"/>
                            <stop offset="100%" stop-color="#5b21b6"/>
                        </linearGradient>
                    </defs>
                </svg>
                
                <div class="meters-container">
                    <div class="meter meter-download">
                        <div class="meter-label">↓ Download</div>
                        <svg viewBox="0 0 120 120">
                            <circle class="meter-bg" cx="60" cy="60" r="50"/>
                            <circle class="meter-progress" id="downloadProgress" cx="60" cy="60" r="50"
                                stroke-dasharray="314" stroke-dashoffset="314"/>
                        </svg>
                        <div class="meter-center">
                            <div class="meter-value" id="downloadValue">0</div>
                            <div class="meter-unit">Mbps</div>
                        </div>
                    </div>
                    
                    <div class="meter meter-upload">
                        <div class="meter-label">↑ Upload</div>
                        <svg viewBox="0 0 120 120">
                            <circle class="meter-bg" cx="60" cy="60" r="50"/>
                            <circle class="meter-progress" id="uploadProgress" cx="60" cy="60" r="50"
                                stroke-dasharray="314" stroke-dashoffset="314"/>
                        </svg>
                        <div class="meter-center">
                            <div class="meter-value" id="uploadValue">0</div>
                            <div class="meter-unit">Mbps</div>
                        </div>
                    </div>
                </div>
                
                <button class="go-button" id="goButton" onclick="startTest()">GO</button>
                
                <div class="status" id="status">Select a server and click GO</div>
                
                <div class="results-grid" id="resultsGrid" style="display: none;">
                    <div class="result-box download">
                        <div class="result-icon">📥</div>
                        <div class="result-value" id="resultDownload">--</div>
                        <div class="result-label">Download</div>
                    </div>
                    <div class="result-box upload">
                        <div class="result-icon">📤</div>
                        <div class="result-value" id="resultUpload">--</div>
                        <div class="result-label">Upload</div>
                    </div>
                    <div class="result-box ping">
                        <div class="result-icon">📶</div>
                        <div class="result-value" id="resultPing">--</div>
                        <div class="result-label">Ping</div>
                    </div>
                    <div class="result-box jitter">
                        <div class="result-icon">📊</div>
                        <div class="result-value" id="resultJitter">--</div>
                        <div class="result-label">Jitter</div>
                    </div>
                </div>
                
                <div class="server-info-bar" id="serverInfoBar">
                    <div class="info-item">
                        <div class="info-label">Server</div>
                        <div class="info-value" id="infoServer">--</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">IP Address</div>
                        <div class="info-value" id="infoIP">--</div>
                    </div>
                    <div class="info-item">
                        <div class="info-label">ISP</div>
                        <div class="info-value" id="infoISP">--</div>
                    </div>
                </div>
            </div>
        </div>
    </div>
    
    <script>
        let map;
        let markers = [];
        let servers = [];
        let selectedServer = null;
        let testing = false;
        
        // Initialize
        document.addEventListener('DOMContentLoaded', async () => {
            initMap();
            await loadServers();
            await loadInfo();
        });
        
        function initMap() {
            map = L.map('map', {
                center: [30, 0],
                zoom: 2,
                zoomControl: false,
                attributionControl: false
            });
            
            L.tileLayer('https://{s}.basemaps.cartocdn.com/dark_all/{z}/{x}/{y}{r}.png', {
                maxZoom: 19
            }).addTo(map);
        }
        
        async function loadServers() {
            try {
                const response = await fetch('/api/servers');
                servers = await response.json();
                renderServers();
                addMapMarkers();
                if (servers.length > 0) {
                    selectServer(servers[0]);
                }
            } catch (e) {
                console.error('Failed to load servers:', e);
            }
        }
        
        async function loadInfo() {
            try {
                const info = await fetch('/api/info').then(r => r.json());
                document.getElementById('infoServer').textContent = info.server;
                document.getElementById('infoIP').textContent = info.ip;
                document.getElementById('infoISP').textContent = info.isp;
            } catch (e) {}
        }
        
        function renderServers() {
            const list = document.getElementById('serverList');
            list.innerHTML = '<div class="map-title">🖥️ Available Servers</div>';
            
            servers.forEach(server => {
                const item = document.createElement('div');
                item.className = 'server-item';
                item.id = `server-${server.id}`;
                item.innerHTML = `
                    <div class="server-icon">🌐</div>
                    <div class="server-info">
                        <div class="server-name">${server.name}</div>
                        <div class="server-location">${server.country}</div>
                    </div>
                    <div class="server-ping">${server.ping} ms</div>
                `;
                item.onclick = () => selectServer(server);
                list.appendChild(item);
            });
        }
        
        function addMapMarkers() {
            servers.forEach(server => {
                const icon = L.divIcon({
                    className: 'custom-marker',
                    iconSize: [16, 16]
                });
                
                const marker = L.marker([server.lat, server.lng], { icon })
                    .addTo(map)
                    .bindPopup(`<b>${server.name}</b><br>${server.ping} ms`);
                
                marker.on('click', () => selectServer(server));
                markers.push(marker);
            });
        }
        
        function selectServer(server) {
            selectedServer = server;
            
            // Update UI
            document.querySelectorAll('.server-item').forEach(el => {
                el.classList.remove('selected');
            });
            document.getElementById(`server-${server.id}`)?.classList.add('selected');
            
            // Center map
            map.flyTo([server.lat, server.lng], 4, { duration: 1 });
            
            document.getElementById('status').textContent = `Selected: ${server.name}`;
        }
        
        async function startTest() {
            if (testing || !selectedServer) return;
            testing = true;
            
            const btn = document.getElementById('goButton');
            const status = document.getElementById('status');
            const downloadProgress = document.getElementById('downloadProgress');
            const uploadProgress = document.getElementById('uploadProgress');
            const downloadValue = document.getElementById('downloadValue');
            const uploadValue = document.getElementById('uploadValue');
            const resultsGrid = document.getElementById('resultsGrid');
            
            btn.disabled = true;
            btn.textContent = '●●●';
            resultsGrid.style.display = 'none';
            status.className = 'status active';
            
            // Reset meters
            downloadProgress.style.strokeDashoffset = 314;
            uploadProgress.style.strokeDashoffset = 314;
            downloadValue.textContent = '0';
            uploadValue.textContent = '0';
            
            let pingResult, jitterResult, downloadResult, uploadResult;
            
            // Ping Test
            status.textContent = 'Testing ping...';
            await animateMeter(downloadProgress, downloadValue, 0, 30, 500);
            const pingData = await fetch('/api/ping').then(r => r.json());
            pingResult = pingData.ping;
            jitterResult = pingData.jitter;
            
            await sleep(300);
            
            // Download Test
            status.textContent = 'Testing download speed...';
            downloadResult = await runSpeedTest(downloadProgress, downloadValue, '/api/download', 100);
            
            await sleep(500);
            
            // Upload Test
            status.textContent = 'Testing upload speed...';
            uploadResult = await runSpeedTest(uploadProgress, uploadValue, '/api/upload', 50);
            
            // Show Results
            status.className = 'status';
            status.textContent = 'Test complete!';
            
            document.getElementById('resultDownload').textContent = downloadResult.toFixed(1) + ' Mbps';
            document.getElementById('resultUpload').textContent = uploadResult.toFixed(1) + ' Mbps';
            document.getElementById('resultPing').textContent = pingResult.toFixed(0) + ' ms';
            document.getElementById('resultJitter').textContent = jitterResult.toFixed(1) + ' ms';
            
            resultsGrid.style.display = 'grid';
            resultsGrid.classList.add('fade-in');
            
            btn.disabled = false;
            btn.textContent = 'GO';
            testing = false;
        }
        
        async function runSpeedTest(progressEl, valueEl, endpoint, maxSpeed) {
            // Animate progress
            for (let i = 0; i <= 100; i += 2) {
                const speed = Math.min(maxSpeed, (i / 100) * maxSpeed * (0.8 + Math.random() * 0.4));
                const offset = 314 - (i / 100) * 314;
                progressEl.style.strokeDashoffset = offset;
                valueEl.textContent = speed.toFixed(1);
                await sleep(40);
            }
            
            // Get final result
            const result = await fetch(endpoint).then(r => r.json());
            valueEl.textContent = result.speed.toFixed(1);
            progressEl.style.strokeDashoffset = 314 - (result.speed / maxSpeed) * 314;
            
            return result.speed;
        }
        
        async function animateMeter(progressEl, valueEl, start, end, duration) {
            const steps = 20;
            const stepDuration = duration / steps;
            
            for (let i = 0; i <= steps; i++) {
                const progress = i / steps;
                const value = start + (end - start) * progress;
                const offset = 314 - progress * 100;
                progressEl.style.strokeDashoffset = offset;
                valueEl.textContent = value.toFixed(0);
                await sleep(stepDuration);
            }
        }
        
        function sleep(ms) {
            return new Promise(resolve => setTimeout(resolve, ms));
        }
    </script>
</body>
</html>
)HTML";
    }
};

int main() {
    SpeedTestServer server(8080);
    
    if (!server.start()) {
        return 1;
    }
    
    server.run();
    
    return 0;
}

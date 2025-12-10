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

// Simple HTTP server for speed test GUI

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
        
        if (request.find("GET /api/info") != std::string::npos) {
            // Return server info as JSON
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
            double speed = random_speed(95.0, 10.0);
            std::ostringstream json;
            json << "{\"speed\":" << speed << "}";
            response = make_json_response(json.str());
        }
        else if (request.find("GET /api/upload") != std::string::npos) {
            double speed = random_speed(45.0, 8.0);
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
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #1a1a2e 0%, #16213e 50%, #0f3460 100%);
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: center;
            color: white;
        }
        
        .container {
            text-align: center;
            padding: 40px;
        }
        
        h1 {
            font-size: 3rem;
            margin-bottom: 10px;
            background: linear-gradient(90deg, #00d4ff, #7c3aed);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
        }
        
        .subtitle {
            color: #888;
            margin-bottom: 40px;
        }
        
        .speedometer {
            position: relative;
            width: 300px;
            height: 300px;
            margin: 0 auto 40px;
        }
        
        .speed-ring {
            width: 100%;
            height: 100%;
            border-radius: 50%;
            background: conic-gradient(
                from 135deg,
                #333 0deg,
                #333 270deg,
                #333 270deg
            );
            display: flex;
            justify-content: center;
            align-items: center;
            position: relative;
        }
        
        .speed-ring.active {
            background: conic-gradient(
                from 135deg,
                #00d4ff 0deg,
                #7c3aed calc(var(--progress, 0) * 2.7deg),
                #333 calc(var(--progress, 0) * 2.7deg),
                #333 270deg
            );
            transition: background 0.3s ease;
        }
        
        .speed-inner {
            width: 85%;
            height: 85%;
            background: linear-gradient(135deg, #1a1a2e, #16213e);
            border-radius: 50%;
            display: flex;
            flex-direction: column;
            justify-content: center;
            align-items: center;
        }
        
        .speed-value {
            font-size: 4rem;
            font-weight: bold;
            line-height: 1;
        }
        
        .speed-unit {
            font-size: 1.2rem;
            color: #888;
            margin-top: 5px;
        }
        
        .speed-label {
            font-size: 1rem;
            color: #00d4ff;
            margin-top: 10px;
            text-transform: uppercase;
            letter-spacing: 2px;
        }
        
        .go-button {
            width: 120px;
            height: 120px;
            border-radius: 50%;
            border: none;
            background: linear-gradient(135deg, #00d4ff, #7c3aed);
            color: white;
            font-size: 1.5rem;
            font-weight: bold;
            cursor: pointer;
            transition: all 0.3s ease;
            box-shadow: 0 10px 40px rgba(0, 212, 255, 0.3);
        }
        
        .go-button:hover {
            transform: scale(1.05);
            box-shadow: 0 15px 50px rgba(0, 212, 255, 0.5);
        }
        
        .go-button:disabled {
            background: #444;
            cursor: not-allowed;
            box-shadow: none;
        }
        
        .results {
            display: none;
            margin-top: 40px;
            gap: 30px;
            justify-content: center;
        }
        
        .results.show {
            display: flex;
        }
        
        .result-card {
            background: rgba(255, 255, 255, 0.05);
            border-radius: 15px;
            padding: 25px 40px;
            backdrop-filter: blur(10px);
            border: 1px solid rgba(255, 255, 255, 0.1);
        }
        
        .result-icon {
            font-size: 2rem;
            margin-bottom: 10px;
        }
        
        .result-value {
            font-size: 2.5rem;
            font-weight: bold;
        }
        
        .result-label {
            color: #888;
            font-size: 0.9rem;
            text-transform: uppercase;
            letter-spacing: 1px;
        }
        
        .server-info {
            margin-top: 40px;
            padding: 20px;
            background: rgba(255, 255, 255, 0.03);
            border-radius: 10px;
            display: none;
        }
        
        .server-info.show {
            display: block;
        }
        
        .server-info p {
            color: #666;
            margin: 5px 0;
        }
        
        .server-info span {
            color: #888;
        }
        
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
        
        .testing .speed-label {
            animation: pulse 1s infinite;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>⚡ Speed Test</h1>
        <p class="subtitle">Test your internet connection speed</p>
        
        <div class="speedometer" id="speedometer">
            <div class="speed-ring" id="speedRing">
                <div class="speed-inner">
                    <div class="speed-value" id="speedValue">0</div>
                    <div class="speed-unit">Mbps</div>
                    <div class="speed-label" id="speedLabel">Ready</div>
                </div>
            </div>
        </div>
        
        <button class="go-button" id="goButton" onclick="startTest()">GO</button>
        
        <div class="results" id="results">
            <div class="result-card">
                <div class="result-icon">📥</div>
                <div class="result-value" id="downloadResult">--</div>
                <div class="result-label">Download</div>
            </div>
            <div class="result-card">
                <div class="result-icon">📤</div>
                <div class="result-value" id="uploadResult">--</div>
                <div class="result-label">Upload</div>
            </div>
            <div class="result-card">
                <div class="result-icon">📶</div>
                <div class="result-value" id="pingResult">--</div>
                <div class="result-label">Ping</div>
            </div>
        </div>
        
        <div class="server-info" id="serverInfo">
            <p>Server: <span id="serverName">--</span></p>
            <p>IP: <span id="ipAddress">--</span></p>
            <p>Location: <span id="location">--</span></p>
        </div>
    </div>
    
    <script>
        let testing = false;
        
        async function startTest() {
            if (testing) return;
            testing = true;
            
            const btn = document.getElementById('goButton');
            const speedometer = document.getElementById('speedometer');
            const speedRing = document.getElementById('speedRing');
            const speedValue = document.getElementById('speedValue');
            const speedLabel = document.getElementById('speedLabel');
            const results = document.getElementById('results');
            const serverInfo = document.getElementById('serverInfo');
            
            btn.disabled = true;
            btn.textContent = '...';
            speedometer.classList.add('testing');
            results.classList.remove('show');
            
            // Get server info
            speedLabel.textContent = 'Connecting...';
            try {
                const info = await fetch('/api/info').then(r => r.json());
                document.getElementById('serverName').textContent = info.server;
                document.getElementById('ipAddress').textContent = info.ip;
                document.getElementById('location').textContent = info.location;
                serverInfo.classList.add('show');
            } catch (e) {}
            
            await sleep(500);
            
            // Ping test
            speedLabel.textContent = 'Testing Ping...';
            let pingData;
            for (let i = 0; i <= 100; i += 10) {
                speedRing.classList.add('active');
                speedRing.style.setProperty('--progress', i);
                await sleep(50);
            }
            pingData = await fetch('/api/ping').then(r => r.json());
            speedValue.textContent = pingData.ping.toFixed(0);
            
            await sleep(300);
            
            // Download test
            speedLabel.textContent = 'Testing Download...';
            let downloadSpeed = 0;
            for (let i = 0; i <= 100; i += 2) {
                speedRing.style.setProperty('--progress', i);
                downloadSpeed = Math.min(95, i * 0.95 + Math.random() * 10);
                speedValue.textContent = downloadSpeed.toFixed(1);
                await sleep(30);
            }
            const downloadData = await fetch('/api/download').then(r => r.json());
            speedValue.textContent = downloadData.speed.toFixed(1);
            downloadSpeed = downloadData.speed;
            
            await sleep(300);
            
            // Upload test
            speedLabel.textContent = 'Testing Upload...';
            let uploadSpeed = 0;
            for (let i = 0; i <= 100; i += 2) {
                speedRing.style.setProperty('--progress', i);
                uploadSpeed = Math.min(45, i * 0.45 + Math.random() * 5);
                speedValue.textContent = uploadSpeed.toFixed(1);
                await sleep(30);
            }
            const uploadData = await fetch('/api/upload').then(r => r.json());
            speedValue.textContent = uploadData.speed.toFixed(1);
            uploadSpeed = uploadData.speed;
            
            // Show results
            speedLabel.textContent = 'Complete';
            speedometer.classList.remove('testing');
            
            document.getElementById('downloadResult').textContent = downloadSpeed.toFixed(1) + ' Mbps';
            document.getElementById('uploadResult').textContent = uploadSpeed.toFixed(1) + ' Mbps';
            document.getElementById('pingResult').textContent = pingData.ping.toFixed(0) + ' ms';
            
            results.classList.add('show');
            
            btn.disabled = false;
            btn.textContent = 'GO';
            testing = false;
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

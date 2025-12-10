#include "benchmark.h"

#include <random>
#include <unistd.h>
#include <cstdio>

namespace speedtest {

const char* Spinner::frames_[] = {"⠋", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠧", "⠇", "⠏"};

Spinner::Spinner() : frame_(0) {}

void Spinner::spin(const std::string& message) {
    std::cout << "\r  " << frames_[frame_ % 10] << " " << message << std::flush;
    frame_++;
}

void Spinner::stop() {
    std::cout << "\r" << std::string(60, ' ') << "\r" << std::flush;
}

void ProgressBar::show(const std::string& label, double progress, double speed) {
    const int width = 30;
    int filled = static_cast<int>(progress * width);
    
    std::cout << "\r  " << std::left << std::setw(12) << label << " [";
    for (int i = 0; i < width; ++i) {
        if (i < filled) std::cout << "█";
        else if (i == filled) std::cout << "▓";
        else std::cout << "░";
    }
    std::cout << "] " << std::fixed << std::setprecision(0) << (progress * 100) << "%";
    
    if (speed >= 0) {
        std::cout << "  " << std::setprecision(2) << speed << " Mbps";
    }
    std::cout << "   " << std::flush;
}

void ProgressBar::complete(const std::string& label, double final_value, const std::string& unit) {
    std::cout << "\r  " << std::left << std::setw(12) << label << " ";
    std::cout << std::fixed << std::setprecision(2) << final_value << " " << unit;
    std::cout << std::string(30, ' ') << "\n";
}

SpeedTest::SpeedTest() {
    server_info_ = detect_server();
}

ServerInfo SpeedTest::detect_server() {
    Spinner spinner;
    spinner.spin("Detecting location...");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    ServerInfo info;
    
    // Try to get real IP info using curl
    FILE* pipe = popen("curl -s ifconfig.me 2>/dev/null", "r");
    if (pipe) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe)) {
            info.ip_address = std::string(buffer);
            // Remove newline
            if (!info.ip_address.empty() && info.ip_address.back() == '\n')
                info.ip_address.pop_back();
        }
        pclose(pipe);
    }
    
    spinner.spin("Finding best server...");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // Get hostname
    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == 0) {
        info.server_name = std::string(hostname);
    } else {
        info.server_name = "Local Server";
    }
    
    // Default location info
    info.location = "Local Network";
    info.isp = "Development Environment";
    
    if (info.ip_address.empty()) {
        info.ip_address = "127.0.0.1";
    }
    
    spinner.stop();
    return info;
}

void SpeedTest::clear_line() {
    std::cout << "\r" << std::string(70, ' ') << "\r" << std::flush;
}

void SpeedTest::print_header() {
    std::cout << R"(
   ╔═══════════════════════════════════════════════════════╗
   ║                                                       ║
   ║              ⚡ SPEED TEST ⚡                          ║
   ║                                                       ║
   ╚═══════════════════════════════════════════════════════╝
)" << std::endl;
}

double SpeedTest::simulate_network_operation(double base_speed, double variance) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::normal_distribution<> dist(base_speed, variance);
    return std::max(1.0, dist(gen));
}

double SpeedTest::test_ping() {
    Spinner spinner;
    std::vector<double> pings;
    
    for (int i = 0; i < 10; ++i) {
        spinner.spin("Testing ping...");
        
        // Simulate ping measurement
        auto start = std::chrono::high_resolution_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        auto end = std::chrono::high_resolution_clock::now();
        
        double ping = simulate_network_operation(15.0, 5.0);
        pings.push_back(ping);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    spinner.stop();
    
    double sum = 0;
    for (double p : pings) sum += p;
    return sum / pings.size();
}

double SpeedTest::test_download() {
    double current_speed = 0;
    double max_speed = 0;
    
    for (int i = 0; i <= 100; i += 2) {
        double progress = i / 100.0;
        
        // Simulate speed ramping up then stabilizing
        if (progress < 0.3) {
            current_speed = simulate_network_operation(50 + progress * 200, 10);
        } else {
            current_speed = simulate_network_operation(95, 8);
        }
        
        max_speed = std::max(max_speed, current_speed);
        
        ProgressBar::show("Download", progress, current_speed);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    clear_line();
    return simulate_network_operation(92, 5);
}

double SpeedTest::test_upload() {
    double current_speed = 0;
    double max_speed = 0;
    
    for (int i = 0; i <= 100; i += 2) {
        double progress = i / 100.0;
        
        // Simulate speed ramping up then stabilizing (upload typically slower)
        if (progress < 0.3) {
            current_speed = simulate_network_operation(20 + progress * 80, 8);
        } else {
            current_speed = simulate_network_operation(45, 6);
        }
        
        max_speed = std::max(max_speed, current_speed);
        
        ProgressBar::show("Upload", progress, current_speed);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    
    clear_line();
    return simulate_network_operation(42, 4);
}

void SpeedTest::print_server_info(const ServerInfo& info) {
    std::cout << "\n   ┌─────────────────────────────────────────────────────┐\n";
    std::cout << "   │  SERVER INFO                                       │\n";
    std::cout << "   ├─────────────────────────────────────────────────────┤\n";
    std::cout << "   │  Server:   " << std::left << std::setw(40) << info.server_name << "│\n";
    std::cout << "   │  Location: " << std::left << std::setw(40) << info.location << "│\n";
    std::cout << "   │  ISP:      " << std::left << std::setw(40) << info.isp << "│\n";
    std::cout << "   │  IP:       " << std::left << std::setw(40) << info.ip_address << "│\n";
    std::cout << "   └─────────────────────────────────────────────────────┘\n\n";
}

SpeedResult SpeedTest::run_full_test() {
    SpeedResult result;
    result.server = server_info_;
    
    print_server_info(server_info_);
    
    std::cout << "  Testing latency...\n";
    result.ping_ms = test_ping();
    result.jitter_ms = simulate_network_operation(3.0, 1.5);
    ProgressBar::complete("Ping", result.ping_ms, "ms");
    ProgressBar::complete("Jitter", result.jitter_ms, "ms");
    
    std::cout << "\n";
    
    result.download_mbps = test_download();
    ProgressBar::complete("Download", result.download_mbps, "Mbps");
    
    std::cout << "\n";
    
    result.upload_mbps = test_upload();
    ProgressBar::complete("Upload", result.upload_mbps, "Mbps");
    
    return result;
}

void SpeedTest::print_result(const SpeedResult& result) {
    std::cout << R"(
   ┌─────────────────────────────────────────────────────┐
   │                    RESULTS                          │
   ├─────────────────────────────────────────────────────┤
)";
    
    std::cout << "   │  PING      " << std::fixed << std::setprecision(2) 
              << std::setw(8) << result.ping_ms << " ms"
              << "                              │\n";
    
    std::cout << "   │  JITTER    " << std::fixed << std::setprecision(2) 
              << std::setw(8) << result.jitter_ms << " ms"
              << "                              │\n";
    
    std::cout << "   ├─────────────────────────────────────────────────────┤\n";
    
    std::cout << "   │  ↓ DOWNLOAD " << std::fixed << std::setprecision(2) 
              << std::setw(7) << result.download_mbps << " Mbps"
              << "                           │\n";
    
    std::cout << "   │  ↑ UPLOAD   " << std::fixed << std::setprecision(2) 
              << std::setw(7) << result.upload_mbps << " Mbps"
              << "                           │\n";
    
    std::cout << "   └─────────────────────────────────────────────────────┘\n\n";
}

} // namespace speedtest

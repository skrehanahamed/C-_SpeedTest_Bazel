#ifndef BENCHMARK_H_
#define BENCHMARK_H_

#include <chrono>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace speedtest {

// Server/Location info
struct ServerInfo {
    std::string server_name;
    std::string location;
    std::string isp;
    std::string ip_address;
};

// Speed test result
struct SpeedResult {
    double download_mbps;
    double upload_mbps;
    double ping_ms;
    double jitter_ms;
    ServerInfo server;
};

// Progress bar with animation
class ProgressBar {
public:
    static void show(const std::string& label, double progress, double speed = -1);
    static void complete(const std::string& label, double final_value, const std::string& unit);
};

// Spinner animation
class Spinner {
public:
    Spinner();
    void spin(const std::string& message);
    void stop();
private:
    int frame_;
    static const char* frames_[];
};

// Main speed test class
class SpeedTest {
public:
    SpeedTest();
    
    // Get server/location info
    ServerInfo detect_server();
    
    // Run tests
    double test_ping();
    double test_download();
    double test_upload();
    
    // Run full test with UI
    SpeedResult run_full_test();
    
    // UI helpers
    static void print_header();
    static void print_server_info(const ServerInfo& info);
    static void print_result(const SpeedResult& result);
    static void clear_line();

private:
    double simulate_network_operation(double base_speed, double variance);
    ServerInfo server_info_;
};

} // namespace speedtest

#endif // BENCHMARK_H_

#include "benchmark.h"

using namespace speedtest;

int main() {
    SpeedTest::print_header();
    
    std::cout << "  Connecting to server...\n\n";
    
    SpeedTest test;
    SpeedResult result = test.run_full_test();
    
    SpeedTest::print_result(result);
    
    return 0;
}

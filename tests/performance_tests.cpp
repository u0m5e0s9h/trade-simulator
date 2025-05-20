#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include "orderbook.h"
#include "models.h"

// Simulate realistic load by feeding synthetic orderbook updates and measuring performance and memory usage
int main() {
    std::cout << "Starting performance and memory usage test..." << std::endl;

    OrderBook orderbook;
    Models models;

    const int num_updates = 100000;
    const int batch_size = 1000;

    // Synthetic orderbook update simulation
    for (int batch = 0; batch < num_updates / batch_size; ++batch) {
        auto batch_start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < batch_size; ++i) {
            // Simulate an orderbook update JSON string (simplified)
            // In real test, use realistic JSON or parsed data
            // Here we just simulate update calls
            orderbook.simulate_update();
        }

        auto batch_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> batch_duration = batch_end - batch_start;

        std::cout << "Batch " << batch + 1 << "/" << (num_updates / batch_size)
                  << " processed in " << batch_duration.count() << " ms" << std::endl;

        // Sleep briefly to simulate real-time pacing
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // Memory usage measurement would require platform-specific code or external tools
    // Here we just print a placeholder
    std::cout << "Memory usage measurement not implemented - please use external tools." << std::endl;

    std::cout << "Performance and memory usage test completed." << std::endl;
    return 0;
}

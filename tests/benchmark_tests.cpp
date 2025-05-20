#include <iostream>
#include <chrono>
#include <thread>
#include "../src/models.h"  // Adjust path if needed

// Benchmark macros with unique IDs to avoid redefinition
#define BENCHMARK_START(id) auto bench_start_##id = std::chrono::high_resolution_clock::now();
#define BENCHMARK_END(id, msg) \
    { \
        auto bench_end_##id = std::chrono::high_resolution_clock::now(); \
        std::chrono::duration<double, std::milli> elapsed_##id = bench_end_##id - bench_start_##id; \
        std::cout << msg << ": " << elapsed_##id.count() << " ms" << std::endl; \
    }

// Benchmark regression model efficiency
void benchmark_regression_model(Models& models, int iterations) {
    double quantity = 100.0;
    double volatility = 0.05;

    // Optimized version benchmark
    BENCHMARK_START(opt)
    for (int i = 0; i < iterations; ++i) {
        volatile double slippage = models.calculate_slippage_optimized(quantity, volatility);
        (void)slippage;
    }
    BENCHMARK_END(opt, "Regression model optimized slippage calculation")

    // Baseline version benchmark
    BENCHMARK_START(base)
    for (int i = 0; i < iterations; ++i) {
        volatile double slippage = models.calculate_slippage(quantity, volatility);
        (void)slippage;
    }
    BENCHMARK_END(base, "Regression model baseline slippage calculation")
}

// Main benchmark runner
int main() {
    Models models;

    std::cout << "Starting benchmark tests..." << std::endl;

    benchmark_regression_model(models, 1000000);

    std::cout << "Benchmark tests completed." << std::endl;
    return 0;
}

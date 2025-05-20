#include <iostream>
#include <vector>
#include <cmath>
#include "models.h"

// Simple model validation test with simulated data
int main() {
    std::cout << "Starting model validation tests..." << std::endl;

    Models models;

    struct TestCase {
        double quantity;
        double volatility;
        double expected_slippage;
        double expected_fees;
        double expected_market_impact;
    };

    // Example test cases with expected approximate values (for demonstration)
    std::vector<TestCase> test_cases = {
        {100.0, 0.05, 0.01, 0.001, 0.005},
        {200.0, 0.10, 0.02, 0.002, 0.010},
        {50.0, 0.02, 0.005, 0.0005, 0.0025}
    };

    for (const auto& tc : test_cases) {
        double slippage = models.calculate_slippage(tc.quantity, tc.volatility);
        double fees = models.calculate_fees(tc.quantity, 1);
        double market_impact = models.calculate_market_impact(tc.quantity, tc.volatility);

        std::cout << "Test case: Quantity=" << tc.quantity << ", Volatility=" << tc.volatility << std::endl;
        std::cout << "Calculated Slippage: " << slippage << ", Expected approx: " << tc.expected_slippage << std::endl;
        std::cout << "Calculated Fees: " << fees << ", Expected approx: " << tc.expected_fees << std::endl;
        std::cout << "Calculated Market Impact: " << market_impact << ", Expected approx: " << tc.expected_market_impact << std::endl;

        // Simple validation: check if values are within 50% of expected (for demo)
        if (std::abs(slippage - tc.expected_slippage) / tc.expected_slippage > 0.5) {
            std::cerr << "Warning: Slippage out of expected range." << std::endl;
        }
        if (std::abs(fees - tc.expected_fees) / tc.expected_fees > 0.5) {
            std::cerr << "Warning: Fees out of expected range." << std::endl;
        }
        if (std::abs(market_impact - tc.expected_market_impact) / tc.expected_market_impact > 0.5) {
            std::cerr << "Warning: Market Impact out of expected range." << std::endl;
        }
    }

    std::cout << "Model validation tests completed." << std::endl;
    return 0;
}

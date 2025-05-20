#include "models.h"
#include <cmath>
#include <iostream>
#include <mutex>
#include <vector>
#include <thread>

Models::Models() {
    // Constructor implementation (if needed)
}

Models::~Models() {
    // Destructor implementation (if needed)
}

// Almgren-Chriss market impact model implementation
double Models::calculate_market_impact(double quantity, double volatility) {
    // Simplified Almgren-Chriss model parameters
    const double gamma = 0.1;  // permanent impact coefficient
    const double eta = 0.05;   // temporary impact coefficient
    const double T = 1.0;      // trading horizon

    double impact = gamma * quantity + eta * quantity / T;
    return impact;
}

// Regression model for slippage estimation
double Models::calculate_slippage(double quantity, double volatility) {
    // Example regression coefficients (to be calibrated)
    const double intercept = 0.001;
    const double coef_quantity = 0.00001;
    const double coef_volatility = 0.05;

    double slippage = intercept + coef_quantity * quantity + coef_volatility * volatility;
    return slippage;
}

// Logistic regression for maker/taker proportion prediction
double Models::predict_maker_taker_proportion(double quantity, double volatility) {
    // Example logistic regression coefficients
    const double intercept = -1.0;
    const double coef_quantity = 0.0001;
    const double coef_volatility = 0.5;

    double linear_combination = intercept + coef_quantity * quantity + coef_volatility * volatility;
    double odds = std::exp(linear_combination);
    double probability = odds / (1.0 + odds);
    return probability;
}

// Calculate fees based on quantity and fee tier
double Models::calculate_fees(double quantity, int fee_tier) {
    // Example fee tiers: 1=0.1%, 2=0.05%, 3=0.02%
    double fee_rate = 0.001; // default 0.1%
    if (fee_tier == 2) fee_rate = 0.0005;
    else if (fee_tier == 3) fee_rate = 0.0002;

    return quantity * fee_rate;
}

// Calculate net cost combining slippage, fees, and market impact
double Models::calculate_net_cost(double quantity, double volatility, int fee_tier) {
    double slippage = calculate_slippage(quantity, volatility);
    double fees = calculate_fees(quantity, fee_tier);
    double market_impact = calculate_market_impact(quantity, volatility);

    return slippage + fees + market_impact;
}

// Bonus: Thread-safe caching for regression coefficients to improve efficiency
class RegressionCache {
private:
    std::mutex cache_mutex;
    std::vector<double> cached_coefficients;
public:
    RegressionCache() : cached_coefficients({0.001, 0.00001, 0.05}) {}

    std::vector<double> get_coefficients() {
        std::lock_guard<std::mutex> lock(cache_mutex);
        return cached_coefficients;
    }

    void update_coefficients(const std::vector<double>& new_coeffs) {
        std::lock_guard<std::mutex> lock(cache_mutex);
        cached_coefficients = new_coeffs;
    }
};

static RegressionCache regression_cache;

// Optimized slippage calculation using cached coefficients
double Models::calculate_slippage_optimized(double quantity, double volatility) {
    auto coeffs = regression_cache.get_coefficients();
    double intercept = coeffs[0];
    double coef_quantity = coeffs[1];
    double coef_volatility = coeffs[2];

    double slippage = intercept + coef_quantity * quantity + coef_volatility * volatility;
    return slippage;
}

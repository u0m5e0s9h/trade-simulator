#include "orderbook.h"
#include <algorithm>
#include <random>
#include <chrono>

OrderBook::OrderBook() {
    // Initialize empty orderbook
    asks_.clear();
    bids_.clear();
}

void OrderBook::update_from_json(const nlohmann::json& j) {
    std::lock_guard<std::mutex> lock(mutex_);
    asks_.clear();
    bids_.clear();

    if (j.contains("asks") && j["asks"].is_array()) {
        for (const auto& level : j["asks"]) {
            if (level.is_array() && level.size() >= 2) {
                OrderLevel ol;
                ol.price = std::stod(level[0].get<std::string>());
                ol.quantity = std::stod(level[1].get<std::string>());
                asks_.push_back(ol);
            }
        }
    }

    if (j.contains("bids") && j["bids"].is_array()) {
        for (const auto& level : j["bids"]) {
            if (level.is_array() && level.size() >= 2) {
                OrderLevel ol;
                ol.price = std::stod(level[0].get<std::string>());
                ol.quantity = std::stod(level[1].get<std::string>());
                bids_.push_back(ol);
            }
        }
    }

    // Sort asks ascending by price
    std::sort(asks_.begin(), asks_.end(), [](const OrderLevel& a, const OrderLevel& b) {
        return a.price < b.price;
    });

    // Sort bids descending by price
    std::sort(bids_.begin(), bids_.end(), [](const OrderLevel& a, const OrderLevel& b) {
        return a.price > b.price;
    });
}

std::vector<OrderLevel> OrderBook::get_asks() {
    std::lock_guard<std::mutex> lock(mutex_);
    return asks_;
}

std::vector<OrderLevel> OrderBook::get_bids() {
    std::lock_guard<std::mutex> lock(mutex_);
    return bids_;
}

void OrderBook::simulate_update() {
    std::lock_guard<std::mutex> lock(mutex_);

    // Generate synthetic asks and bids with random prices and quantities
    asks_.clear();
    bids_.clear();

    std::default_random_engine eng(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count()));
    std::uniform_real_distribution<double> price_dist(95000.0, 96000.0);
    std::uniform_real_distribution<double> quantity_dist(0.01, 10.0);

    // Generate 10 ask levels
    for (int i = 0; i < 10; ++i) {
        OrderLevel ol;
        ol.price = price_dist(eng) + i * 0.5; // ascending prices
        ol.quantity = quantity_dist(eng);
        asks_.push_back(ol);
    }

    // Generate 10 bid levels
    for (int i = 0; i < 10; ++i) {
        OrderLevel ol;
        ol.price = price_dist(eng) - i * 0.5; // descending prices
        ol.quantity = quantity_dist(eng);
        bids_.push_back(ol);
    }
}

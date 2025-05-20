#pragma once

#include <vector>
#include <string>
#include <mutex>
#include <nlohmann/json.hpp>

struct OrderLevel {
    double price;
    double quantity;
};

class OrderBook {
public:
    OrderBook();

    void update_from_json(const nlohmann::json& j);

    std::vector<OrderLevel> get_asks();
    std::vector<OrderLevel> get_bids();

    // For performance testing: simulate synthetic orderbook update
    void simulate_update();

private:
    std::vector<OrderLevel> asks_;
    std::vector<OrderLevel> bids_;
    std::mutex mutex_;
};

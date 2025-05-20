#pragma once

#include "orderbook.h"
#include "models.h"
#include <vector>
#include <string>
#include <chrono>

class UI {
public:
    UI(OrderBook& orderbook, Models& models);
    ~UI();

    void run();

private:
    OrderBook& orderbook_;
    Models& models_;

    int fee_tier_;
    double quantity_;
    double volatility_;

    // For dynamic spot asset selection
    std::vector<std::string> spot_assets_;
    int spot_asset_index_;

    // For internal latency measurement
    std::chrono::steady_clock::time_point last_tick_time_;
    double internal_latency_ms_;

    // For UI update latency measurement
    double ui_update_latency_ms_;

    void record_tick_time();

    void render();
    void render_input_panel();
    void render_output_panel();

};

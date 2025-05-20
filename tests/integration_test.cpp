#include <iostream>
#include <thread>
#include <chrono>
#include "websocket_client.h"
#include "orderbook.h"
#include "models.h"

// Simple integration test for real-time data flow with end-to-end latency benchmarking
int main() {
    std::cout << "Starting integration test for trade simulator..." << std::endl;

    OrderBook orderbook;
    Models models;

    // WebSocket endpoint for OKX BTC-USDT-SWAP
    std::string ws_uri = "wss://ws.gomarket-cpp.goquant.io/ws/l2-orderbook/okx/BTC-USDT-SWAP";

    WebSocketClient ws_client(ws_uri, orderbook);

    std::cout << "Connecting to WebSocket: " << ws_uri << std::endl;

    // Run WebSocket client in a separate thread
    std::thread ws_thread([&ws_client]() {
        ws_client.run();
    });

    // Allow some time for data to be received and processed
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // Fetch current orderbook snapshot
    auto asks = orderbook.get_asks();
    auto bids = orderbook.get_bids();

    if (asks.empty() || bids.empty()) {
        std::cerr << "Orderbook data not received or empty." << std::endl;
        ws_client.stop();
        ws_thread.join();
        return 1;
    }

    std::cout << "Orderbook data received:" << std::endl;
    std::cout << "Top ask: Price = " << asks[0].price << ", Quantity = " << asks[0].quantity << std::endl;
    std::cout << "Top bid: Price = " << bids[0].price << ", Quantity = " << bids[0].quantity << std::endl;

    // Measure end-to-end latency: from message receipt to UI update simulation
    auto start = std::chrono::high_resolution_clock::now();

    // Simulate processing and UI update
    double quantity = 100.0;
    double volatility = 0.05;
    int fee_tier = 1;

    double slippage = models.calculate_slippage(quantity, volatility);
    double fees = models.calculate_fees(quantity, fee_tier);
    double market_impact = models.calculate_market_impact(quantity, volatility);
    double net_cost = models.calculate_net_cost(quantity, volatility, fee_tier);
    double maker_taker = models.predict_maker_taker_proportion(quantity, volatility);

    // Simulate UI update delay
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> end_to_end_latency = end - start;

    std::cout << "Model outputs:" << std::endl;
    std::cout << "Slippage: " << slippage << std::endl;
    std::cout << "Fees: " << fees << std::endl;
    std::cout << "Market Impact: " << market_impact << std::endl;
    std::cout << "Net Cost: " << net_cost << std::endl;
    std::cout << "Maker/Taker Proportion: " << maker_taker << std::endl;

    std::cout << "End-to-end simulation loop latency: " << end_to_end_latency.count() << " ms" << std::endl;

    // Stop WebSocket client and join thread
    ws_client.stop();
    ws_thread.join();

    std::cout << "Integration test completed successfully." << std::endl;
    return 0;
}

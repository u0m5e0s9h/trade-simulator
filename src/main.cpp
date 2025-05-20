#include <iostream>
#include <thread>
#include "websocket_client.h"
#include "orderbook.h"
#include "models.h"
#include "ui.h"

int main() {
    std::cout << "Starting Trade Simulator..." << std::endl;

    // Initialize orderbook
    OrderBook orderbook;

    // Initialize WebSocket client with orderbook reference
    WebSocketClient ws_client("wss://ws.gomarket-cpp.goquant.io/ws/l2-orderbook/okx/BTC-USDT-SWAP", orderbook);

    // Start WebSocket client in a separate thread
    std::thread ws_thread([&ws_client]() {
        ws_client.run();
    });

    // Initialize UI with orderbook and models
    Models models;
    UI ui(orderbook, models);

    // Run UI main loop (blocking)
    ui.run();

    // Cleanup
    ws_client.stop();
    if (ws_thread.joinable()) {
        ws_thread.join();
    }

    std::cout << "Trade Simulator stopped." << std::endl;
    return 0;
}

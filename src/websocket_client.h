#pragma once

#include <string>
#include <atomic>
#include "orderbook.h"

class WebSocketClient {
public:
    WebSocketClient(const std::string& uri, OrderBook& orderbook);
    ~WebSocketClient();

    void run();
    void stop();

private:
    std::string uri_;
    OrderBook& orderbook_;
    std::atomic<bool> running_;

    void on_message(const std::string& message);
    void connect();
    void disconnect();
};

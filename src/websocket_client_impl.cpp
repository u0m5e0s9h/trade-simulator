#include "websocket_client.h"
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <functional>

using websocketpp::connection_hdl;
using client = websocketpp::client<websocketpp::config::asio_client>;

class WebSocketClientImpl {
public:
    WebSocketClientImpl(const std::string& uri, OrderBook& orderbook)
        : uri_(uri), orderbook_(orderbook), running_(false) {
        client_.init_asio();
        client_.set_message_handler(std::bind(&WebSocketClientImpl::on_message, this, std::placeholders::_1, std::placeholders::_2));
        client_.set_open_handler(std::bind(&WebSocketClientImpl::on_open, this, std::placeholders::_1));
        client_.set_close_handler(std::bind(&WebSocketClientImpl::on_close, this, std::placeholders::_1));
        client_.set_fail_handler(std::bind(&WebSocketClientImpl::on_fail, this, std::placeholders::_1));
    }

    void run() {
        websocketpp::lib::error_code ec;
        auto con = client_.get_connection(uri_, ec);
        if (ec) {
            std::cerr << "Could not create connection because: " << ec.message() << std::endl;
            return;
        }
        hdl_ = con->get_handle();
        client_.connect(con);
        running_ = true;
        client_.run();
    }

    void stop() {
        running_ = false;
        websocketpp::lib::error_code ec;
        client_.close(hdl_, websocketpp::close::status::going_away, "", ec);
        if (ec) {
            std::cerr << "Error closing connection: " << ec.message() << std::endl;
        }
    }

private:
    void on_message(connection_hdl hdl, client::message_ptr msg) {
        try {
            auto start = std::chrono::high_resolution_clock::now();

            std::cout << "[WebSocket] Received message of size: " << msg->get_payload().size() << std::endl;
            std::cout << "[WebSocket] Message payload (truncated): " << msg->get_payload().substr(0, 200) << std::endl;

            orderbook_.update_from_json(nlohmann::json::parse(msg->get_payload()));

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> processing_time = end - start;
            std::cout << "[Benchmark] Data processing latency: " << processing_time.count() << " ms" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[WebSocket] Error parsing message: " << e.what() << std::endl;
        }
    }

    void on_open(connection_hdl hdl) {
        std::cout << "[WebSocket] Connection opened." << std::endl;
    }

    void on_close(connection_hdl hdl) {
        std::cout << "[WebSocket] Connection closed." << std::endl;
        if (running_) {
            std::cout << "[WebSocket] Attempting to reconnect in 5 seconds..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
            run();
        }
    }

    void on_fail(connection_hdl hdl) {
        std::cout << "[WebSocket] Connection failed." << std::endl;
        if (running_) {
            std::cout << "[WebSocket] Attempting to reconnect in 5 seconds..." << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(5));
            run();
        }
    }

    std::string uri_;
    OrderBook& orderbook_;
    client client_;
    connection_hdl hdl_;
    bool running_;
};

// Wrapper class to hide implementation details
WebSocketClient::WebSocketClient(const std::string& uri, OrderBook& orderbook)
    : impl_(new WebSocketClientImpl(uri, orderbook)) {}

WebSocketClient::~WebSocketClient() {
    stop();
    delete impl_;
}

void WebSocketClient::run() {
    impl_->run();
}

void WebSocketClient::stop() {
    impl_->stop();
}

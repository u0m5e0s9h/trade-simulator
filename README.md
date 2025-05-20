# High-Performance Trade Simulator

This project is a high-performance trade simulator implemented in C++ that connects to real-time L2 orderbook data from the OKX exchange via WebSocket. It estimates transaction costs and market impact using advanced models and provides a user interface for input parameters and output results.

## Features

- Real-time L2 orderbook data processing from OKX via WebSocket
- Input parameters panel for exchange, asset, order type, quantity, volatility, fee tier
- Output parameters panel showing expected slippage, fees, market impact, net cost, maker/taker proportion, and internal latency
- Models implemented:
  - Almgren-Chriss market impact model
  - Regression models for slippage estimation
  - Logistic regression for maker/taker proportion prediction
- UI implemented using ImGui
- Logging and error handling
- Performance measurement hooks
- Comprehensive testing including benchmark, integration, performance, and model validation tests

## Setup

### Prerequisites

- Windows with Visual Studio 2022(0r compatible)
- C++17 compatible compiler
- CMake 3.15 or higher
- Boost libraries (system, thread) - install Boost 1.87
- `ImGui` and `WebSocket++` manually cloned or downloaded into `external/`
 ```bash
 # step to install build
   git submodule update --init --recursive
  
  ```

### Build Instructions

```bash
# Step 1: Clone repository
git clone https://github.com/u0m5e0s9h/trade-simulator.git
cd trade-simulator

# Step 2: Prepare build
cmake -S . -B build -G "Visual Studio 17 2022"

# Step 3: Compile
cmake --build build --config Release

# Step 4: Navigate to binaries
cd build/Release


## Running the Simulator

Run the main executable:

```bash
./trade_simulator
```

This will launch the UI with input and output panels.

## Running Tests

### Benchmark Tests

```bash
./benchmark_tests
```

### Integration Tests

```bash
./integration_test
```

### Performance Tests

```bash
./performance_tests
```

### Model Validation Tests

```bash
./model_validation_tests
```

## Documentation

See the `docs/MODELS_AND_ALGORITHMS.md` file for detailed explanations of models, algorithms, and performance analysis.

## Notes

- Memory usage measurement is not implemented in code; use external tools for profiling.
- Model parameters may require tuning based on real market data.
- Ensure network connectivity and VPN access for OKX WebSocket endpoint.

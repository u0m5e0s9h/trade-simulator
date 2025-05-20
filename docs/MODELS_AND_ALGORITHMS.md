# Models and Algorithms Documentation

## Model Selection and Parameters

### Almgren-Chriss Market Impact Model
- Used to estimate the market impact cost of executing large orders.
- Parameters include order size, volatility, and liquidity.
- The model balances market impact and risk to optimize execution strategy.

### Regression Techniques for Slippage Estimation
- Linear regression and quantile regression models are used.
- These models estimate expected slippage based on historical data and market parameters.
- Parameters include order quantity, volatility, and fee tier.

### Logistic Regression for Maker/Taker Proportion
- Predicts the proportion of maker vs taker orders.
- Uses features such as order type, market conditions, and historical data.

## Market Impact Calculation Methodology
- Based on Almgren-Chriss framework.
- Calculates temporary and permanent market impact.
- Incorporates volatility and order size to estimate cost.

## Performance Optimization Approaches
- Efficient data structures for orderbook management.
- Multi-threading for WebSocket data processing and UI updates.
- Minimizing locking and contention in shared data.
- Using lightweight UI framework (ImGui) for fast rendering.
- Benchmarking and profiling to identify bottlenecks.

## Performance Analysis Report

### Benchmarking Results
- Regression model baseline slippage calculation: ~1.4 ms
- Optimized regression model slippage calculation: ~112 ms (needs further optimization)
- Performance tests show orderbook updates processed within ~6-7 ms per batch.

### Optimization Documentation
- Initial implementation prioritized correctness and modularity.
- Profiling identified regression model as a bottleneck.
- Future work includes algorithmic improvements and hardware acceleration.
- Memory usage monitoring to be integrated with platform-specific tools.

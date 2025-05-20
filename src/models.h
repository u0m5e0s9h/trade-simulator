#ifndef MODELS_H
#define MODELS_H

class Models {
public:
    Models();
    ~Models();

    double calculate_market_impact(double quantity, double volatility);
    double calculate_slippage(double quantity, double volatility);
    double calculate_fees(double quantity, int fee_tier);
    double calculate_net_cost(double quantity, double volatility, int fee_tier);
    double predict_maker_taker_proportion(double quantity, double volatility);

    // Bonus optimized method
    double calculate_slippage_optimized(double quantity, double volatility);
};

#endif // MODELS_H

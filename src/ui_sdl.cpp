// #include "ui.h"
// #include <imgui.h>
// #include <imgui_impl_sdl.h>
// #include <imgui_impl_opengl3.h>
// #include <SDL.h>
// #include <SDL_opengl.h>
// #include <iostream>

// UI::UI(OrderBook& orderbook, Models& models)
//     : orderbook_(orderbook), models_(models), fee_tier_(1), quantity_(100.0), volatility_(0.05) {}

// UI::~UI() {}

// void UI::run() {
//     // Initialize SDL
//     if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
//         std::cerr << "Error: SDL_Init failed: " << SDL_GetError() << std::endl;
//         return;
//     }

//     // Setup OpenGL attributes
//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
//     SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

//     // Create window with OpenGL context
//     SDL_Window* window = SDL_CreateWindow("Trade Simulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 900, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
//     if (window == nullptr) {
//         std::cerr << "Error: SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
//         SDL_Quit();
//         return;
//     }

//     SDL_GLContext gl_context = SDL_GL_CreateContext(window);
//     if (gl_context == nullptr) {
//         std::cerr << "Error: SDL_GL_CreateContext failed: " << SDL_GetError() << std::endl;
//         SDL_DestroyWindow(window);
//         SDL_Quit();
//         return;
//     }

//     SDL_GL_MakeCurrent(window, gl_context);
//     SDL_GL_SetSwapInterval(1); // Enable vsync

//     // Setup ImGui context
//     IMGUI_CHECKVERSION();
//     ImGui::CreateContext();
//     ImGuiIO& io = ImGui::GetIO(); (void)io;

//     // Setup ImGui style
//     ImGui::StyleColorsDark();

//     // Setup Platform/Renderer backends
//     ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
//     ImGui_ImplOpenGL3_Init("#version 330");

//     bool done = false;
//     while (!done) {
//         // Poll and handle events
//         SDL_Event event;
//         while (SDL_PollEvent(&event)) {
//             ImGui_ImplSDL2_ProcessEvent(&event);
//             if (event.type == SDL_QUIT)
//                 done = true;
//             if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
//                 done = true;
//         }

//         // Start ImGui frame
//         ImGui_ImplOpenGL3_NewFrame();
//         ImGui_ImplSDL2_NewFrame(window);
//         ImGui::NewFrame();

//         // Render UI
//         render();

//         // Rendering
//         ImGui::Render();
//         glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
//         glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
//         glClear(GL_COLOR_BUFFER_BIT);
//         ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

//         SDL_GL_SwapWindow(window);
//     }

//     // Cleanup
//     ImGui_ImplOpenGL3_Shutdown();
//     ImGui_ImplSDL2_Shutdown();
//     ImGui::DestroyContext();

//     SDL_GL_DeleteContext(gl_context);
//     SDL_DestroyWindow(window);
//     SDL_Quit();
// }

// void UI::render() {
//     ImGui::Begin("Trade Simulator");

//     render_input_panel();
//     ImGui::SameLine();
//     render_output_panel();

//     ImGui::End();
// }

// void UI::render_input_panel() {
//     ImGui::BeginChild("Input Panel", ImVec2(300, 0), true);

//     ImGui::Text("Input Parameters");

//     // Exchange (fixed to OKX)
//     ImGui::Text("Exchange: OKX");

//     // Spot Asset (fixed to BTC-USDT-SWAP for now)
//     ImGui::Text("Spot Asset: BTC-USDT-SWAP");

//     // Order Type (fixed to market)
//     ImGui::Text("Order Type: Market");

//     // Quantity input
//     ImGui::InputDouble("Quantity (USD)", &quantity_, 1.0, 10.0);

//     // Volatility input
//     ImGui::InputDouble("Volatility", &volatility_, 0.01, 0.1);

//     // Fee Tier input
//     ImGui::SliderInt("Fee Tier", &fee_tier_, 1, 3);

//     ImGui::EndChild();
// }

// void UI::render_output_panel() {
//     ImGui::BeginChild("Output Panel", ImVec2(0, 0), true);

//     ImGui::Text("Output Parameters");

//     double slippage = models_.calculate_slippage(quantity_, volatility_);
//     double fees = models_.calculate_fees(quantity_, fee_tier_);
//     double market_impact = models_.calculate_market_impact(quantity_, volatility_);
//     double net_cost = models_.calculate_net_cost(quantity_, volatility_, fee_tier_);
//     double maker_taker = models_.predict_maker_taker_proportion(quantity_, volatility_);

//     ImGui::Text("Expected Slippage: %.6f", slippage);
//     ImGui::Text("Expected Fees: %.6f", fees);
//     ImGui::Text("Expected Market Impact: %.6f", market_impact);
//     ImGui::Text("Net Cost: %.6f", net_cost);
//     ImGui::Text("Maker/Taker Proportion: %.6f", maker_taker);

//     // Internal Latency placeholder
//     ImGui::Text("Internal Latency: N/A");

//     ImGui::EndChild();
// }




#include "ui.h"
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_opengl3.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <iostream>
#include <chrono>
#include <vector>
#include <string>

UI::UI(OrderBook& orderbook, Models& models)
    : orderbook_(orderbook), models_(models), fee_tier_(1), quantity_(100.0), volatility_(0.05),
      last_tick_time_(std::chrono::steady_clock::now()), internal_latency_ms_(0.0), spot_asset_index_(0)
{
    spot_assets_ = {
        "BTC-USDT-SWAP",
        "ETH-USDT-SWAP",
        "LTC-USDT-SWAP",
        "XRP-USDT-SWAP",
        "BCH-USDT-SWAP"
    };
}

UI::~UI() {}

void UI::record_tick_time() {
    auto now = std::chrono::steady_clock::now();
    internal_latency_ms_ = std::chrono::duration<double, std::milli>(now - last_tick_time_).count();
    last_tick_time_ = now;
}

void UI::run() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cerr << "Error: SDL_Init failed: " << SDL_GetError() << std::endl;
        return;
    }

    // Setup OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    // Create window with OpenGL context
    SDL_Window* window = SDL_CreateWindow("Trade Simulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 900, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        std::cerr << "Error: SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == nullptr) {
        std::cerr << "Error: SDL_GL_CreateContext failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    bool done = false;
    while (!done) {
        // Poll and handle events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        // Record tick time for internal latency measurement
        record_tick_time();

        // Render UI
        render();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void UI::render() {
    ImGui::Begin("Trade Simulator");

    render_input_panel();
    ImGui::SameLine();
    render_output_panel();

    ImGui::End();
}

void UI::render_input_panel() {
    ImGui::BeginChild("Input Panel", ImVec2(300, 0), true);

    ImGui::Text("Input Parameters");

    // Exchange (fixed to OKX)
    ImGui::Text("Exchange: OKX");

    // Spot Asset selection
    if (ImGui::BeginCombo("Spot Asset", spot_assets_[spot_asset_index_].c_str())) {
        for (int n = 0; n < (int)spot_assets_.size(); n++) {
            bool is_selected = (spot_asset_index_ == n);
            if (ImGui::Selectable(spot_assets_[n].c_str(), is_selected))
                spot_asset_index_ = n;
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // Order Type (fixed to market)
    ImGui::Text("Order Type: Market");

    // Quantity input
    ImGui::InputDouble("Quantity (USD)", &quantity_, 1.0, 10.0);

    // Volatility input
    ImGui::InputDouble("Volatility", &volatility_, 0.01, 0.1);

    // Fee Tier input
    ImGui::SliderInt("Fee Tier", &fee_tier_, 1, 3);

    ImGui::EndChild();
}

void UI::render_output_panel() {
    ImGui::BeginChild("Output Panel", ImVec2(0, 0), true);

    ImGui::Text("Output Parameters");

    double slippage = models_.calculate_slippage(quantity_, volatility_);
    double fees = models_.calculate_fees(quantity_, fee_tier_);
    double market_impact = models_.calculate_market_impact(quantity_, volatility_);
    double net_cost = models_.calculate_net_cost(quantity_, volatility_, fee_tier_);
    double maker_taker = models_.predict_maker_taker_proportion(quantity_, volatility_);

    ImGui::Text("Expected Slippage: %.6f", slippage);
    ImGui::Text("Expected Fees: %.6f", fees);
    ImGui::Text("Expected Market Impact: %.6f", market_impact);
    ImGui::Text("Net Cost: %.6f", net_cost);
    ImGui::Text("Maker/Taker Proportion: %.6f", maker_taker);

    // Internal Latency display
    ImGui::Text("Internal Latency: %.3f ms", internal_latency_ms_);

    ImGui::EndChild();
}

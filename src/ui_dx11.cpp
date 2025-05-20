#include "ui.h"
#include <imgui.h>
#include <imgui_impl_sdl.h>
#include <imgui_impl_dx11.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <d3d11.h>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <string>
#include <chrono>

// Data
static ID3D11Device*           g_pd3dDevice = NULL;
static ID3D11DeviceContext*    g_pd3dDeviceContext = NULL;
static IDXGISwapChain*         g_pSwapChain = NULL;
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

static SDL_Window*             g_Window = NULL;
static SDL_Event               g_Event;

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

HRESULT CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2,
        D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return E_FAIL;

    CreateRenderTarget();
    return S_OK;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

UI::UI(OrderBook& orderbook, Models& models)
    : orderbook_(orderbook), models_(models), fee_tier_(1), quantity_(100.0), volatility_(0.05), spot_asset_index_(0),
      last_tick_time_(std::chrono::steady_clock::now()), internal_latency_ms_(0.0)
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
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        std::cerr << "Error: SDL_Init failed: " << SDL_GetError() << std::endl;
        return;
    }

    // Create application window with SDL and DirectX11
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    g_Window = SDL_CreateWindow("Trade Simulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 900, 600, window_flags);
    if (g_Window == NULL)
    {
        std::cerr << "Error: SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return;
    }

    // Get native window handle for DirectX
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(g_Window, &wmInfo))
    {
        std::cerr << "Error: SDL_GetWindowWMInfo failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(g_Window);
        SDL_Quit();
        return;
    }
    HWND hwnd = wmInfo.info.win.window;

    if (CreateDeviceD3D(hwnd) < 0)
    {
        std::cerr << "Error: CreateDeviceD3D failed" << std::endl;
        SDL_DestroyWindow(g_Window);
        SDL_Quit();
        return;
    }

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForD3D(g_Window);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    bool done = false;
    while (!done)
    {
        // Poll and handle events
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
        }

        // Start ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplSDL2_NewFrame(g_Window);
        ImGui::NewFrame();

        // Record tick time for internal latency measurement
        record_tick_time();

        // Render UI
        render();

        // Rendering
        ImGui::Render();
        const float clear_color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    SDL_DestroyWindow(g_Window);
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


cmake_minimum_required(VERSION 3.15)
project(TradeSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(Boost REQUIRED COMPONENTS system thread)
# WebSocket++ and ImGui assumed to be included as submodules or installed

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/external/websocketpp
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

add_executable(trade_simulator
    src/main.cpp
    src/websocket_client.cpp
    src/orderbook.cpp
    src/models.cpp
    src/ui.cpp
)

add_executable(integration_test
    tests/integration_test.cpp
    src/websocket_client.cpp
    src/orderbook.cpp
    src/models.cpp
)

target_link_libraries(trade_simulator
    ${Boost_LIBRARIES}
    pthread
)

target_link_libraries(integration_test
    ${Boost_LIBRARIES}
    pthread
)

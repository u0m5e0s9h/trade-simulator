#include "ui.h"
#include <windows.h>
#include <d3d11.h>
#include <tchar.h>

#include <imgui.h>
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"

#include <iostream>
#include <chrono>
#include <vector>
#include <string>

// DX11 global objects
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Helper functions
bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 1280;
    sd.BufferDesc.Height = 720;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[1] = { D3D_FEATURE_LEVEL_11_0 };

    return SUCCEEDED(D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        featureLevelArray, 1, D3D11_SDK_VERSION,
        &sd, &g_pSwapChain, &g_pd3dDevice,
        &featureLevel, &g_pd3dDeviceContext
    ));
}

void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

UI::UI(OrderBook& orderbook, Models& models)
    : orderbook_(orderbook), models_(models), fee_tier_(1), quantity_(100.0), volatility_(0.05),
      spot_asset_index_(0), last_tick_time_(std::chrono::steady_clock::now()), internal_latency_ms_(0.0),
      ui_update_latency_ms_(0.0)
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
    // Register window class
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L,
                      GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
                      _T("TradeSimWindowClass"), NULL };
    RegisterClassEx(&wc);
    HWND hwnd = CreateWindow(wc.lpszClassName, _T("Trade Simulator"),
                             WS_OVERLAPPEDWINDOW, 100, 100, 1280, 720,
                             NULL, NULL, wc.hInstance, NULL);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return;
    }

    CreateRenderTarget();

    // Show window
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Main loop
    MSG msg{};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Record tick time for internal latency measurement
        record_tick_time();

        auto ui_start = std::chrono::high_resolution_clock::now();

        render();

        auto ui_end = std::chrono::high_resolution_clock::now();
        ui_update_latency_ms_ = std::chrono::duration<double, std::milli>(ui_end - ui_start).count();

        ImGui::Render();
        const float clear_color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // VSync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
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

    ImGui::Text("Order Type: Market");

    ImGui::InputDouble("Quantity (USD)", &quantity_, 1.0, 10.0);
    ImGui::InputDouble("Volatility", &volatility_, 0.01, 0.1);

    // Fix fee tier slider to allow values 1, 2, and 3
    static const char* fee_tier_items[] = { "1", "2", "3" };
    static int current_fee_tier_idx = fee_tier_ - 1;
    if (ImGui::Combo("Fee Tier", &current_fee_tier_idx, fee_tier_items, IM_ARRAYSIZE(fee_tier_items))) {
        fee_tier_ = current_fee_tier_idx + 1;
    }

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

    ImGui::Text("Internal Latency: %.3f ms", internal_latency_ms_);
    ImGui::Text("UI Update Latency: %.3f ms", ui_update_latency_ms_);

    ImGui::EndChild();
}

// Windows message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

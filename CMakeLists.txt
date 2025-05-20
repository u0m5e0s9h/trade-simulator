
cmake_minimum_required(VERSION 3.15)
project(TradeSimulator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Boost settings
set(BOOST_ROOT "C:/boost_1_87_0")
set(BOOST_INCLUDEDIR "C:/boost_1_87_0")
set(BOOST_LIBRARYDIR "C:/boost_1_87_0/stage/lib")

find_package(Boost REQUIRED COMPONENTS system thread)

if (NOT MSVC)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
endif()

# Include directories
include_directories(
    ${Boost_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/external/websocketpp
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui/backends
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/tests
)

# Source files
set(SOURCE_FILES
    src/main.cpp
    src/websocket_client.cpp
    src/orderbook.cpp
    src/models.cpp
    src/ui.cpp
)

# ImGui source files
set(IMGUI_SOURCES
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_demo.cpp # Optional

    # Win32 + DirectX11 backend
    external/imgui/backends/imgui_impl_win32.cpp
    external/imgui/backends/imgui_impl_dx11.cpp
)

# Define executable for main simulator
add_executable(trade_simulator
    ${SOURCE_FILES}
    ${IMGUI_SOURCES}
)

# Link libraries for trade_simulator
target_link_libraries(trade_simulator
    ${Boost_LIBRARIES}
    d3d11
    dxgi
    user32
    gdi32
    imm32
    shell32
    ole32
    oleaut32
    uuid
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:pthread>
)

# Integration test executable
add_executable(integration_test
    tests/integration_test.cpp
    src/websocket_client.cpp
    src/orderbook.cpp
    src/models.cpp
)

target_link_libraries(integration_test
    ${Boost_LIBRARIES}
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:pthread>
)

# Performance test executable
add_executable(performance_tests
    tests/performance_tests.cpp
    src/orderbook.cpp
    src/models.cpp
)

target_link_libraries(performance_tests
    ${Boost_LIBRARIES}
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:pthread>
)

# Model validation test executable
add_executable(model_validation_tests
    tests/model_validation_tests.cpp
    src/models.cpp
)

target_link_libraries(model_validation_tests
    ${Boost_LIBRARIES}
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:pthread>
)

# Benchmark test executable (NEW)
add_executable(benchmark_tests
    tests/benchmark_tests.cpp
    src/orderbook.cpp
    src/models.cpp
)

target_link_libraries(benchmark_tests
    ${Boost_LIBRARIES}
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:pthread>
)

# Enable CTest-based testing 
enable_testing()
add_test(NAME IntegrationTest COMMAND integration_test)
add_test(NAME PerformanceTests COMMAND performance_tests)
add_test(NAME ModelValidationTests COMMAND model_validation_tests)

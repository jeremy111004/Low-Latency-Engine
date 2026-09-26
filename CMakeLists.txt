cmake_minimum_required(VERSION 3.14)
project(LOB_Engine VERSION 1.0 LANGUAGES CXX)

# Enforce C++20 Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable LTO natively through CMake
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported)

# Fetch Google Benchmark automatically
include(FetchContent)
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable benchmark testing" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

# Define common HPC flags for reuse
set(HPC_FLAGS -O3 -march=native -Wall -Wextra -Wpedantic)

# ==========================================
# Module 1: Baseline Engine
# ==========================================
add_library(lob_baseline STATIC src/baseline/OrderBook.cpp)
target_include_directories(lob_baseline PUBLIC include/baseline)
target_compile_options(lob_baseline PRIVATE ${HPC_FLAGS})
if(lto_supported)
    set_property(TARGET lob_baseline PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

add_executable(bench_baseline benchmarks/baseline_main.cpp)
target_link_libraries(bench_baseline PRIVATE lob_baseline benchmark::benchmark benchmark::benchmark_main pthread)
target_compile_options(bench_baseline PRIVATE ${HPC_FLAGS})
if(lto_supported)
    set_property(TARGET bench_baseline PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# ==========================================
# Module 2: V2 Engine
# ==========================================
add_library(lob_v2 STATIC src/v2/OrderBook2.cpp)
target_include_directories(lob_v2 PUBLIC include/v2)
target_compile_options(lob_v2 PRIVATE ${HPC_FLAGS})
if(lto_supported)
    set_property(TARGET lob_v2 PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

add_executable(bench_v2 benchmarks/v2_main.cpp)
target_link_libraries(bench_v2 PRIVATE lob_v2 benchmark::benchmark benchmark::benchmark_main pthread)
target_compile_options(bench_v2 PRIVATE ${HPC_FLAGS})
if(lto_supported)
    set_property(TARGET bench_v2 PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

# Debug executable (No optimizations, keeping debug symbols)
# ==========================================
# Module 3: V3 Engine
# ==========================================
add_library(lob_v3 STATIC src/v3/OrderBook2.cpp src/v3/parser.cpp)
target_include_directories(lob_v3 PUBLIC include/v3)
target_compile_options(lob_v3 PRIVATE ${HPC_FLAGS})
if(lto_supported)
    set_property(TARGET lob_v3 PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

add_executable(bench_v3 benchmarks/v3_main.cpp)
target_link_libraries(bench_v3 PRIVATE lob_v3 benchmark::benchmark benchmark::benchmark_main pthread)
target_compile_options(bench_v3 PRIVATE ${HPC_FLAGS})
if(lto_supported)
    set_property(TARGET bench_v3 PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

add_executable(bench_v3_live benchmarks/v3_mainlive.cpp)
target_link_libraries(bench_v3_live PRIVATE lob_v3 benchmark::benchmark benchmark::benchmark_main pthread)
target_compile_options(bench_v3_live PRIVATE ${HPC_FLAGS})
if(lto_supported)
    set_property(TARGET bench_v3_live PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
endif()

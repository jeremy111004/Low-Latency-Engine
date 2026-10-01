cmake_minimum_required(VERSION 3.14)
project(LOB_Engine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(HPC_FLAGS -O3 -march=native -Wall -Wextra -Wpedantic)

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported)

find_package(Threads REQUIRED)

include(FetchContent)
FetchContent_Declare(
  googlebenchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG        v1.8.3
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

function(lob_optimise target)
  target_compile_options(${target} PRIVATE ${HPC_FLAGS})
  if(lto_supported)
    set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  endif()
endfunction()

function(add_lob_engine name include_dir)
  add_library(${name} STATIC ${ARGN})
  target_include_directories(${name} PUBLIC ${include_dir})
  lob_optimise(${name})
endfunction()

function(add_lob_benchmark name source engine)
  add_executable(${name} ${source})
  target_link_libraries(${name} PRIVATE
    ${engine} benchmark::benchmark benchmark::benchmark_main Threads::Threads)
  lob_optimise(${name})
endfunction()

function(add_lob_tool name source engine)
  add_executable(${name} ${source})
  target_link_libraries(${name} PRIVATE ${engine} Threads::Threads)
  lob_optimise(${name})
endfunction()

add_lob_engine(lob_baseline include/baseline src/baseline/OrderBook.cpp)
add_lob_engine(lob_v2       include/v2       src/v2/OrderBook2.cpp)
add_lob_engine(lob_v3       include/v3       src/v3/OrderBook2.cpp src/v3/parser.cpp)

add_lob_benchmark(bench_baseline benchmarks/baseline_main.cpp lob_baseline)
add_lob_benchmark(bench_v2       benchmarks/v2_main.cpp       lob_v2)
add_lob_benchmark(bench_v3       benchmarks/v3_main.cpp       lob_v3)
add_lob_benchmark(bench_v3_live  benchmarks/v3_mainlive.cpp   lob_v3)

add_lob_tool(latency_v3 benchmarks/v3_latency.cpp lob_v3)

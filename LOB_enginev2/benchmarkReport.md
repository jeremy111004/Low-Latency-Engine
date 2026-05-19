jeremy1110@fedora:~/LOB_enginev2$ perf stat -e cycles,instructions,cache-misses,L1-dcache-load-misses,L1-icache-load-misses,branch-misses,page-faults ./engine
2026-05-19T18:04:04+02:00
Running ./engine
Run on (32 X 4149.91 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x16)
  L1 Instruction 32 KiB (x16)
  L2 Unified 1024 KiB (x16)
  L3 Unified 32768 KiB (x2)
Load Average: 0.22, 0.26, 0.18
----------------------------------------------------------------------------------------
Benchmark                              Time             CPU   Iterations UserCounters...
----------------------------------------------------------------------------------------
LOB_Continuous/iterations:500     241279 ns       241397 ns          500 items_per_second=82.851M/s

 Performance counter stats for './engine':

       583,606,753      cycles:u                                                                (82.99%)
     1,413,991,322      instructions:u                                                          (82.98%)
           558,156      cache-misses:u                                                          (82.97%)
        16,244,748      L1-dcache-load-misses:u                                                 (83.72%)
             7,160      L1-icache-load-misses:u                                                 (83.77%)
         6,115,305      branch-misses:u                                                         (83.56%)
             3,240      page-faults:u                                                         

       0.130059558 seconds time elapsed

       0.121014000 seconds user
       0.009001000 seconds sys


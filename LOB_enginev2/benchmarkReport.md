sudo perf stat -e cycles,instructions,branches,branch-misses,cache-misses,cache-references,L1-dcache-load-misses,L1-dcache-loads ./engine
2026-05-25T10:53:20+02:00
Running ./engine
Run on (32 X 4966.52 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x16)
  L1 Instruction 32 KiB (x16)
  L2 Unified 1024 KiB (x16)
  L3 Unified 32768 KiB (x2)
Load Average: 0.68, 0.39, 0.26
----------------------------------------------------------------------------------------
Benchmark                              Time             CPU   Iterations UserCounters...
----------------------------------------------------------------------------------------
LOB_Continuous/iterations:500     261883 ns       261888 ns          500 items_per_second=76.3685M/s

 Performance counter stats for './engine':

       691,755,449      cycles                                                                  (61.51%)
     1,452,482,937      instructions                                                            (61.69%)
       273,392,056      branches                                                                (62.43%)
         8,175,785      branch-misses                                                           (63.15%)
         1,102,127      cache-misses                                                            (63.69%)
        38,950,947      cache-references                                                        (63.24%)
        18,479,457      L1-dcache-load-misses                                                   (62.50%)
       714,826,571      L1-dcache-loads                                                         (61.80%)

       0.140857928 seconds time elapsed

       0.133804000 seconds user
       0.007042000 seconds sys

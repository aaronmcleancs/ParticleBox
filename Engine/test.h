#ifndef PARTICLE_SIM_TEST_H
#define PARTICLE_SIM_TEST_H

// Performance benchmark entry point. Runs a battery of headless benchmarks
// across different particle counts, comparing single-threaded vs multi-
// threaded execution and with/without the spatial grid. Prints results to
// stdout and returns when all scenarios are done.
void runPerformanceTests();

#endif

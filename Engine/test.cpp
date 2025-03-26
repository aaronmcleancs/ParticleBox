#include "test.h"

#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <cmath>

#include "simulation.h"

static float runSingleTest(int particleCount, int framesToSimulate,
                           bool multiThread, bool pairwise, bool grid)
{
    Simulation sim;
    sim.reset(particleCount);
    
    
    if (sim.isMultithreadingEnabled() != multiThread) {
        sim.toggleMultithreading();
    }
    if (sim.isReducedPairwiseComparisonsEnabled() != pairwise) {
        sim.toggleReducedPairwiseComparisons();
    }
    if (sim.isGridEnabled() != grid) {
        sim.toggleGrid();
    }

    sim.start();

    auto startTime = std::chrono::steady_clock::now();
    for (int frame = 0; frame < framesToSimulate; ++frame) {
        
        sim.update(0.016f);
    }
    auto endTime = std::chrono::steady_clock::now();
    double elapsedSec = std::chrono::duration<double>(endTime - startTime).count();

    float averageFPS = static_cast<float>(framesToSimulate / elapsedSec);
    return averageFPS;
}

void runPerformanceTests()
{
    
    std::vector<int> particleCounts = {100, 500, 1000, 2000, 5000, 10000};
    int framesToSimulate = 300;
    
    std::vector<bool> boolValues = {false, true};
    
    std::cout << "=== Comprehensive Simulation Performance Tests ===\n";
    std::cout << "We will test these particle counts: ";
    for (int pc : particleCounts) {
        std::cout << pc << " ";
    }
    std::cout << "\nFrames simulated per test: " << framesToSimulate << "\n\n";
    
    std::cout << "Particles"
              << "\tMultiThr"
              << "\tPairwise"
              << "\tGrid"
              << "\tAvgFPS\n";
    std::cout << "-----------------------------------------------\n";
    
    for (int pc : particleCounts) {
        for (bool mt : boolValues) {
            for (bool pw : boolValues) {
                for (bool gd : boolValues) {
                    float fps = runSingleTest(pc, framesToSimulate, mt, pw, gd);

                    
                    std::cout << pc << "\t"
                              << (mt ? "ON" : "OFF") << "\t\t"
                              << (pw ? "ON" : "OFF") << "\t\t"
                              << (gd ? "ON" : "OFF") << "\t"
                              << fps << "\n";
                }
            }
        }
    }

    std::cout << "\nPerformance tests complete.\n";
}
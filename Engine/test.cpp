#include "test.h"

#include "simulation.h"

#include <chrono>
#include <cstdio>
#include <vector>

namespace {

struct Scenario {
  int  particleCount;
  int  frames;
  bool multithread;
  bool grid;
  const char *label;
};

// Run one scenario and return total wall-clock ms for all update() calls.
double runScenario(const Scenario &s) {
  Simulation sim;
  sim.reset(s.particleCount);

  // Configure runtime flags through the shared InputState. Toggling has to
  // happen via the engine wrappers so the physics engine stays in sync.
  if (sim.isMultithreadingEnabled() != s.multithread) sim.toggleMultithreading();
  if (sim.isGridEnabled()           != s.grid)        sim.toggleGrid();

  // Use a fixed frame dt so the benchmark is reproducible.
  const float dt = 1.0f / 60.0f;

  auto t0 = std::chrono::steady_clock::now();
  for (int f = 0; f < s.frames; ++f) sim.update(dt);
  auto t1 = std::chrono::steady_clock::now();

  return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

} // namespace

void runPerformanceTests() {
  std::printf("==== Particle Simulation Benchmark ====\n");
  std::printf("Each scenario simulates a fixed number of frames headlessly\n");
  std::printf("and reports total ms + average ms per frame.\n\n");

  const int frames = 240; // ~4 seconds of simulated time at 60 fps
  std::vector<Scenario> scenarios = {
      {  500, frames, false, true,  "  500 particles   ST + grid"},
      {  500, frames, true,  true,  "  500 particles   MT + grid"},
      { 2000, frames, false, true,  " 2000 particles   ST + grid"},
      { 2000, frames, true,  true,  " 2000 particles   MT + grid"},
      { 2000, frames, true,  false, " 2000 particles   MT no-grid"},
      { 5000, frames, true,  true,  " 5000 particles   MT + grid"},
      {10000, frames, true,  true,  "10000 particles   MT + grid"},
  };

  std::printf("%-32s %12s %12s\n", "Scenario", "total (ms)", "per-frame (ms)");
  std::printf("---------------------------------------------------------------\n");
  for (const auto &s : scenarios) {
    double total = runScenario(s);
    double per   = total / static_cast<double>(s.frames);
    std::printf("%-32s %12.2f %12.3f\n", s.label, total, per);
  }
  std::printf("\nDone.\n");
}

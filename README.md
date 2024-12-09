Particle Simulation

Multithreaded particle simulation written in C++ using SDL2. Simulates basic physics interactions between thousands of particles.

Features

Spatial Partitioning
Replaced brute-force O(N²) collision checks with a uniform grid.
Particles are only compared to neighbors in adjacent cells, drastically reducing unnecessary computations and improving scalability.

Multithreading
Divided the particle set into chunks processed in parallel using multiple threads.
Utilizes multi-core CPUs effectively and speeds up force computation and updates.

Memory Management & Random Generation
Pre-allocated particle storage and replaced rand() with modern, thread-local random number generators.
Reduces allocation overhead and improves randomness quality and consistency.

Faster Particle Updates
Inlined common math operations and minimized function calls, reducing per-particle overhead.
Avoided unnecessary computations (like repeated sqrt() calls) by using squared values where possible.

Efficient Rendering
Implemented the midpoint circle algorithm for drawing particles.
Cuts down on draw calls and improves render performance.

Installation

Prerequisites
- Homebrew (for macOS users)
- C++ compiler supporting C++11 or later
- SDL2 and SDL2_ttf libraries

Steps
1. Install SDL2 and SDL2_ttf
   brew install sdl2 sdl2_ttf

2. Build the Project
   make

3. Run the Simulator
   ./ParticleSimulator

[Convert to GIF project speed change]
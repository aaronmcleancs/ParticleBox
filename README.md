Performant Computing Particle Simulation

Multithreaded particle simulation written in C++ using SDL2. Simulates basic physics interactions between thousands of particles.

Features

Spatial Partitioning
Replaced brute-force O(N²) collision checks with a partitioned grid.

Multithreading
Divided the particle set into chunks processed in parallel using threads.

Memory Management & Random Generation
Pre-allocated particle storage and replaced rand() with modern, thread-local rngs.

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
Particle Simulation
Multithreaded particle simulation written in C++ using SDL2. Simulation of basic physics interactions between thousands of particles.

brew install sdl2 sdl2_ttf
make
./ParticleSimulator

	•	Spatial Partitioning:
Replaced brute-force O(N²) collision checks with a uniform grid. Particles are only compared to neighbors in adjacent cells, drastically reducing unnecessary computations and improving scalability.
	•	Multithreading:
Divided the particle set into chunks processed in parallel using multiple threads. This utilizes multi-core CPUs effectively and speeds up force computation and updates.
	•	Memory Management & Random Generation:
Pre-allocated particle storage and replaced rand() with modern, thread-local random number generators. This reduces allocation overhead and improves randomness quality and consistency.
	•	Faster Particle Updates:
Inlined common math operations and minimized function calls, reducing per-particle overhead. Avoided unnecessary computations (like repeated sqrt() calls) by using squared values where possible.
	•	Efficient Rendering:
Implemented the midpoint circle algorithm for drawing particles, cutting down on draw calls and improving render performance.

![Convert to GIF project speed change](https://github.com/user-attachments/assets/f957c122-692e-4334-a618-6d247023f7f8)

Particle Simulation
This is a multithreaded particle simulation written in C++ using SDL2 for rendering. The simulation demonstrates basic physics interactions between particles, including boundary collisions and customizable behaviors.

Features
Multithreading: The simulation leverages multiple threads to efficiently update particle positions and interactions.
FPS Cap: The simulation is capped at 60 FPS to ensure smooth and consistent rendering across different hardware.
Dynamic Particle Management: Add or remove particles during the simulation to see how it affects the system.
Real-Time Frame Rate Display: The frame rate is calculated and displayed in real-time, allowing for performance monitoring.
Dependencies
SDL2: For rendering and handling window events.
SDL2_ttf: For text rendering.
Ensure you have the necessary libraries installed. On macOS, you can install them using Homebrew:

bash
Copy code
brew install sdl2 sdl2_ttf
Building
To build the project, simply use the provided Makefile:

bash
Copy code
make
This will compile the source files and link against the SDL2 libraries, producing an executable named ParticleSimulator.

Running the Simulation
After building, you can run the simulation with:

bash
Copy code
./ParticleSimulator
Controls
Spacebar: Start/stop the simulation.
R: Reset the simulation.
G: Toggle gravity.
+: Add a particle.
-: Remove a particle.
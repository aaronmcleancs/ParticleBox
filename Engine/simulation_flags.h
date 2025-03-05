#ifndef SIMULATION_FLAGS_H
#define SIMULATION_FLAGS_H

enum SimulationFlags {
    SIM_FLAG_NONE                   = 0,
    SIM_FLAG_MULTITHREADING_ENABLED = 1 << 0,
    SIM_FLAG_GRID_ENABLED           = 1 << 1,
    SIM_FLAG_REDUCED_PAIRWISE       = 1 << 2,
    SIM_FLAG_GRAVITY_ENABLED        = 1 << 3
};

inline SimulationFlags operator|(SimulationFlags a, SimulationFlags b) {
    return static_cast<SimulationFlags>(static_cast<int>(a) | static_cast<int>(b));
}

inline SimulationFlags operator&(SimulationFlags a, SimulationFlags b) {
    return static_cast<SimulationFlags>(static_cast<int>(a) & static_cast<int>(b));
}

inline SimulationFlags& operator|=(SimulationFlags &a, SimulationFlags b) {
    a = a | b;
    return a;
}

inline SimulationFlags& operator&=(SimulationFlags &a, SimulationFlags b) {
    a = a & b;
    return a;
}

#endif
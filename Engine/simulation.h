#include <vector>
#include "particle.h"
#include "physics.h"

class Simulation {
    PhysicsEngine physics;
    std::vector<Particle> particles;
public:
    Simulation();
    void update(double deltaTime);
    void render(SDL_Renderer* renderer);
    void handleEvent(const SDL_Event& event);
    Particle createRandomParticle();
};

#ifndef PARTICLES_H
#define PARTICLES_H

#include <SDL2/SDL.h>

#include "./vec2.hpp"
#include "./util.hpp"
#include "./texture.hpp"
#include "./tiles.hpp"

#include <array>
#include <cstdlib>

struct Particle
{
    vec2<double> pos;
    vec2<double> vel;
    double size;
    SDL_Color color;
};

class ParticleSpawner
{
private:
    const int _total;
    Particle** _particles;
    int _spawning;

    vec2<double> _pos;
    vec2<double> _friction;
    const double _gravity;
    const double _decay;
    const bool _solid;

    vec2<double> _vel{0.0, 0.0};
    SDL_Color _color {0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE};

public:
    ParticleSpawner(const int total_particles, int spawning, vec2<double> pos, vec2<double> friction, const double gravity, const double decay, const bool solid);
    ~ParticleSpawner();

    int getSpawning() {return _spawning;}
    void setSpawning(int spawning, vec2<double> vel, SDL_Color color);

    bool isDead(Particle* particle);

    void updateParticle(Particle* particle, const double& time_step, World* world);
    void renderParticle(Particle* particle, const int scrollX, const int scrollY, SDL_Renderer* renderer, Texture* tex);

    void update(const double& time_step, vec2<double> pos, const int scrollX, const int scrollY, SDL_Renderer* renderer, World* world, Texture* tex);
};

#endif
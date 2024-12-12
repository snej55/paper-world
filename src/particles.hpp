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
    SDL_Color _palette[8] {{0xa8, 0x60, 0x5d}, {0xd1, 0xa6, 0x7e}, {0xf6, 0xe7, 0x9c}, {0xb6, 0xcf, 0x8e}, {0x60, 0xae, 0x7b}, {0x3c, 0x6b, 0x64}, {0x1f, 0x24, 0x4b}, {0x65, 0x40, 0x53}};
    int _palette_length {8};

public:
    ParticleSpawner(const int total_particles, int spawning, vec2<double> pos, vec2<double> friction, const double gravity, const double decay, const bool solid);
    ~ParticleSpawner();

    int getSpawning() {return _spawning;}
    void setSpawning(int spawning, vec2<double> vel, SDL_Color color);

    void setPos(vec2<double> pos) {_pos = pos;}

    template <int N>
    void setPalette(const SDL_Color palette[N])
    {
        // do nothing :(
    }

    void setPalette(SDL_Color* palette, int palette_length)
    {
        // do nothing :(
    }

    bool isDead(Particle* particle);

    void updateParticle(Particle* particle, const double& time_step, World* world);
    void renderParticle(Particle* particle, const int scrollX, const int scrollY, SDL_Renderer* renderer, TexMan* texman);

    void update(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, World* world, TexMan* texman);
};

struct Smoke
{
    vec2<double> pos;
    vec2<double> vel;
    double size;
    double angle;
    double target_angle;
    SDL_Color color;
};

class SmokeSpawner
{
private:
    const int _total;
    int _spawning;
    Smoke** _smoke;
    vec2<double> _pos;
    const double _decay;
    const bool _solid;
    vec2<double> _vel{0.0, 0.0};
    SDL_Color _color {0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE};

public:
    SmokeSpawner(const int total_particles, int spawning, vec2<double> pos, const double decay, const bool solid);
    ~SmokeSpawner();

    int getSpawning() {return _spawning;}
    void setSpawning(int spawning, vec2<double> vel, SDL_Color color);

    void setPos(vec2<double> pos) {_pos = pos;}

    bool isDead(Smoke* smoke);

    void updateSmoke(Smoke* smoke, const double& time_step, World* world);
    void renderSmoke(Smoke* smoke, const int scrollX, const int scrollY, SDL_Renderer* renderer, Texture* tex);

    void update(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, World* world, Texture* tex);
};

struct Fire
{
    vec2<double> pos;
    vec2<double> vel;
    double frame{0};
};

class FireSpawner
{
private:
    const int _total;
    int _spawning;
    Fire** _fire;
    vec2<double> _pos;
    const double _decay;
    const bool _solid;

public:
    FireSpawner(const int total_particles, int spawning, vec2<double> pos, const double decay, const bool solid);
    ~FireSpawner();

    int getSpawning() {return _spawning;}
    void setSpawning(int spawning) {_spawning = spawning;}

    void setPos(vec2<double> pos) {_pos = pos;}

    bool isDead(Fire* fire);

    void updateFire(Fire* fire, const double& time_step, World* world);
    void renderFire(Fire* fire, const int scrollX, const int scrollY, SDL_Renderer* renderer, Texture* tex);
    
    void update(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, World* world, Texture* tex);
};

#endif
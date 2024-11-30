#include "particles.hpp"

ParticleSpawner::ParticleSpawner(const int total_particles, int spawning, vec2<double> pos, vec2<double> friction, const double gravity, const double decay, const bool solid)
 : _total{total_particles}, _spawning{spawning}, _pos{pos}, _friction{friction}, _gravity{gravity}, _decay{decay}, _solid{solid}
{
    _particles = new Particle*[total_particles];
    for (std::size_t i{0}; i < total_particles; ++i)
    {
        _particles[i] = nullptr;
    }
}
ParticleSpawner::~ParticleSpawner()
{
    for (std::size_t i{0}; i < _total; ++i)
    {
        if (_particles[i] != nullptr)
            delete _particles[i];
        _particles[i] = nullptr;
    }
    delete _particles;
}

void ParticleSpawner::setSpawning(int spawning, vec2<double> vel, SDL_Color color)
{
    _spawning = spawning;
    _vel = vel;
    _color = color;
}

bool ParticleSpawner::isDead(Particle* particle)
{
    return particle->size < 0.1;
}

void ParticleSpawner::updateParticle(Particle* particle, const double& time_step, World* world)
{
    particle->vel.y += _gravity * time_step;
    particle->vel.x *= _friction.x;
    particle->vel.y *= _friction.y;
    particle->pos.x += particle->vel.x * time_step;
    if (_solid)
    {
        Tile* tile {world->getTileAt(particle->pos.x, particle->pos.y)};
        if (tile != nullptr)
        {
            if (tile->type != TileType::SPIKE)
            {
                particle->pos.x -= particle->vel.x * time_step;
                particle->vel.x *= -0.7;
                particle->vel.y *= 0.9;
            }
        }
    }
    particle->pos.y += particle->vel.y * time_step;
    if (_solid)
    {
        Tile* tile {world->getTileAt(particle->pos.x, particle->pos.y)};
        if (tile != nullptr)
        {
            if (tile->type != TileType::SPIKE)
            {
                particle->pos.y -= particle->vel.y * time_step;
                particle->vel.y *= -0.7;
                particle->vel.x *= 0.9;
            }
        }
    }
    particle->size -= _decay * time_step;
}

void ParticleSpawner::renderParticle(Particle* particle, const int scrollX, const int scrollY, SDL_Renderer* renderer,  Texture* tex)
{
    particle->color.a = static_cast<uint8_t>(static_cast<int>(particle->size / 5.0 * 255.0));
    tex->setColor(particle->color.r, particle->color.g, particle->color.b);
    tex->setAlpha(particle->color.a);
    tex->setBlendMode(SDL_BLENDMODE_ADD);
    tex->render((int)particle->pos.x - scrollX, (int)particle->pos.y - scrollY, renderer);
}

void ParticleSpawner::update(const double& time_step, vec2<double> pos, const int scrollX, const int scrollY, SDL_Renderer* renderer, World* world, Texture* tex)
{
    for (std::size_t i{0}; i < _total; ++i)
    {
        Particle* particle {_particles[i]};
        if (particle == nullptr)
        {
            if (_spawning > 0)
            {
                --_spawning;
                _particles[i] = new Particle{pos, vec2<double>{Util::random() * _vel.x - _vel.x / 2.0, Util::random() * _vel.y - _vel.y / 2.0}, 5.0, _color};
            }
        } else {
            if (isDead(particle))
            {
                if (_spawning > 0)
                {
                    --_spawning;
                    delete particle;
                    _particles[i] = new Particle{pos, vec2<double>{Util::random() * _vel.x - _vel.x / 2.0, Util::random() * _vel.y - _vel.y / 2.0}, 5.0, _color};
                }
            } else {
                updateParticle(particle, time_step, world);
                renderParticle(particle, scrollX, scrollY, renderer, tex);
            }
        }
    }
}

SmokeSpawner::SmokeSpawner(const int total_particles, int spawning, vec2<double> pos, const double decay, const bool solid)
 : _total{total_particles}, _spawning{spawning}, _pos{pos}, _decay{decay}, _solid{solid}
{
    _smoke = new Smoke*[total_particles];
    for (std::size_t i{0}; i < total_particles; ++i)
    {
        _smoke[i] = nullptr;
    }
}

SmokeSpawner::~SmokeSpawner()
{
    for (std::size_t i{0}; i < _total; ++i)
    {
        if (_smoke[i] != nullptr)
            delete _smoke[i];
        _smoke[i] = nullptr;
    }
    delete _smoke;
}

void SmokeSpawner::setSpawning(int spawning, vec2<double> vel, SDL_Color color)
{
    _spawning = spawning;
    _vel = vel;
    _color = color;
}

bool SmokeSpawner::isDead(Smoke* smoke)
{
    return smoke->size >= 15.0;
}

void SmokeSpawner::updateSmoke(Smoke* smoke, const double& time_step)
{
    smoke->vel.y *= 0.989;
    smoke->vel.x *= 0.989;
    smoke->angle += std::min(4.0, (smoke->target_angle - smoke->angle) / 10.0) * time_step * (15.0 - std::min(smoke->size, 15.0)) / 15.0;
    smoke->size += _decay * time_step;
    smoke->pos.x += smoke->vel.x * time_step;
    smoke->pos.y += smoke->vel.y * time_step;
}

void SmokeSpawner::renderSmoke(Smoke* smoke, const int scrollX, const int scrollY, SDL_Renderer* renderer, Texture* tex)
{
    smoke->color.a = static_cast<uint8_t>(static_cast<int>((15.0 - std::min(smoke->size, 15.0)) / 15.0 * 255.0 * 0.6));
    tex->setColor(smoke->color.r, smoke->color.g, smoke->color.b);
    tex->setAlpha(smoke->color.a);
    tex->setBlendMode(SDL_BLENDMODE_ADD);
    tex->render((int)smoke->pos.x - scrollX - smoke->size / 2, (int)smoke->pos.y - scrollY - smoke->size / 2, renderer, smoke->angle, NULL, SDL_FLIP_NONE, NULL, smoke->size);
}

void SmokeSpawner::update(const double& time_step, vec2<double> pos, const int scrollX, const int scrollY, SDL_Renderer* renderer, Texture* tex)
{
    for (std::size_t i{0}; i < _total; ++i)
    {
        Smoke* smoke {_smoke[i]};
        if (smoke == nullptr)
        {
            if (_spawning > 0)
            {
                --_spawning;
                double angle{Util::random() * 360.0};
                _smoke[i] = new Smoke{pos, vec2<double>{Util::random() * _vel.x - _vel.x / 2.0, -Util::random() * _vel.y}, 1.0, angle, angle + 360 * Util::random() + 3600, _color};
            }
        } else {
            if (isDead(smoke))
            {
                if (_spawning > 0)
                {
                    --_spawning;
                    delete smoke;
                    double angle{Util::random() * 360.0};
                    _smoke[i] = new Smoke{pos, vec2<double>{Util::random() * _vel.x - _vel.x / 2.0, -Util::random() * _vel.y - 2.0}, 1.0, angle, angle + 360 * Util::random() + 360, _color};
                }
            } else {
                updateSmoke(smoke, time_step);
                renderSmoke(smoke, scrollX, scrollY, renderer, tex);
            }
        }
    }
}
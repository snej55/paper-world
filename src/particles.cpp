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
        delete _particles[i];
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
                particle->vel.y *= 0.99;
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
                particle->vel.x *= 0.99;
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
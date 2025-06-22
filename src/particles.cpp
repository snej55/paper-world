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
    delete[] _particles;
    delete[] _palette;
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
    particle->vel.x += (particle->vel.x * _friction.x - particle->vel.x) * time_step;
    particle->vel.y += (particle->vel.y * _friction.y - particle->vel.y) * time_step;
    particle->pos.x += particle->vel.x * time_step;
    if (_solid)
    {
        Tile* tile {world->getTileAt(particle->pos.x, particle->pos.y)};
        if (tile != nullptr)
        {
            if (tile->type != TileType::SPIKE)
            {
                particle->pos.x -= particle->vel.x * time_step;
                particle->vel.x *= -0.5;
                particle->vel *= 0.98;
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
                particle->vel.y *= -0.5;
                particle->vel *= 0.98;
            }
        }
    }
    particle->size -= _decay * time_step;
}

void ParticleSpawner::renderParticle(Particle* particle, const int scrollX, const int scrollY, SDL_Renderer* renderer, TexMan* texman)
{
    // texman->particleFire.setBlendMode(SDL_BLENDMODE_ADD);
    // texman->particleFire.setAlpha(particle->color.a);
    // SDL_Rect clip{0, 0, 5, 5};
    // texman->particleFire.render(static_cast<int>(particle->pos.x - 2.5) - scrollX, static_cast<int>(particle->pos.y - 2.5) - scrollY, renderer, &clip);
    particle->color.a = static_cast<uint8_t>(static_cast<int>(particle->size / 5.0 * 255.0));
    texman->particle.setColor(particle->color.r, particle->color.g, particle->color.b);
    texman->particle.setAlpha(particle->color.a);
    texman->particle.setBlendMode(SDL_BLENDMODE_BLEND);
    texman->particle.render((int)particle->pos.x - scrollX, (int)particle->pos.y - scrollY, renderer);
}

void ParticleSpawner::update(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, World* world, TexMan* texman)
{
    for (std::size_t i{0}; i < _total; ++i)
    {
        Particle* particle {_particles[i]};
        if (particle == nullptr)
        {
            if (_spawning > 0)
            {
                --_spawning;
                SDL_Color color {_color};
                if (_palette != nullptr)
                {
                    color = _palette[static_cast<std::size_t>(std::rand() % _palette_length)];
                    //std::cout << "Color(" << (int)color.r << ", " << (int)color.g << ", " << (int)color.b << ", " << (int)color.a << ")\n";
                }
                _particles[i] = new Particle{_pos, vec2<double>{Util::random() * _vel.x - _vel.x / 2.0, Util::random() * _vel.y - _vel.y / 2.0}, 5.0, color};
            }
        } else {
            if (isDead(particle))
            {
                if (_spawning > 0)
                {
                    --_spawning;
                    delete particle;
                    SDL_Color color {_color};
                    if (_palette != nullptr)
                    {
                        color = _palette[static_cast<std::size_t>(std::rand() % _palette_length)];
                        //std::cout << "Color(" << (int)color.r << ", " << (int)color.g << ", " << (int)color.b << ", " << (int)color.a << ")\n";
                    }
                    _particles[i] = new Particle{_pos, vec2<double>{Util::random() * _vel.x - _vel.x / 2.0, Util::random() * _vel.y - _vel.y / 2.0}, 5.0, color};
                }
            } else {
                updateParticle(particle, time_step, world);
                renderParticle(particle, scrollX, scrollY, renderer, texman);
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
    delete[] _smoke;
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

void SmokeSpawner::updateSmoke(Smoke* smoke, const double& time_step, World* world)
{
    smoke->vel.y += (smoke->vel.y * 0.98 - smoke->vel.y) * time_step;
    smoke->vel.x += (smoke->vel.x * 0.98 - smoke->vel.x) * time_step;
    smoke->angle += std::min(7.0, (smoke->target_angle - smoke->angle) / 15.0) * time_step;
    smoke->size += _decay * time_step;
    smoke->pos.x += smoke->vel.x * time_step;
    if (_solid)
    {
        Tile* tile {world->getTileAt(smoke->pos.x, smoke->pos.y)};
        if (tile != nullptr)
        {
            if (tile->type != TileType::SPIKE)
            {
                smoke->pos.x -= smoke->vel.x * time_step;
                smoke->vel.x *= -0.8;
            }
        }
    }
    if (_solid)
    {
        smoke->vel.y += 0.01 * time_step;
    }
    smoke->pos.y += smoke->vel.y * time_step;
    if (_solid)
    {
        Tile* tile {world->getTileAt(smoke->pos.x, smoke->pos.y)};
        if (tile != nullptr)
        {
            if (tile->type != TileType::SPIKE)
            {
                smoke->pos.y -= smoke->vel.y * time_step;
                smoke->vel.y *= -0.8;
            }
        }
    }
}

void SmokeSpawner::renderSmoke(Smoke* smoke, const int scrollX, const int scrollY, SDL_Renderer* renderer, Texture* tex)
{
    smoke->color.a = static_cast<uint8_t>(static_cast<int>((15.0 - std::min(smoke->size, 15.0)) / 15.0 * 255.0 * 0.6));
    tex->setColor(smoke->color.r, smoke->color.g, smoke->color.b);
    tex->setAlpha(smoke->color.a);
    tex->setBlendMode(SDL_BLENDMODE_ADD);
    tex->render((int)smoke->pos.x - scrollX - smoke->size / 2, (int)smoke->pos.y - scrollY - smoke->size / 2, renderer, smoke->angle, NULL, SDL_FLIP_NONE, NULL, smoke->size);
    tex->setBlendMode(SDL_BLENDMODE_NONE);
}

void SmokeSpawner::update(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, World* world, Texture* tex)
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
                double speed{Util::random() + 1};
                double sangle{Util::random() * M_PI * 2};
                _smoke[i] = new Smoke{_pos, vec2<double>{std::cos(sangle) * speed, std::sin(sangle) * speed}, 1.0, angle, angle + 360 * Util::random() + 360, _color};
            }
        } else {
            if (isDead(smoke))
            {
                if (_spawning > 0)
                {
                    --_spawning;
                    delete smoke;
                    double angle{Util::random() * 360.0};
                    double speed{Util::random() + 1};
                    double sangle{Util::random() * M_PI * 2};
                    SDL_Color color{_color};
                    color.b += std::rand() % 5;
                    _smoke[i] = new Smoke{_pos, vec2<double>{std::cos(sangle) * speed, std::sin(sangle) * speed}, 1.0, angle, angle + 360 * Util::random() + 360, _color};
                }
            } else {
                updateSmoke(smoke, time_step, world);
                renderSmoke(smoke, scrollX, scrollY, renderer, tex);
            }
        }
    }
}

FireSpawner::FireSpawner(const int total_particles, int spawning, vec2<double> pos, const double decay, const bool solid)
 : _total{total_particles}, _spawning{spawning}, _pos{pos}, _decay{decay}, _solid{solid}
{
    _fire = new Fire*[total_particles];
    for (std::size_t i{0}; i < total_particles; ++i)
    {
        _fire[i] = nullptr;
    }
}

FireSpawner::~FireSpawner()
{
    for (std::size_t i{0}; i < _total; ++i)
    {
        if (_fire[i] != nullptr)
            delete _fire[i];
        _fire[i] = nullptr;
    }
    delete[] _fire;
}

bool FireSpawner::isDead(Fire* fire)
{
    return fire->frame >= 8.0;
}

void FireSpawner::updateFire(Fire* fire, const double& time_step, World* world)
{
    fire->pos.x += fire->vel.x * time_step;
    fire->pos.y += fire->vel.y * time_step;
    fire->frame += _decay * time_step;
}

void FireSpawner::renderFire(Fire* fire, const int scrollX, const int scrollY, SDL_Renderer* renderer, Texture* tex)
{
    SDL_Rect clip{0, 0, 5, 5};
    const int step{(int)std::min(7.0, fire->frame)};
    clip.x = step * 5;
    tex->setAlpha(0x88);
    tex->setBlendMode(SDL_BLENDMODE_ADD);
    tex->render(static_cast<int>(fire->pos.x - 2.5) - scrollX, static_cast<int>(fire->pos.y - 2.5) - scrollY, renderer, &clip);
    tex->setBlendMode(SDL_BLENDMODE_NONE);
}

void FireSpawner::update(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, World* world, Texture* tex)
{
    for (std::size_t i{0}; i < _total; ++i)
    {
        Fire* fire {_fire[i]};
        if (fire == nullptr)
        {
            if (_spawning > 0)
            {
                --_spawning;
                double dist{Util::random() * 16.0 - 8.0};
                double angle{Util::random() * M_PI * 2};
                _fire[i] = new Fire{{_pos.x + std::cos(angle) * dist, _pos.y + std::sin(angle) * dist}, {0, -1.0 * Util::random() - 1.0}, static_cast<double>(std::rand() % 7)};
            }
        } else {
            if (isDead(fire))
            {
                if (_spawning > 0)
                {
                    --_spawning;
                    delete fire;
                    double dist{Util::random() * 16.0 - 8.0};
                    double angle{Util::random() * M_PI * 2};
                    _fire[i] = new Fire{{_pos.x + std::cos(angle) * dist, _pos.y + std::sin(angle) * dist}, {0, -1.0 * Util::random() - 1.0}, static_cast<double>(std::rand() % 7)};
                }
            } else {
                updateFire(fire, time_step, world);
                renderFire(fire, scrollX, scrollY, renderer, tex);
            }
        }
    }
}
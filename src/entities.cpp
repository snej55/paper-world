#include "./entities.hpp"

Entity::Entity(vec2<double> pos, vec2<double> vel, double gravity, bool peaceful, std::string name)
    : _pos{pos}, _vel{vel}, _gravity{gravity}, _peaceful{peaceful}, _name{name}
{
    _id = sId;
    ++sId;
    _rect.x = pos.x;
    _rect.y = pos.y;
    _rect.w = _dimensions.x;
    _rect.h = _dimensions.y;
}

int Entity::getId()
{
    return _id;
}

vec2<double> &Entity::getPos()
{
    return _pos;
}

vec2<double> Entity::getCenter()
{
    return {_pos.x + _dimensions.x / 2.0, _pos.y + _dimensions.y / 2.0};
}

void Entity::updateRect()
{
    _rect.x = _pos.x;
    _rect.y = _pos.y;
}

SDL_Rect *Entity::getRect()
{
    return &_rect;
}

bool Entity::getShouldDie()
{
    return _should_die;
}

bool Entity::getFlipped()
{
    return _flipped;
}

bool Entity::getPeaceful()
{
    return _peaceful;
}

std::string_view Entity::getName()
{
    return _name;
}

void Entity::damage(const double damage, double* screen_shake)
{
    *screen_shake = std::max(*screen_shake, 6.0);
    _recover = 0.0;
    _should_damage = true;
    setHealth(getHealth() - damage);
    if (getHealth() <= 0.0)
    {
        *screen_shake = std::max(*screen_shake, 8.0);
        die(screen_shake);
    }
}

void Entity::die(double *screen_shake)
{
    // if (!_should_die)
    //     *screen_shake = std::max(*screen_shake, 8.0);
    _should_die = true;
    _vel.x = 0;
}

void Entity::update(const double &time_step, World &world, double *screen_shake)
{
    _falling += time_step;
    _recover += time_step;
    _wander_timer -= time_step;
    if (_wander_timer <= 0.0)
    {
        _wandering = !_wandering;
        _wander_timer = (std::rand() % 180) + 60;
    }

    updateVel(time_step);
    handlePhysics(time_step, _vel, world, screen_shake);
    updateHealthBar();
}

void Entity::updateVel(const double &time_step)
{
    _vel.x = std::min(std::max(_vel.x, -_top_speed), _top_speed);
    _vel.y += _gravity * time_step;
}

void Entity::handlePhysics(const double &time_step, vec2<double> frame_movement, World &world, double *screen_shake)
{
    _pos.x += frame_movement.x * time_step;
    _rect.x = _pos.x;
    _rect.y = _pos.y;

    std::array<SDL_Rect, 9> rects;
    world.getTilesAroundPos(_pos, rects);
    for (int i{0}; i < 9; ++i)
    {
        SDL_Rect *tile_rect{&(rects[i])};
        if (Util::checkCollision(&_rect, tile_rect))
        {
            if (frame_movement.x > 0)
            {
                _rect.x = tile_rect->x - _rect.w;
            }
            else
            {
                _rect.x = tile_rect->x + tile_rect->w;
            }
            _pos.x = _rect.x;
            _vel.x = 0;
        }
    }

    _pos.y += frame_movement.y * time_step;
    _rect.x = _pos.x;
    _rect.y = _pos.y;

    world.getTilesAroundPos(_pos, rects);
    for (int i{0}; i < 9; ++i)
    {
        SDL_Rect *tile_rect{&(rects[i])};
        if (Util::checkCollision(&_rect, tile_rect))
        {
            if (frame_movement.y > 0)
            {
                _rect.y = tile_rect->y - _rect.h;
                _falling = 0.0;
            }
            else
            {
                _rect.y = tile_rect->y + tile_rect->h;
            }
            _vel.y = 0.0;
            _pos.y = _rect.y;
        }
    }

    world.getDangerAroundPos(_pos, rects);
    for (int i{0}; i < 9; ++i)
    {
        SDL_Rect *tile_rect{&(rects[i])};
        if (Util::checkCollision(&_rect, tile_rect))
        {
            die(screen_shake);
            break;
        }
    }
}

void Entity::render(const int scrollX, const int scrollY, SDL_Renderer *renderer)
{
    SDL_Rect renderRect{static_cast<int>(_pos.x) - scrollX, static_cast<int>(_pos.y) - scrollY, _dimensions.x, _dimensions.y};
    if (_recover < _recover_time - 1.0)
    {
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
    }

    SDL_RenderFillRect(renderer, &renderRect);
}

void Entity::touchPlayer(Player *player, double *screen_shake, double* slomo)
{
    _rect.x = _pos.x;
    _rect.y = _pos.y;
    SDL_Rect *player_rect{player->getRect()};
    if (player->getAttacking())
    {
        SDL_Rect playerAttackRect {player->getAttackRect()};
        if (Util::checkCollision(&_rect, &playerAttackRect) && _recover > _recover_time && player->getRecover() > 10.0)
        {
            damage(player->getSwordDamage(), screen_shake);
        }
    } else if (Util::checkCollision(player_rect, &_rect) && _recover > _recover_time && player->getRecover() > 10.0)
    {
        player->damage(_damage, screen_shake, slomo);
    }
}

void Entity::followPlayer(Player *player, World *world, const double& time_step)
{
    SDL_Rect *player_rect{player->getRect()};
    vec2<double> player_pos{player->getCenter()};
    if (Util::distance(player_pos, getCenter()) < 100.0)
    {
        vec2<double> checkTilePos{getCenter().x + (_flipped ? -10.0 : 10.0), getCenter().y + 8};
        Tile *tile{world->getTileAt(checkTilePos.x, checkTilePos.y)};
        if (tile == nullptr)
        {
            if (!(std::abs(static_cast<int>(player_pos.x - getCenter().x)) < 16 && player_pos.y > getCenter().y + 4.0 && _falling < 3.0))
            {
                _vel.x += (_flipped ? 0.2 : -0.2) * time_step;
                _flipped = !_flipped;
            }
        }
        else if (Util::elementIn<TileType, std::size(DANGER_TILES)>(tile->type, DANGER_TILES))
        {
            if (!(std::abs(static_cast<int>(player_pos.x - getCenter().x)) < 16 && player_pos.y > getCenter().y + 4.0 && _falling < 3.0))
            {
                _vel.x += (_flipped ? 0.2 : -0.2) * time_step;
                _flipped = !_flipped;
            }
        }
        else
        {
            if (player_pos.x > getCenter().x) // player is to the right of entity
            {
                _flipped = false;
            }
            else
            {
                _flipped = true;
            }
            _vel.x += (_flipped ? -0.1 : 0.1) * time_step;
            _anim_flipped = _flipped;
        }
        if (player_pos.y + 4.0 < getCenter().y)
        {
            if (_falling < 3.0)
            {
                _vel.y = -3.2;
            }
        }
    }
    else
    {
        wander(world, time_step);
    }
}

void Entity::wander(World *world, const double& time_step)
{
        {
        if (_wandering)
        {
            vec2<double> checkTilePos{getCenter().x + (_flipped ? -10.0 : 10.0), getCenter().y + 10};
            Tile *tile{world->getTileAt(checkTilePos.x, checkTilePos.y)};
            if (tile == nullptr)
            {
                _vel.x += (_flipped ? 0.2 : -0.2) * time_step;
                _flipped = !_flipped;
            }
            else if (Util::elementIn<TileType, std::size(DANGER_TILES)>(tile->type, DANGER_TILES))
            {
                _vel.x += (_flipped ? 0.2 : -0.2) * time_step;
                _flipped = !_flipped;
            }
            else
            {
                checkTilePos = {getCenter().x + (_flipped ? -10.0 : 10.0), getCenter().y};
                tile = {world->getTileAt(checkTilePos.x, checkTilePos.y)};
                if (tile != nullptr)
                {
                    if (Util::elementIn<TileType, std::size(SOLID_TILES)>(tile->type, SOLID_TILES))
                    {
                        _flipped = !_flipped;
                    }
                }
                _vel.x += _flipped ? -0.08 : 0.08;
            }
        }
        else
        {
            _vel.x *= 0.8;
        }
        _anim_flipped = _flipped;
    }
}

void Entity::updateHealthBar()
{
    if (_health_bar != nullptr)
    {
        _health_bar->update(this);
    }
}

void Entity::renderHealthBar(const int scrollX, const int scrollY, SDL_Renderer* renderer)
{
    if (_health_bar != nullptr && getHealth() < getMaxHealth())
    {
        _health_bar->render(scrollX, scrollY, renderer, this);
    }
}

Slime::Slime(vec2<double> pos, vec2<double> vel, double gravity, bool peaceful, std::string name, TexMan* texman)
    : Entity{pos, vel, gravity, peaceful, name}
{
    loadAnim(texman);
    _maxHealth = 40.0;
    _health = 40.0;
}

Slime::~Slime()
{
    delete _idleAnim;
    delete _runAnim;
    delete _jumpAnim;
    delete _flash;
}

void Slime::loadAnim(TexMan* texman)
{
    _idleAnim = new Anim{13, 9, 6, 0.16, true, &(texman->slimeIdle)};
    _runAnim = new Anim{13, 9, 5, 0.2, true, &(texman->slimeRun)};
    _jumpAnim = new Anim{13, 9, 8, 0.21, true, &(texman->slimeJump)};
    _flash = new Anim{13, 9, 1, 0.2, true, &(texman->slimeFlash)};
    Entity::loadAnim(texman);
    _health_bar->setDimensions({11, 2});
}

void Slime::handleAnim(const double& time_step)
{
    _anim = _idleAnim;
    if (_falling >= 3.0)
    {
        _anim = _jumpAnim;
        _runAnim->reset();
        _idleAnim->reset();
    } else if (std::abs(_vel.x) > 0.05)
    {
        _anim = _runAnim;
        _jumpAnim->reset();
        _idleAnim->reset();
    } else {
        _anim = _idleAnim;
        _jumpAnim->reset();
        _runAnim->reset();
    }
    _anim->setFlipped(_anim_flipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
    _anim->tick(time_step);
}

void Slime::update(const double& time_step, World& world, double* screen_shake)
{
    handleAnim(time_step);
    Entity::update(time_step, world, screen_shake);
}

void Slime::render(const int scrollX, const int scrollY, SDL_Renderer* renderer)
{
    if (_recover > _recover_time)
    {
        _anim->render((int)_pos.x - _anim_offset.x, (int)_pos.y - _anim_offset.y, scrollX, scrollY, renderer);
    } else {
        _flash->render((int)_pos.x - _anim_offset.x, (int)_pos.y - _anim_offset.y, scrollX, scrollY, renderer);
    }
    Entity::renderHealthBar(scrollX, scrollY, renderer);
}


Bat::Bat(vec2<double> pos, vec2<double> vel, double gravity, bool peaceful, std::string name, TexMan* texman)
    : Entity{pos, vel, 0.05, peaceful, name}
{
    loadAnim(texman);
    _maxHealth = 20.0;
    _health = 20.0;
}

Bat::~Bat()
{
    delete _anim;
    delete _flash;
}

void Bat::loadAnim(TexMan* texman)
{
    _anim = new Anim{7, 4, 2, 0.3, true, &(texman->bat)};
    _flash = new Anim{7, 4, 1, 0.2, true, &(texman->batFlash)};
    _glowTex = &(texman->lightTex);
    Entity::loadAnim(texman);
    _health_bar->setDimensions({7, 2});
    _health_bar->setOffset({1, 0});
}

void Bat::handleAnim(const double& time_step)
{
    _anim->setAngle(_angle);
    _anim->tick(time_step);
}

void Bat::handlePhysics(const double& time_step, vec2<double> frame_movement, World& world, double* screen_shake)
{
    _pos.x += frame_movement.x * time_step * _speed;
    _rect.x = _pos.x;
    _rect.y = _pos.y;

    std::array<SDL_Rect, 9> rects;
    world.getTilesAroundPos(_pos, rects);
    for (int i{0}; i < 9; ++i)
    {
        SDL_Rect* tile_rect {&(rects[i])};
        if (Util::checkCollision(&_rect, tile_rect))
        {
            if (frame_movement.x > 0)
            {
                _rect.x = tile_rect->x - _rect.w;
            } else {
                _rect.x = tile_rect->x + tile_rect->w;
            }
            _pos.x = _rect.x;
            _vel.x *= -1.2;
            _vel.y *= 1.2;
        }
    }

    _pos.y += frame_movement.y * time_step * _speed;
    _rect.x = _pos.x;
    _rect.y = _pos.y;

    world.getTilesAroundPos(_pos, rects);
    for (int i{0}; i < 9; ++i)
    {
        SDL_Rect* tile_rect {&(rects[i])};
        if (Util::checkCollision(&_rect, tile_rect))
        {
            if (frame_movement.y > 0)
            {
                _rect.y = tile_rect->y - _rect.h;
                _falling = 0.0;
            } else {
                _rect.y = tile_rect->y + tile_rect->h;
            }
            _vel.y *= -1;
            _pos.y = _rect.y;
        }
    }

    world.getDangerAroundPos(_pos, rects);
    for (int i{0}; i < 9; ++i)
    {
        SDL_Rect* tile_rect{&(rects[i])};
        if (Util::checkCollision(&_rect, tile_rect))
        {
            die(screen_shake);
            break;
        }
    }
    _vel.x = std::min(3.0, std::max(-3.0, _vel.x));
    _vel.y = std::min(3.0, std::max(-3.0, _vel.y));
}

void Bat::damage(const double damage, double* screen_shake)
{
    Entity::damage(damage, screen_shake);
    double angle = Util::random() * 2.0 * M_PI;
    _vel.x += std::cos(angle) * 5.0;
    _vel.y += std::sin(angle) * 5.0;
}

void Bat::touchPlayer(Player* player, double* screen_shake, double* slomo)
{
    _rect.x = _pos.x;
    _rect.y = _pos.y;
    SDL_Rect* player_rect{player->getRect()};
    if (player->getAttacking())
    {
        SDL_Rect playerAttackRect {player->getAttackRect()};
        if (Util::checkCollision(&_rect, &playerAttackRect) && _recover > _recover_time && player->getRecover() > 10.0)
        {
            damage(player->getSwordDamage(), screen_shake);
        }
    } if (Util::checkCollision(player_rect, &_rect) && player->getRecover() > 10.0)
    {
        double angle = Util::random() * 2.0 * M_PI;
        _vel.x += std::cos(angle) * 5.0;
        _vel.y += std::sin(angle) * 5.0;
        player->damage(_damage, screen_shake, slomo);
    }
}

void Bat::followPlayer(Player* player, World* world, const double& time_step)
{
    SDL_Rect* player_rect{player->getRect()};
    vec2<double> player_pos{player->getCenter()};
    if (Util::distance(player_pos, getCenter()) < 200.0)
    {
        _vel.x += std::max(-0.1, std::min(0.1, (player_pos.x - getCenter().x) * 0.005)) * time_step;
        _vel.y += std::max(-0.1, std::min(0.1, (player_pos.y - getCenter().y) * 0.005)) * time_step;
        vec2<double> check_pos {getCenter().x + _vel.x * 2.0, getCenter().y + _vel.y * 2.0};
        Tile* tile {world->getTileAt(check_pos.x, check_pos.y)};
        if (tile != nullptr)
        {    
            if (Util::elementIn<TileType, std::size(DANGER_TILES)>(tile->type, DANGER_TILES))
            {
                _vel.x *= -1;
                _vel.y *= -1;
            }
        }
        _vel.x += (_vel.x * 0.95 - _vel.x) * time_step;
        _vel.y += (_vel.y * 0.95 - _vel.y) * time_step;
    } else {
        wander(world, time_step);
    }
}

void Bat::update(const double& time_step, World& world, double* screen_shake)
{
    handleAnim(time_step);
    Entity::update(time_step, world, screen_shake);
}

void Bat::render(const int scrollX, const int scrollY, SDL_Renderer* renderer)
{
    _glowTex->setBlendMode(SDL_BLENDMODE_ADD);
    _glowTex->setAlpha(10);
    _glowTex->setColor(246, 231, 156);
    SDL_Rect renderQuad{static_cast<int>(getCenter().x - 6 - _anim_offset.x) - scrollX, static_cast<int>(getCenter().y - 6 - _anim_offset.y - 2) - scrollY, 10, 10};
    SDL_RenderCopyEx(renderer, _glowTex->getTexture(), NULL, &renderQuad, 0, NULL, SDL_FLIP_NONE);
    //_glowTex->render(static_cast<int>(getCenter().x - 2.5 - _anim_offset.x) - scrollX, static_cast<int>(getCenter().y - 2.5 - _anim_offset.y - 2) - scrollY, renderer, &clip);
    if (_recover > _recover_time)
    {
        _anim->render((int)_pos.x - _anim_offset.x, (int)_pos.y - _anim_offset.y, scrollX, scrollY, renderer);
    } else {
        _flash->render((int)_pos.x - _anim_offset.x, (int)_pos.y - _anim_offset.y, scrollX, scrollY, renderer);
    }
    Entity::renderHealthBar(scrollX, scrollY, renderer);
}


Turtle::Turtle(vec2<double> pos, vec2<double> vel, double gravity, bool peaceful, std::string name, TexMan* texman)
    : Entity{pos, vel, 0.4, peaceful, name}
{
    loadAnim(texman);
    _maxHealth = 60.0;
    _health = 60.0;
}

Turtle::~Turtle()
{
    delete _idleAnim;
    delete _runAnim;
    delete _jumpAnim;
    delete _landAnim;
    delete _flash;
}

void Turtle::loadAnim(TexMan* texman)
{
    _idleAnim = new Anim{8, 8, 6, 0.2, true, &(texman->turtleIdle)};
    _runAnim = new Anim{8, 8, 5, 0.2, true, &(texman->turtleRun)};
    _jumpAnim = new Anim{8, 8, 6, 0.2, false, &(texman->turtleJump)};
    _landAnim = new Anim{8, 8, 6, 0.3, false, &(texman->turtleLand)};
    _flash = new Anim{7, 4, 1, 0.2, true, &(texman->turtleFlash)};
    Entity::loadAnim(texman); // for the health_bar
    _health_bar->setDimensions({8, 2});
}

void Turtle::handleAnim(const double& time_step)
{
    _anim = _idleAnim;
    if (_falling >= 3.0)
    {
        _anim = _jumpAnim;
        _runAnim->reset();
        _idleAnim->reset();
        _landAnim->reset();
        _grounded = 99.9;
    } else if (std::abs(_vel.x) > 0.05)
    {
        _anim = _runAnim;
        _jumpAnim->reset();
        _idleAnim->reset();
        _landAnim->reset();
    } else if (_grounded > 3.0)
    {
        _anim = _landAnim;
        _jumpAnim->reset();
        _idleAnim->reset();
        _runAnim->reset();
        if (_landAnim->getFinished())
        {
            _grounded = 0.0;
        }
    } else {
        _anim = _idleAnim;
        _jumpAnim->reset();
        _runAnim->reset();
        _landAnim->reset();
        _grounded = 0.0;
    }
    _anim->tick(time_step);
    _anim->setFlipped(_anim_flipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void Turtle::damage(const double damage, double* screen_shake)
{
    _vel.x *= 0.1;
    Entity::damage(damage, screen_shake);
}

void Turtle::touchPlayer(Player* player, double* screen_shake, double* slomo)
{
    _rect.x = _pos.x;
    _rect.y = _pos.y;
    SDL_Rect *player_rect{player->getRect()};
    if (Util::checkCollision(&_rect, player_rect))
    {
        if (player->getFalling() > 3.0 && player->getVelY() > 0.1)
        {
            _grounded = 99.9;
            player->setVelY(-3.6);
            _landAnim->setFrame(2); // at the bottom
            _wandering = false;
            _wander_timer = 100.0;
            _jumped_on = true;
        }
    }
    if (player->getAttacking())
    {
        SDL_Rect playerAttackRect {player->getAttackRect()};
        if (Util::checkCollision(&_rect, &playerAttackRect) && _recover > _recover_time)
        {
            damage(player->getSwordDamage() * 0.5, screen_shake);
        }
    }
}

void Turtle::wander(World* world, const double& time_step)
{
    {
        if (_wandering)
        {
            vec2<double> checkTilePos{getCenter().x + (_flipped ? -10.0 : 10.0), getCenter().y + 10};
            Tile *tile{world->getTileAt(checkTilePos.x, checkTilePos.y)};
            if (tile == nullptr)
            {
                _vel.x += (_flipped ? 0.2 : -0.2) * time_step;
                _flipped = !_flipped;
            }
            else if (Util::elementIn<TileType, std::size(DANGER_TILES)>(tile->type, DANGER_TILES))
            {
                _vel.x += (_flipped ? 0.2 : -0.2) * time_step;
                _flipped = !_flipped;
            }
            else
            {
                checkTilePos = {getCenter().x + (_flipped ? -10.0 : 10.0), getCenter().y};
                tile = {world->getTileAt(checkTilePos.x, checkTilePos.y)};
                if (tile != nullptr)
                {
                    if (Util::elementIn<TileType, std::size(SOLID_TILES)>(tile->type, SOLID_TILES))
                    {
                        _flipped = !_flipped;
                        
                    }
                }
                _vel.x += _flipped ? -0.04 : 0.04;
            }
        }
        else
        {
            _vel.x *= 0.8;
        }
        _anim_flipped = _flipped;
    }
}

void Turtle::update(const double& time_step, World& world, double* screen_shake)
{
    handleAnim(time_step);
    _vel.x *= 0.9;
    Entity::update(time_step, world, screen_shake);
}

void Turtle::render(const int scrollX, const int scrollY, SDL_Renderer* renderer)
{
    if (_recover > _recover_time - 1.0)
    {
        _anim->render((int)_pos.x - _anim_offset.x, (int)_pos.y - _anim_offset.y, scrollX, scrollY, renderer);
    } else {
        _flash->render((int)_pos.x - _anim_offset.x, (int)_pos.y - _anim_offset.y, scrollX, scrollY, renderer);
    }
    Entity::renderHealthBar(scrollX, scrollY, renderer);
}

EntityManager::EntityManager(vec2<double> pos, const int total, Entity** entities)
    : _total{total}, _pos{pos}
{
    _Entities = new Entity*[MAX_ENTITIES];
    for (std::size_t i{0}; i < total; ++i)
    {
        _Entities[i] = nullptr;
        _Entities[i] = entities[i];
        _name = _Entities[i]->getName();
        _Entities[i]->setPalette(&_Particles);
    }
}

EntityManager::EntityManager(vec2<double> pos, const int total, std::vector<Entity*> entities)
    : _total{total}, _pos{pos}
{
    _Entities = new Entity*[MAX_ENTITIES];
    for (std::size_t i{0}; i < total; ++i)
    {
        _Entities[i] = nullptr;
        _Entities[i] = entities[i];
        _name = _Entities[i]->getName();
        _Entities[i]->setPalette(&_Particles);
    }
}

EntityManager::~EntityManager()
{
    free();
}

void EntityManager::free()
{
    for (std::size_t i{0}; i < _total; ++i)
    {
        if (_Entities[i] != nullptr)
        {
            delete _Entities[i];
            _Entities[i] = nullptr;
        }
    }
    _total = 0;
    delete[] _Entities;
    _Entities = nullptr;
}

std::string_view EntityManager::getName()
{
    return _name;
}

Entity* EntityManager::getEntity(std::size_t idx) {return _Entities[idx];}

int EntityManager::getTotal() {return _total;}

void EntityManager::addEntity(Entity* entity)
{
    _Entities[_total] = entity;
    ++_total;
}

void EntityManager::update(const double& time_step, World& world, double* screen_shake, Player* player, double* slomo, TexMan* texman, CoinManager* coinmanager)
{
    const int num{_total};
    for (std::size_t i{0}; i < num; ++i)
    {
        if (i < _total)
        {
            Entity* entity {_Entities[i]};
            entity->update(time_step, world, screen_shake);
            if (!(entity->getPeaceful()))
            {
                entity->followPlayer(player, &world, time_step);
                entity->touchPlayer(player, screen_shake, slomo);
            } else {
                entity->wander(&world, time_step);
                if (entity->getName() == "turtle")
                {
                    entity->touchPlayer(player, screen_shake, slomo);
                    if (static_cast<Turtle*>(entity)->getJumpedOn())
                    {
                        texman->SFX_turtle.play();
                        static_cast<Turtle*>(entity)->setJumpedOn(false);
                    }
                }
            }
            // some black magic
            if (entity->getShouldDie())
            {
                texman->SFX_death_0.play();
                if (entity->getName() == "turtle")
                {
                    _Particles.setPos(entity->getCenter());
                    _Smoke.setPos(entity->getCenter());
                    _Fire.setPos(entity->getCenter());
                    _Particles.setSpawning(32, {8.0, 8.0}, SDL_Color{0x00, 0x00, 0x00});
                    _Smoke.setSpawning(20, {1, 2}, {0x88, 0x88, 0x88});
                    _Fire.setSpawning(20);
                    int num{(std::rand() % 10) + 15};
                    for (int i{0}; i < num; ++i)
                    {
                        _SparkManager.addSpark(new Spark{entity->getCenter(), Util::random() * M_PI * 2.0, Util::random() * 3.0 + 2.0});
                    }
                } else if (entity->getName() == "slime")
                {
                    _Particles.setPos(entity->getCenter());
                    _Smoke.setPos(entity->getCenter());
                    _Fire.setPos(entity->getCenter());
                    _Particles.setSpawning(32, {8.0, 8.0}, SDL_Color{0x00, 0x00, 0x00});
                    _Smoke.setSpawning(20, {1, 2}, {0x88, 0x88, 0x88});
                    _Fire.setSpawning(20);
                    int num{(std::rand() % 10) + 15};
                    for (int i{0}; i < num; ++i)
                    {
                        _SparkManager.addSpark(new Spark{entity->getCenter(), Util::random() * M_PI * 2.0, Util::random() * 3.0 + 2.0});
                    }
                } else if (entity->getName() == "bat")
                {
                    _Particles.setPos(entity->getCenter());
                    _Smoke.setPos(entity->getCenter());
                    _Fire.setPos(entity->getCenter());
                    _Particles.setSpawning(16, {8.0, 4.0}, SDL_Color{0x00, 0x00, 0x00});
                    _Smoke.setSpawning(5, {1, 2}, {0x88, 0x88, 0x88});
                    _Fire.setSpawning(10);
                    int num{(std::rand() % 10) + 15};
                    for (int i{0}; i < num; ++i)
                    {
                        _SparkManager.addSpark(new Spark{entity->getCenter(), Util::random() * M_PI * 2.0, Util::random() * 2.0 + 1.0});
                    }
                }
                int num{(std::rand() % 10) + 5};
                for (int i{0}; i < num; ++i)
                {
                    coinmanager->addCoin(entity->getCenter(), {Util::random() * 2.0 - 1.0, Util::random() * -2.0});
                }
                Util::swap(&_Entities[i], &_Entities[_total - 1]); // swap dead entity with last entity in the array
                delete _Entities[_total - 1]; // deallocate dead entity
                _Entities[_total - 1] = nullptr;
                --_total; // deincrement total to avoid undefined behaviour when we reference nullptr
            } else if (entity->getShouldDamage())
            {
                texman->playDamageSound();
                entity->setShouldDamage(false);
                if (entity->getName() == "turtle")
                {
                    int num{(std::rand() % 5) + 10};
                    for (int i{0}; i < num; ++i)
                    {
                        _SparkManager.addSpark(new Spark{entity->getCenter(), Util::random() * M_PI * 2.0, Util::random() * 2.0 + 0.5});
                    }
                    texman->SFX_turtle.play();
                    _Particles.setPos(entity->getCenter());
                    _Particles.setSpawning(16, {8.0, 8.0}, SDL_Color{0x00, 0x00, 0x00});
                }
                if (entity->getName() == "slime")
                {
                    int num{(std::rand() % 5) + 10};
                    for (int i{0}; i < num; ++i)
                    {
                        _SparkManager.addSpark(new Spark{entity->getCenter(), Util::random() * M_PI * 2.0, Util::random() * 3.0 + 1.0});
                    }
                    _Particles.setPos(entity->getCenter());
                    _Particles.setSpawning(16, {3.0, 10.0}, SDL_Color{0x00, 0x00, 0x00});
                }
                if (entity->getName() == "bat")
                {
                    int num{(std::rand() % 5) + 6};
                    for (int i{0}; i < num; ++i)
                    {
                        _SparkManager.addSpark(new Spark{entity->getCenter(), Util::random() * M_PI * 2.0, Util::random() * 2.0 + 0.5});
                    }
                    _Particles.setPos(entity->getCenter());
                    _Particles.setSpawning(8, {4.0, 4.0}, SDL_Color{0x00, 0x00, 0x00});
                }
            }
        }
    }
    //std::cout << _total << '\n';
}

void EntityManager::render(const int scrollX, const int scrollY, SDL_Renderer* renderer)
{
    const int num{_total};
    for (std::size_t i{0}; i < num; ++i)
    {
        if (i < _total)
        {
            Entity* entity {_Entities[i]};
            entity->render(scrollX, scrollY, renderer);
        }
    }
}

void EntityManager::updateParticles(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, World* world, TexMan* texman)
{
    _Particles.update(time_step, scrollX, scrollY, renderer, world, texman);
    _Smoke.update(time_step, scrollX, scrollY, renderer, world, &texman->particle);
    _Fire.update(time_step, scrollX, scrollY, renderer, world, &texman->particleFire);
    _SparkManager.setTexture(&(texman->particle));
    _SparkManager.update(time_step, scrollX, scrollY, renderer);
}

// "Manager of the Managers" Entity-Manager-Manager
EMManager::EMManager()
{
}

EMManager::~EMManager()
{
    free();
}

void EMManager::free()
{
    for (std::size_t i{0}; i < _Managers.size(); ++i)
    {
        _Managers[i]->free();
        delete _Managers[i];
    }
    _Managers.clear();
}

void EMManager::loadFromPath(std::string path, TexMan* texman)
{
    std::ifstream f{path};
    json data = json::parse(f);

    std::vector<std::vector<Entity*>> entities;
    for (const auto& e : data["level"]["entities"])
    {
        std::string entity_name {e["type"]};
        bool found_entity{false};
        for (auto& entity_vec : entities)
        {
            if (!found_entity)
            {
                for (Entity* entity : entity_vec)
                {
                    if (entity->getName() == entity_name)
                    {
                        found_entity = true;
                        if (entity_name == "slime")
                        {
                            entity_vec.push_back(new Slime{vec2<double>{(double)e["pos"][0], (double)e["pos"][1]}, vec2<double> {0, 0}, 0.2, false, entity_name, texman});
                        } else if (entity_name == "bat")
                        {
                            entity_vec.push_back(new Bat{vec2<double>{(double)e["pos"][0], (double)e["pos"][1]}, vec2<double> {0, 0}, 0.2, false, entity_name, texman});
                        } else if (entity_name == "turtle")
                        {
                            entity_vec.push_back(new Turtle{vec2<double>{(double)e["pos"][0], (double)e["pos"][1]}, vec2<double> {0, 0}, 0.2, true, entity_name, texman});
                        } else {
                            entity_vec.push_back(new Entity{vec2<double>{(double)e["pos"][0], (double)e["pos"][1]}, vec2<double> {0, 0}, 0.2, false, "default"});
                        }
                        break;
                    }
                }
            }
        }
        if (!found_entity)
        {
            if (entity_name == "slime")
            {
                entities.push_back(std::vector<Entity*>{new Slime{vec2<double>{(double)e["pos"][0], (double)e["pos"][1]}, vec2<double> {0, 0}, 0.2, false, entity_name, texman}});
            } else if (entity_name == "bat")
            {
                entities.push_back(std::vector<Entity*>{new Bat{vec2<double>{(double)e["pos"][0], (double)e["pos"][1]}, vec2<double> {0, 0}, 0.2, false, entity_name, texman}});
            } else if (entity_name == "turtle")
            {
                entities.push_back(std::vector<Entity*>{new Turtle{vec2<double>{(double)e["pos"][0], (double)e["pos"][1]}, vec2<double> {0, 0}, 0.2, true, entity_name, texman}});
            } else {
                entities.push_back(std::vector<Entity*>{new Entity{vec2<double>{(double)e["pos"][0], (double)e["pos"][1]}, vec2<double> {0, 0}, 0.2, false, "default"}});
            }
        }
    }
    
    for (EntityManager* manager : _Managers)
    {
        manager->free();
        delete manager;
    }
    _Managers.clear();

    for (auto& entity_vec : entities)
    {
        _Managers.push_back(new EntityManager{vec2<double>{0, 0}, static_cast<int>(entity_vec.size()), entity_vec});
    }

    f.close();
}

void EMManager::addEntity(Entity* entity)
{
    for (std::size_t i{0}; i < _Managers.size(); ++i)
    {
        if (_Managers[i]->getName() == entity->getName())
        {
            _Managers[i]->addEntity(entity);
            return; // break;
        }
    }
    // we didn't find anything
    _Managers.push_back(new EntityManager{vec2<double>{0, 0}, 1, std::vector<Entity*>{entity}});
}

void EMManager::update(const double& time_step, World& world, double* screen_shake, Player* player, double* slomo, TexMan* texman, CoinManager* coinmanager)
{
    for (std::size_t i{0}; i < _Managers.size(); ++i)
    {
        _Managers[i]->update(time_step, world, screen_shake, player, slomo, texman, coinmanager);
    }
}
// updateParticles(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, World* world, TexMan* texman)
void EMManager::render(const int scrollX, const int scrollY, SDL_Renderer* renderer, const double& time_step, World* world, TexMan* texman)
{
    for (std::size_t i{0}; i < _Managers.size(); ++i)
    {
        _Managers[i]->render(scrollX, scrollY, renderer);
        _Managers[i]->updateParticles(time_step, scrollX, scrollY, renderer, world, texman);
    }
}

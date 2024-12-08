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

void Entity::die(double *screen_shake)
{
    //if (!_should_die)
        //*screen_shake = std::max(*screen_shake, 8.0);
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
    SDL_Rect renderRect{_pos.x - scrollX, _pos.y - scrollY, _dimensions.x, _dimensions.y};
    if (_recover < _recover_time)
    {
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    }
    else
    {
        SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
    }

    SDL_RenderFillRect(renderer, &renderRect);
}

void Entity::touchPlayer(Player *player, double *screen_shake)
{
    _rect.x = _pos.x;
    _rect.y = _pos.y;
    SDL_Rect *player_rect{player->getRect()};
    if (Util::checkCollision(player_rect, &_rect))
    {
        player->damage(_damage, screen_shake);
    }
}

void Entity::followPlayer(Player *player, World *world)
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
                _vel.x += _flipped ? 0.2 : -0.2;
                _flipped = !_flipped;
            }
        }
        else if (Util::elementIn<TileType, std::size(DANGER_TILES)>(tile->type, DANGER_TILES))
        {
            if (!(std::abs(static_cast<int>(player_pos.x - getCenter().x)) < 16 && player_pos.y > getCenter().y + 4.0 && _falling < 3.0))
            {
                _vel.x += _flipped ? 0.2 : -0.2;
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
            _vel.x += _flipped ? -0.1 : 0.1;
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
        wander(world);
    }
}

void Entity::wander(World *world)
{
    if (_wandering)
    {
        vec2<double> checkTilePos{getCenter().x + (_flipped ? -10.0 : 10.0), getCenter().y + 8};
        Tile *tile{world->getTileAt(checkTilePos.x, checkTilePos.y)};
        if (tile == nullptr)
        {
            _vel.x += _flipped ? 0.2 : -0.2;
            _flipped = !_flipped;
        }
        else if (Util::elementIn<TileType, std::size(DANGER_TILES)>(tile->type, DANGER_TILES))
        {
            _vel.x += _flipped ? 0.2 : -0.2;
            _flipped = !_flipped;
        }
        else
        {
            vec2<double> checkTilePos{getCenter().x + (_flipped ? -10.0 : 10.0), getCenter().y};
            Tile *tile{world->getTileAt(checkTilePos.x, checkTilePos.y)};
            if (tile != nullptr)
            {
                _flipped = !_flipped;
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
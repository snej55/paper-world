#include "./player.hpp"

#include <cmath>

Player::Player(vec2<double> pos, vec2<int> dimensions)
    : _pos{pos}, _dimensions{dimensions}, _spawn_pos{pos}
{
    _rect.x = _pos.x;
    _rect.y = _pos.y;
    _rect.w = _dimensions.x;
    _rect.h = _dimensions.y;
    _Particles.setPalette<5>(_Palette);
}

Player::~Player()
{
    delete _idleAnim;
    delete _runAnim;
    delete _jumpAnim;
    delete _landAnim;
}

void Player::loadAnim(TexMan* texman)
{
    _idleAnim = new Anim{8, 8, 5, 0.16, true, &(texman->playerIdle)};
    _runAnim = new Anim{8, 8, 5, 0.3, true, &(texman->playerRun)};
    _jumpAnim = new Anim{8, 8, 2, 0.2, true, &(texman->playerJump)};
    _landAnim = new Anim{8, 8, 5, 0.2, false, &(texman->playerLand)};
    _flash = &(texman->playerFlash);
    _anim = _idleAnim;
}

SDL_Rect* Player::getRect()
{
    return &_rect;
}

Controller* Player::getController()
{
    return &_Controller;
}

vec2<double>& Player::getPos()
{
    return _pos;
}

vec2<double> Player::getCenter()
{
    return {_pos.x + _dimensions.x / 2.0, _pos.y + _dimensions.y / 2.0};
}

void Player::damage(double amount, double* screen_shake)
{
    // we can add buffs later
    if (_recover > _recover_time + 30)
    {
        *screen_shake = std::max(*screen_shake, 8.0);
        _health -= amount;
        _Particles.setPos(getCenter());
        _Smoke.setPos(getCenter());
        _Fire.setPos(getCenter());
        _Particles.setSpawning(16, {4.0, 4.0}, _Palette[0]);
        _Smoke.setSpawning(10, {1, 2}, {0x88, 0x88, 0x88});
        _recover = 0.0;
        if (_health < 0.0)
        {
            die(screen_shake);
        }
    }
}

void Player::die(double* screen_shake)
{
    _health = _max_health;
    _ad = 0;
    _Particles.setPos(getCenter());
    _Smoke.setPos(getCenter());
    _Fire.setPos(getCenter());
    _Particles.setSpawning(128, {16.0, 8.0}, _Palette[0]);
    _Smoke.setSpawning(100, {1, 2}, {0xAA, 0xAA, 0xAA});
    _Fire.setSpawning(100);
    _pos = _spawn_pos;
    _rect.x = _pos.x;
    _rect.y = _pos.y;
    *screen_shake = std::max(*screen_shake, 16.0);
}

void Player::update(const double& time_step, World& world, double* screen_shake)
{
    _falling += time_step;
    _Controller.update(time_step);

    if (_ad > 120)
    {
        updateVel(time_step);
        handlePhysics(time_step, _vel, world, screen_shake);
        handleAnim(time_step);
    }
}

void Player::updateVel(const double& time_step)
{
    // x velocity
    if (_falling > 3.0)
    {
        _friction = 0.55;
    } else {
        _friction = 0.6;
    }
    _vel.x *= _friction;
    if (_Controller.getControl(Control::LEFT))
    {
        _flipped = true;
        _vel.x -= 1.1;
    }
    if (_Controller.getControl(Control::RIGHT))
    {
        _flipped = false;
        _vel.x += 1.1;
    }
    // y velocity
    if (_vel.y >= -1.0 && _vel.y <= 0.0)
    {
        _gravity = 0.2;
    } else if (_vel.y < 0)
    {
        _gravity = 0.22;
    } else {
        _gravity = 0.25;
    }
    _vel.y += _gravity * time_step;
    if (_Controller.getJumping() < 10.0)
    {
        if (_falling < 9.0)
        {
            _vel.y = -3.2;
            _falling = 99.0;
            _Controller.setJumping(99.0f);
        }
    }
    _vel.y = std::min(8.0, _vel.y);
}

void Player::handlePhysics(const double& time_step, vec2<double> frame_movement, World& world, double* screen_shake)
{
    _pos.x += frame_movement.x * time_step;
    _rect.x = _pos.x;
    _rect.y = _pos.y;

    std::array<SDL_Rect, 9> rects;
    world.getTilesAroundPos(_pos, rects);
    for (int i{0}; i < 9; ++i)
    {
        SDL_Rect* tile_rect {&(rects[i])};
        if (Util::checkCollision(&_rect, tile_rect))
        {
            // moving right
            if (frame_movement.x > 0)
            {
                _rect.x = tile_rect->x - _rect.w;
            } else { // moving left
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
        SDL_Rect* tile_rect {&(rects[i])};
        if (Util::checkCollision(&_rect, tile_rect))
        {
            // moving down
            if (frame_movement.y > 0)
            {
                _rect.y = tile_rect->y - _rect.h;
                _falling = 0.0;
            } else { // moving left
                _rect.y = tile_rect->y + tile_rect->h;
            }
            _vel.y = 0.0;
            _pos.y = _rect.y;
        }
    }

    _rect.x = _pos.x;
    _rect.y = _pos.y;
    std::vector<Spring*>& springs {world.getSprings()};
    for (Spring* spring : springs)
    {
        if (Util::checkCollision(&_rect, spring->getRect()))
        {
            _vel.y = -5;
            spring->setVel(4.0);
            break;
        }
    }

    // check for danger
    world.getDangerAroundPos(_pos, rects);
    for (int i{0}; i < 9; ++i)
    {
        SDL_Rect* tile_rect {&(rects[i])};
        if (Util::checkCollision(&_rect, tile_rect))
        {
            // we died
            die(screen_shake);
            break;
        }
    }
}

void Player::updateParticles(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, World* world, TexMan* texman)
{
    _Particles.update(time_step, scrollX, scrollY, renderer, world, texman);
    _Smoke.update(time_step, scrollX, scrollY, renderer, world, &texman->particle);
    _Fire.update(time_step, scrollX, scrollY, renderer, world, &texman->particleFire);
}

void Player::handleAnim(const double& time_step)
{
    if (_falling > 3.0)
    {
        _anim = _jumpAnim;
        _runAnim->reset();
        _idleAnim->reset();
        _landAnim->reset();
        _grounded = 99.9;
    } else if (_Controller.getControl(Control::RIGHT) || _Controller.getControl(Control::LEFT))
    {
        _anim = _runAnim;
        _jumpAnim->reset();
        _idleAnim->reset();
        _landAnim->reset();
        _grounded = 0.0;
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
    _anim->setFlipped(_flipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void Player::render(const int scrollX, const int scrollY, SDL_Renderer* renderer)
{
    // SDL_Rect renderRect{_pos.x - scrollX, _pos.y - scrollY, _dimensions.x, _dimensions.y};
    // if (_recover < _recover_time)
    // {
    //     SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    // } else {
    //     SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF);
    // }

    // SDL_RenderFillRect(renderer, &renderRect);
    if (_recover < _recover_time)
    {
        _flash->render((int)_pos.x - 2 - scrollX, (int)_pos.y - scrollY, renderer, 0, NULL, _flipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE, NULL);    
    } else {
        _anim->render((int)_pos.x - 2, (int)_pos.y, scrollX, scrollY, renderer);
    }
}

void Controller::setControl(Control control, const bool val)
{
    switch (control)
    {
        case Control::UP:
            _control_up = val;
            return;
        case Control::DOWN:
            _control_down = val;
            return;
        case Control::LEFT:
            _control_left = val;
            return;
        case Control::RIGHT:
            _control_right = val;
            return;
        default:
            return;
    }
}

bool const Controller::getControl(Control control)
{
    switch (control)
    {
        case Control::UP:
            return _control_up;
        case Control::DOWN:
            return _control_down;
        case Control::LEFT:
            return _control_left;
        case Control::RIGHT:
            return _control_right;
        default:
            return false;
    }
}

void Controller::update(const double& time_step)
{
    _jumping += time_step;
}
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
    // _Sword = new Sword{_pos, this};
}

Player::~Player()
{
    free();
}

void Player::free()
{
    delete _idleAnim;
    delete _runAnim;
    delete _jumpAnim;
    delete _landAnim;
    if (_Sword != nullptr)
    {
        delete _Sword;
    }
    if (_Slash != nullptr)
    {
        delete _Slash;
    }
}

void Player::reset()
{
    _Controller.setControl(Control::UP, false);
    _Controller.setControl(Control::DOWN, false);
    _Controller.setControl(Control::RIGHT, false);
    _Controller.setControl(Control::LEFT, false);
}

void Player::loadAnim(TexMan* texman)
{
    _idleAnim = new Anim{8, 8, 5, 0.16, true, &(texman->playerIdle)};
    _runAnim = new Anim{8, 8, 5, 0.3, true, &(texman->playerRun)};
    _jumpAnim = new Anim{8, 8, 2, 0.2, true, &(texman->playerJump)};
    _landAnim = new Anim{8, 8, 5, 0.2, false, &(texman->playerLand)};
    _flash = &(texman->playerFlash);
    _anim = _idleAnim;
    _Sword = new Sword(this, texman);
}

SDL_Rect* Player::getRect()
{
    return &_rect;
}

SDL_Rect Player::getAttackRect()
{
    return {static_cast<int>(_flipped ? getCenter().x - 12 : getCenter().x), static_cast<int>(_pos.y) - 6, 12, 16};
}

bool Player::getAttacking()
{
    return _swordAttacking;
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

void Player::damage(double amount, double* screen_shake, double* slomo, ShockWaveManager& shockwaves)
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
        int num{(std::rand() % 10) + 15};
        for (int i{0}; i < num; ++i)
        {
            _SparkManager.addSpark(new Spark{getCenter(), Util::random() * M_PI * 2.0, Util::random() * 3.0 + 1.0});
        }
        //_Smoke.setSpawning(2, {1, 2}, {0x88, 0x88, 0x88});
        _recover = 0.0;
        *slomo = std::min(0.7, *slomo);
        if (_health < 0.0)
        {
            *slomo = std::min(0.5, *slomo);
            die(screen_shake, shockwaves);
        }
        _should_damage = true;
    }
}

void Player::die(double* screen_shake, ShockWaveManager& shockwaves)
{
    _last_pos = _pos;
    _health = _max_health;
    _ad = 0;
    _Particles.setPos(getCenter());
    _Smoke.setPos(getCenter());
    _Fire.setPos(getCenter());
    _Particles.setSpawning(128, {16.0, 8.0}, _Palette[0]);
    _Smoke.setSpawning(100, {1, 2}, {0xAA, 0xAA, 0xAA});
    _Fire.setSpawning(100);
    shockwaves.addShockWave(getCenter());
    int num{(std::rand() % 20) + 10};
    for (int i{0}; i < num; ++i)
    {
        _SparkManager.addSpark(new Spark{getCenter(), Util::random() * M_PI * 2.0, Util::random() * 5.0 + 3.0});
    }
    _pos = _spawn_pos;
    _rect.x = _pos.x;
    _rect.y = _pos.y;
    *screen_shake = std::max(*screen_shake, 16.0);
}

void Player::update(const double& time_step, World& world, double* screen_shake, TexMan* texman, ShockWaveManager& shockwaves)
{
    // add all the timers here
    _swordAttacked += time_step;
    _falling += time_step;
    _in_water += time_step;
    _Controller.update(time_step);

    if (_ad > 120)
    {
        updateVel(time_step);
        handlePhysics(time_step, _vel, world, screen_shake, texman, shockwaves);
        // updateSword(time_step);
        handleAnim(time_step);
        updateRect();
        if (_Slash != nullptr)
        {
            _Slash->update(time_step);
            if (_Slash->getFinished())
            {
                _swordAttacking = false;
                delete _Slash;
                _Slash = nullptr;
            }
        }
        if (_Sword != nullptr)
        {
            _Sword->update(time_step);
            if (_swordAttacked > 120.0)
            {
                _slashVFLIP = false;
                _Sword->setUp(true);
            }
        }
    }
}

void Player::updateVel(const double& time_step)
{
    // x velocity
    if (_falling > 3.0)
    {
        _friction = 0.6;
    } else {
        _friction = 0.7;
    }
    _vel.x *= _friction;
    if (_Controller.getControl(Control::LEFT))
    {
        _flipped = true;
        _vel.x -= 0.7;
    }
    if (_Controller.getControl(Control::RIGHT))
    {
        _flipped = false;
        _vel.x += 0.7;
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

void Player::handlePhysics(const double& time_step, vec2<double> frame_movement, World& world, double* screen_shake, TexMan* texman, ShockWaveManager& shockwaves)
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
                if (_vel.y > 5.0)
                {
                    _Particles.setPos(getCenter());
                    _Particles.setSpawning(16, {4.0, 5.0}, _Palette[0]);
                    texman->SFX_landing.play();
                }
            } else { // moving up
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
            texman->SFX_spring.play();
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
            die(screen_shake, shockwaves);
            return;
        }
    }
    if (_lava_struck)
    {
        die(screen_shake, shockwaves);
    }
}

void Player::updateParticles(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, World* world, TexMan* texman)
{
    _Particles.update(time_step, scrollX, scrollY, renderer, world, texman);
    _Smoke.update(time_step, scrollX, scrollY, renderer, world, &texman->particle);
    _Fire.update(time_step, scrollX, scrollY, renderer, world, &texman->particleFire);
    _SparkManager.setTexture(&(texman->particle));
    _SparkManager.update(time_step, scrollX, scrollY, renderer);
    if (_should_damage)
    {
        texman->SFX_player_hit.play();
        _should_damage = false;
    }
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
    if (_Sword != nullptr)
    {
        if (_Sword->getUp())
        {
            renderSword(scrollX, scrollY, renderer);
            renderPlayer(scrollX, scrollY, renderer);
        } else {
            renderPlayer(scrollX, scrollY, renderer);
            renderSword(scrollX, scrollY, renderer);
        }
    } else {
        renderPlayer(scrollX, scrollY, renderer);
    }
    renderSlash(scrollX, scrollY, renderer);
}

void Player::renderPlayer(const int scrollX, const int scrollY, SDL_Renderer* renderer)
{
    if (_recover < _recover_time)
    {
        _flash->render((int)_pos.x - 2 - scrollX, (int)_pos.y - scrollY, renderer, 0, NULL, _flipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE, NULL);    
    } else {
        _anim->render((int)_pos.x - 2, (int)_pos.y, scrollX, scrollY, renderer);
    }
}

void Player::renderSword(const int scrollX, const int scrollY, SDL_Renderer* renderer)
{
    if (_Sword != nullptr)
    {
        _Sword->render(scrollX, scrollY, renderer);
    }
}

void Player::renderSlash(const int scrollX, const int scrollY, SDL_Renderer* renderer)
{
    if (_Slash != nullptr)
    {
        _Slash->render(scrollX, scrollY, renderer, this);
    }
}

void Player::attackSword(TexMan* texman)
{
    if (!_swordAttacking)
    {
        _Slash = new Slash{_slashVFLIP, _flipped, {(_flipped ? -13.0 : -3.0), -10.0}, texman};
        _swordAttacking = true;
        _slashVFLIP = !_slashVFLIP;
        _swordAttacked = 0.0;
        _Sword->flipUp();
        texman->SFX_sword_swoosh.play();
    }
}

// void Player::updateSword(const double& time_step)
// {
//     _Sword->update(time_step);
// }

// void Player::renderSword(const int scrollX, const int scrollY, SDL_Renderer* renderer, TexMan* texman)
// {
//     _Sword->render(scrollX, scrollY, renderer, texman);
// }

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
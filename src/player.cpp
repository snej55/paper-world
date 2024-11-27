#include "./player.hpp"

#include <cmath>

Player::Player(vec2<double> pos, vec2<int> dimensions)
    : _pos{pos}, _dimensions{dimensions}
{
    _rect.x = _pos.x;
    _rect.y = _pos.y;
    _rect.w = _dimensions.x;
    _rect.h = _dimensions.y;
}

SDL_Rect* Player::getRect()
{
    return &_rect;
}

Controller* Player::getController()
{
    return &_Controller;
}

void Player::update(const double& time_step, World& world)
{
    _falling += time_step;
    _Controller.update(time_step);

    updateVel(time_step);
    handlePhysics(time_step, _vel, world);
}

void Player::updateVel(const double& time_step)
{
    // x velocity
    _vel.x *= _friction;
    if (_Controller.getControl(Control::LEFT))
    {
        _vel.x -= 1.5;
    }
    if (_Controller.getControl(Control::RIGHT))
    {
        _vel.x += 1.5;
    }
    // y velocity
    if (_vel.y < 0)
    {
        _gravity = 0.25;
    } else {
        _gravity = 0.3;
    }
    _vel.y += _gravity * time_step;
    if (_Controller.getJumping() < 10.0)
    {
        if (_falling < 10.0)
        {
            _vel.y = -3.5;
            _falling = 99.0;
            _Controller.setJumping(99.0f);
        }
    }
}

void Player::handlePhysics(const double& time_step, vec2<double> frame_movement, World& world)
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
            _vel.x = 0;
        }
        _pos.x = _rect.x;
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
        }
        _pos.y = _rect.y;
    }
}

void Player::render(const int scrollX, const int scrollY, SDL_Renderer* renderer)
{
    SDL_Rect renderRect{_pos.x - scrollX, _pos.y - scrollY, _dimensions.x, _dimensions.y};
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF);
    SDL_RenderFillRect(renderer, &renderRect);
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
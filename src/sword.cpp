#include "./sword.hpp"
#include "./player.hpp"

Sword::Sword(vec2<double> pos, void* player)
 : _pos{pos}, _Player{(Player*)player}
{
}

void Sword::attack()
{
    if (_target_dir == -M_PI * 0.25)
    {
        _target_dir = M_PI * 0.75;
    } else {
        _target_dir = -M_PI * 0.25;
    }
    if (_target_turn == 90.0)
    {
        _target_turn = 200.0;
    } else {
        _target_turn = 90.0;
    }
    _attacking = true;
    _attacked = 0;
    _damp = 0.6;
}

void Sword::update(const double& time_step)
{
    _attacked += time_step;
    _pos = static_cast<Player*>(_Player)->getCenter();
    if (!_attacking)
    {
        _target_dir = -M_PI * 0.25;
        _target_turn = 90.0;
    } else {
        if (_angle + _angle_offset > _target_dir)
        {
            _damp = 0.5;
        }
        if (_attacked > 40.0)
        {
            _attacking = false;
            _attacked = 4.0;
            _damp = 0.4;
        }
    }
    double force {(-_target_dir - _angle) * 0.3};
    _swing_vel += force * time_step;
    _angle += _swing_vel * time_step;
    if (_flipped)
    {
        _pos.x += -std::cos(-_angle) * _arm_length; // NOTE: - sign 
    } else {
        _pos.x += std::cos(-_angle) * _arm_length;
    }
    _pos.y += std::sin(-_angle) * _arm_length;
    _angle_offset = 90.0 + (_target_turn - 90.0) * _angle / _target_dir;
    _swing_vel += (_swing_vel * _damp - _swing_vel) * time_step;
}

void Sword::render(const int scrollX, const int scrollY, SDL_Renderer* renderer, TexMan* texman)
{
    _flipped = false;
    vec2<double> offset {_offset};
    if (static_cast<Player*>(_Player)->getFlipped())
    {
        _flipped = true;
        offset.x -= 3;
    }
    SDL_Point center {3, 8};
    texman->swordBase.render(static_cast<int>(_pos.x + offset.x) - scrollX, static_cast<int>(_pos.y + offset.y) - scrollY, renderer, _angle - 90.0 + _angle_offset, &center, (_flipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));
}
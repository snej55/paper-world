#include "./sword.hpp"

Sword::Sword(vec2<double> pos, Player* player)
 : _pos{pos}, _Player{player}
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
    _pos = _Player->getCenter();
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
    double force {-_target_dir - _angle} * 0.3;
    //
}
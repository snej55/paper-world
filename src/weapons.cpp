#include "./weapons.hpp"
#include "./player.hpp"

// -------------------------------------------------- Slash -------------------------------------------------- //

Slash::Slash(bool vflip, bool flip, vec2<double> offset, TexMan* texman)
 : _vflip{vflip}, _flip{flip}, _offset{offset}
{
    loadAnim(texman);
}

Slash::~Slash()
{
    delete _anim;
}

void Slash::loadAnim(TexMan* texman)
{
    _anim = new Anim(16, 16, 15, 1.5, false, &(texman->slash));
}

void Slash::update(const double& time_step)
{
    _anim->tick(time_step);
    _finished = _anim->getFinished();
}

void Slash::render(const int scrollX, const int scrollY, SDL_Renderer* renderer, void* target)
{
    Player* player {(Player*)target}; // cast target to player, it has to be void* as "Player*" is not yet defined int slash.hpp
    vec2<double> render_pos {player->getCenter()};
    render_pos.x += _offset.x;
    render_pos.y += _offset.y;
    _flip = player->getFlipped();
    if (_vflip && _flip)
    {
        _anim->setAngle(180.0);
    } else {
        _anim->setFlipped(_vflip ? SDL_FLIP_VERTICAL : _anim->getFlipped());
        _anim->setFlipped(_flip ? SDL_FLIP_HORIZONTAL : _anim->getFlipped());
    }
    _anim->render((int)render_pos.x, (int)render_pos.y, scrollX, scrollY, renderer);
}

// -------------------------------------------------- Sword -------------------------------------------------- //

Sword::Sword(void* player, TexMan* texman)
 : _Player{static_cast<Player*>(player)}
{
    loadTex(texman);
}

void Sword::loadTex(TexMan* texman)
{
    _tex = &(texman->swordBase);
}

void Sword::update(const double& time_step)
{
    _pos = static_cast<Player*>(_Player)->getCenter();
    _target_angle = (_up ? M_PI * 0.25 : M_PI * 1.7) + M_PI;
    _angle_vel += (_target_angle - _angle) * 0.3 * time_step;
    const double max_speed{0.5};
    _angle += std::max(-max_speed, std::min(_angle_vel, max_speed)) * time_step;
    _angle_vel += (_angle_vel * 0.5 - _angle_vel) * time_step;
    if (_flipped)
    {
        _pos.x += -std::cos(_angle) * _arm_length;
        _pos.y += std::sin(_angle) * _arm_length;
    } else {
        _pos.x += std::cos(_angle) * _arm_length;
        _pos.y += std::sin(_angle) * _arm_length;
    }
}

void Sword::render(const int scrollX, const int scrollY, SDL_Renderer* renderer)
{
    int angle {static_cast<int>(_angle * 180 / M_PI + (-90.0 + 180.0 * (_angle - M_PI * 0.25) / (M_PI * 1.7)))};
    _flipped = static_cast<Player*>(_Player)->getFlipped();
    SDL_Point center {3, 8};
    _tex->render(static_cast<int>(_pos.x) - scrollX - (_flipped ? 4 : 3), static_cast<int>(_pos.y) - scrollY - 8, renderer, angle * (_flipped ? -1 : 1), &center, (_flipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE));
}
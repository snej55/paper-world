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
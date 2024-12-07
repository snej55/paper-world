#include "./anim.hpp"

Anim::Anim(int w, int h, int length, double speed, bool loop, Texture* texture)
 : _width{w}, _height{h}, _length{length}, _speed{speed}, _loop{loop}, _texture{texture}
{
}

Anim::~Anim()
{
}

void Anim::setTex(Texture* texture)
{
    _texture = texture;
}

void Anim::reset()
{
    _finished = false;
    _frame = 0.0;
    _step = 0;
}

void Anim::tick(const double& time_step)
{
    _frame += _speed * time_step;
    if (!_loop)
    {
        if (_frame > (double)_length)
        {
            _finished = true;
        }
        // cap frame to last anim. frame
        _step = std::min(static_cast<int>(_frame), _length - 1);
    } else {
        _step = static_cast<int>(_frame) % _length;
    }
}

void Anim::render(int x, int y, const int scrollX, const int scrollY, SDL_Renderer* renderer)
{
    SDL_Rect clip_rect {_step * _width, 0, _width, _height}; // all animation spritesheets are single strips
    _texture->render(x - scrollX, y - scrollY, renderer, _angle, _center, _flipped, &clip_rect);
}
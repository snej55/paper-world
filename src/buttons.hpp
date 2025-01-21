#ifndef BUTTONS_H
#define BUTTONS_H

#include "./texman.hpp"
#include "./vec2.hpp"
#include "./util.hpp"

class Button
{
private:
    vec2<int> _pos;
    Texture* _tex;
    SDL_Rect _rect{};
    double _expand{0.0};
    double _vel{0.0};
    double _flash{0.0};

public:
    Button(vec2<int> pos, Texture* tex)
     : _pos{pos}, _tex{tex}
    {
        _rect = SDL_Rect{_pos.x, _pos.y, _tex->getWidth(), _tex->getHeight()};
    }

    void setExpand(double val)
    {
        _expand = val;
    }

    void setPos(vec2<int> pos)
    {
        _pos = pos;
        _rect = SDL_Rect{_pos.x, _pos.y, _tex->getWidth(), _tex->getHeight()};
    }

    SDL_Rect* getRect()
    {
        return &_rect;
    }

    void update(const double& time_step, int mouseX, int mouseY, SDL_Renderer* renderer)
    {
        SDL_Rect mouseRect {mouseX, mouseY, 1, 1};
        if (Util::checkCollision(&mouseRect, &_rect))
        {
            _vel = (8.0 - _expand) * 0.8 * time_step;
            _tex->setColor(255, 255, 255);
        } else {
            _tex->setColor(220, 220, 220);
            _vel += (_vel * 0.7 - _vel) * 0.7 * time_step;
            _vel += (0.0 - _expand) * time_step;
        }
        _expand += _vel * 0.4 * time_step;
        
        SDL_Rect renderQuad{_pos.x - static_cast<int>(_expand), _pos.y - static_cast<int>(_expand), _tex->getWidth() + static_cast<int>(_expand) * 2, _tex->getHeight() + static_cast<int>(_expand) * 2};
        SDL_RenderCopyEx(renderer, _tex->getTexture(), NULL, &renderQuad, 0, NULL, SDL_FLIP_NONE);
    }
};

#endif
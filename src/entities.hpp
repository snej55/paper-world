#ifndef ENTITIES_H
#define ENTITIES_H

#include <SDL2/SDL.h>

#include "./util.hpp"
#include "./vec2.hpp"
#include "./tiles.hpp"

class Entity
{
private:
    vec2<double> _pos;
    vec2<double> _vel;
    vec2<int> _dimensions;
    double _gravity;
    bool _peaceful;

    double _falling{99.0};
    double _friction{0.8};

    SDL_Rect _rect{0, 0, 0, 0};
    bool _should_die {false};

public:
    Entity(vec2<double> pos, vec2<double> vel, vec2<int> dimensions, double gravity, bool peaceful)
     : _pos{pos}, _vel{vel}, _dimensions{dimensions}, _gravity{gravity}, _peaceful{peaceful}
    { 
        _rect.x = pos.x;
        _rect.y = pos.y;
        _rect.w = dimensions.x;
        _rect.h = dimensions.y;
    }

    vec2<double>& getPos() {return _pos;}
    vec2<double> getCenter() {return {_pos.x + _dimensions.x / 2.0, _pos.y + _dimensions.y / 2.0};}
    SDL_Rect* getRect() {return &_rect;}
    bool getShouldDie() {return _should_die;}

    virtual void die(double* screen_shake)
    {
        *screen_shake = std::max(*screen_shake, 8.0);
    }

    virtual void update(const double& time_step, World& world, double* screen_shake)
    {
        _falling += time_step;

        if (!_should_die)
        {
            updateVel(time_step);
            handlePhysics(time_step, _vel, world, screen_shake);
        }
    }

    virtual void updateVel(const double& time_step)
    {
        _vel.x *= _friction;
        _vel.y += _gravity * time_step;
    }

    virtual void handlePhysics(const double& time_step, vec2<double> frame_movement, World& world, double* screen_shake)
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
                if (frame_movement.x > 0)
                {
                    _rect.x = tile_rect->x - _rect.w;
                } else {
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
                if (frame_movement.y > 0)
                {
                    _rect.y = tile_rect->y - _rect.h;
                    _falling = 0.0;
                } else {
                    _rect.y = tile_rect->y + tile_rect->h;
                }
                _vel.y = 0.0;
                _pos.y = _rect.y;
            }
        }

        world.getDangerAroundPos(_pos, rects);
        for (int i{0}; i < 9; ++i)
        {
            SDL_Rect* tile_rect{&(rects[i])};
            if (Util::checkCollision(&_rect, tile_rect))
            {
                die(screen_shake);
                break;
            }
        }
    }

    virtual void render(const int scrollX, const int scrollY, SDL_Renderer* renderer)
    {
        SDL_Rect renderRect{_pos.x - scrollX, _pos.y - scrollY, _dimensions.x, _dimensions.y};
        SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
        SDL_RenderFillRect(renderer, &renderRect);
    }
};

#endif
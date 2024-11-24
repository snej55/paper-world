#ifndef UTIL_H
#define UTIL_H

#include "./lib/SDL2/SDL.h"

namespace Util {
    inline bool checkCollision(const SDL_Rect* rect_0, const SDL_Rect* rect_1)
    {
        return (rect_0->x < rect_1->x + rect_1->w && rect_0->x + rect_0->w > rect_1->x && rect_0->y + rect_0->h > rect_1->y && rect_0->y < rect_1->y + rect_1->h);
    }
}

#endif

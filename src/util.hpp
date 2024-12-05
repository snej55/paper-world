#ifndef UTIL_H
#define UTIL_H

#include <SDL2/SDL.h>
#include <array>
#include <cstdlib>
#include <cmath>

#include "./vec2.hpp"

namespace Util {
    inline bool checkCollision(const SDL_Rect* rect_0, const SDL_Rect* rect_1)
    {
        return (rect_0->x < rect_1->x + rect_1->w && rect_0->x + rect_0->w > rect_1->x && rect_0->y + rect_0->h > rect_1->y && rect_0->y < rect_1->y + rect_1->h);
    }

    template <typename T, int N>
    inline bool elementIn(const T& element, const T arr[N])
    {
        for (std::size_t i{0}; i < N; ++i)
        {
            if (element == arr[i])
            {
                return true;
            }
        }
        return false;
    }

    template <typename T, int N>
    inline T pickRandom(const T arr[N])
    {
        return arr[static_cast<std::size_t>(std::rand() % N)];
    }

    inline double random()
    {
        return static_cast<double>((double)std::rand() / (RAND_MAX));
    }

    template <typename T>
    inline double distance(vec2<T> vec1, vec2<T> vec2)
    {
        return std::sqrt(std::pow(vec1.y - vec2.y, 2) + std::pow(vec1.x - vec2.x, 2));
    }

    template <typename T>
    inline void swap(T** val1, T** val2)
    {
        T* temp {*val1}; // create temp for value of val1
        *val1 = *val2; // swap addresses
        *val2 = temp;
    }
}

#endif

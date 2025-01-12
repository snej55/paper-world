#ifndef UTIL_H
#define UTIL_H

#include "SDL2/SDL.h"
#include <array>
#include <vector>
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

    inline SDL_Color lerpColor(SDL_Color& a, SDL_Color& b, const double x)
    {
        SDL_Color color{0x00, 0x00, 0x00, 0xFF};
        color.r = static_cast<uint8_t>(static_cast<int>(a.r + (static_cast<double>(b.r) - static_cast<double>(a.r)) * x));
        color.g = static_cast<uint8_t>(static_cast<int>(a.g + (static_cast<double>(b.g) - static_cast<double>(a.g)) * x));
        color.b = static_cast<uint8_t>(static_cast<int>(a.b + (static_cast<double>(b.b) - static_cast<double>(a.b)) * x));
        return color;
    }

    inline SDL_Color lerpColorAlpha(SDL_Color& a, SDL_Color& b, const double x)
    {
        SDL_Color color{0x00, 0x00, 0x00, 0xFF};
        color.r = static_cast<uint8_t>(static_cast<int>(a.r + (static_cast<double>(b.r) - static_cast<double>(a.r)) * x));
        color.g = static_cast<uint8_t>(static_cast<int>(a.g + (static_cast<double>(b.g) - static_cast<double>(a.g)) * x));
        color.b = static_cast<uint8_t>(static_cast<int>(a.b + (static_cast<double>(b.b) - static_cast<double>(a.b)) * x));
        color.a = static_cast<uint8_t>(static_cast<int>(a.a + (static_cast<double>(b.a) - static_cast<double>(a.a)) * x));
        return color;
    }

    template <typename type>
    std::vector<int> get_water_indices(std::vector<type> vertices)
    {
        int T{static_cast<int>(vertices.size())};
        int H{static_cast<int>((T + 1) / 2)};
        int TQ{static_cast<int>((T - 1) / 2)};
        std::vector<int> indices;
        indices.resize(TQ * 6);

        for (int c{0}; c < TQ; ++c)
        {
            int qi {c * 6}; // quad index
            indices[qi] = c;
            indices[qi + 1] = c + 1;
            indices[qi + 2] = H + c;
            indices[qi + 3] = H + c;
            indices[qi + 4] = H + c + 1;
            indices[qi + 5] = c + 1;
        }

        return indices;
    }

    template <typename T>
    void printVector(const std::vector<T>& vec)
    {
        const std::size_t size{vec.size()};
        std::cout << "{";
        for (std::size_t i{0}; i < size; ++i)
        {
            std::cout << vec[i];
            if (i < size - 1)
            {
                std::cout << ", ";
            }
        }
        std::cout << "}\n";
    }
}

#endif
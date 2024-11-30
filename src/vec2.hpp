#ifndef VECTOR2_H
#define VECTOR2_H

#include <iostream>

template <typename T>
struct vec2
{
    //operator vec2<int>() const {
    //    return vec2<int>{(int)x, (int)y};
    //}
    T x;
    T y;
};

template <typename T>
vec2<T> operator+(vec2<T> vec_0, vec2<T> vec_1)
{
    return vec2<T>{vec_0.x + vec_1.x, vec_0.y + vec_1.y};
}

template <typename T>
vec2<T> operator-(vec2<T> vec_0, vec2<T> vec_1)
{
    return vec2<T>{vec_0.x - vec_1.x, vec_0.y - vec_1.y};
}

template <typename T>
std::ostream& operator<< (std::ostream& out, const vec2<T> vec)
{
    out << "vec2(" << vec.x << ", " << vec.y << ")";
    return out;
}

template <typename T, typename U>
vec2<T> operator*(vec2<T> vec, U mult)
{
    return vec2<T>{vec.x * mult, vec.y * mult};
}

template <typename T, typename U>
vec2<T> operator*(U mult, vec2<T> vec)
{
    return vec2<T>{vec.x * mult, vec.y * mult};
}

template <typename T, typename U>
vec2<T> operator*=(vec2<T> vec, U mult)
{
    return vec2<T>{vec.x * mult, vec.y * mult};
}

template <typename T, typename U>
vec2<T> operator*=(U mult, vec2<T> vec)
{
    return vec2<T>{vec.x * mult, vec.y * mult};
}

#endif

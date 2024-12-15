#ifndef HEALTH_BARS_H
#define HEALTH_BARS_H

#include "./vec2.hpp"
#include "./util.hpp"
#include "./texman.hpp"

class EntityHealthBar
{
private:
    Texture* _tex;
    vec2<int> _dimensions;
    const double _maxHealth;
    double _health{0.0};
    SDL_Color _greenDark{0x32, 0x6b, 0x64, 0xFF};
    SDL_Color _greenLight{0x60, 0xae, 0x7b, 0xFF};
    SDL_Color _redDark{0xa8, 0x60, 0x5d, 0xFF};
    SDL_Color _redLight{0xd1, 0xa7, 0x7e, 0xFF};

public:
    EntityHealthBar(Texture* tex, vec2<int> dimensions, const double maxHealth);

    void update(void* entity);

    void render(const int scrollX, const int scrollY, SDL_Renderer* renderer, void* entity);
};

#endif
#include "./entities.hpp"

EntityHealthBar::EntityHealthBar(Texture* tex, vec2<int> dimensions, const double maxHealth)
    : _tex{tex}, _dimensions{dimensions}, _maxHealth{maxHealth}
{
    _health = maxHealth;
}

void EntityHealthBar::update(void* entity)
{
    _health = static_cast<Entity*>(entity)->getHealth();
}

void EntityHealthBar::render(const int scrollX, const int scrollY, SDL_Renderer* renderer, void* entity)
{
    vec2<double> targetPos {static_cast<Entity*>(entity)->getCenter()};
    targetPos.y = static_cast<Entity*>(entity)->getPos().y - _dimensions.y - 3;
    targetPos.x -= 8;
    SDL_Rect upperBar{(int)targetPos.x + 2 - scrollX, (int)targetPos.y + 1 - scrollY, _dimensions.x * _health / _maxHealth, 1};
    SDL_Rect lowerBar{(int)targetPos.x + 2 - scrollX, (int)targetPos.y + 2 - scrollY, _dimensions.x * _health / _maxHealth, 1};
    SDL_Color lightColor {Util::lerpColor(_redLight, _greenLight, _health / _maxHealth)};
    SDL_Color darkColor {Util::lerpColor(_redDark, _greenDark, _health / _maxHealth)};
    SDL_Rect Bar{(int)targetPos.x + 2 - scrollX, (int)targetPos.y + 1 - scrollY, _dimensions.x, 2};
    SDL_SetRenderDrawColor(renderer, 0x1f, 0x24, 0x4b, 0xff);
    SDL_RenderFillRect(renderer, &Bar);
    SDL_SetRenderDrawColor(renderer, lightColor.r, lightColor.g, lightColor.b, 0xFF);
    SDL_RenderFillRect(renderer, &upperBar);
    SDL_SetRenderDrawColor(renderer, darkColor.r, darkColor.g, darkColor.b, 0xFF);
    SDL_RenderFillRect(renderer, &lowerBar);
}
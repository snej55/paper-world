#ifndef COIN_H
#define COIN_H

#include "./texture.hpp"
#include "./vec2.hpp"
#include "./util.hpp"
#include "./anim.hpp"

#include <vector>
#include <array>

struct Coin
{
    vec2<double> pos;
    vec2<double> vel;
    Anim* anim;
};

struct Glow
{
    vec2<double> pos;
    vec2<double> vel;
    double size;
};

class CoinManager
{
private:
    std::vector<Coin*> _Coins;
    Texture* _coinTex{nullptr};
    Texture* _glowTex{nullptr};

    std::vector<Glow*> _Glow;

public:
    CoinManager();
    CoinManager(Texture* coinTex, Texture* glowTex);
    ~CoinManager();

    void setTex(Texture* coinTex, Texture* glowTex);
    
    void free();

    void addGlow(vec2<double> pos, vec2<double> vel);

    void addCoin(vec2<double> pos, vec2<double> vel);

    void updateCoin(Coin* coin, const double& time_step, void* world);

    void renderCoin(Coin* coin, const int scrollX, const int scrollY, SDL_Renderer* renderer);

    void update(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, void* world, SDL_Rect* player_rect);
};

#endif
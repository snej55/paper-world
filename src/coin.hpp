#ifndef COIN_H
#define COIN_H

#include "./texman.hpp"
#include "./vec2.hpp"
#include "./util.hpp"
#include "./timer.hpp"
#include "./anim.hpp"
#include "./sparks.hpp"

#include <vector>
#include <array>

struct Coin
{
    vec2<double> pos;
    vec2<double> vel;
    Anim* anim;
    bool dead{false};
    Timer timer{};
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

    SparkManager _SparkManager{0.0, 0.2, 1.0, nullptr};

    Timer timer{};

    std::vector<Glow*> _Glow;

    int _score{0};

public:
    CoinManager();
    CoinManager(Texture* coinTex, Texture* glowTex);
    ~CoinManager();

    int getScore() {return _score;}
    void setScore(int val) {_score = val;}

    void setTex(Texture* coinTex, Texture* glowTex);
    
    void free();

    void addGlow(vec2<double> pos, vec2<double> vel);

    void addCoin(vec2<double> pos, vec2<double> vel);

    void updateCoin(Coin* coin, const double& time_step, void* world);

    void renderCoin(Coin* coin, const int scrollX, const int scrollY, SDL_Renderer* renderer);

    void update(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, void* world, TexMan* texman, SDL_Rect* player_rect, double& last_coin);
};

#endif
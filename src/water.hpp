#ifndef WATER_H
#define WATER_H

#include "./constants.hpp"
#include "./texman.hpp"
#include "./vec2.hpp"
#include "./util.hpp"
#include "./polygons.hpp"
#include "./player.hpp"
#include "./timer.hpp"

#include <vector>
#include <cmath>

struct WaterSpring
{
    vec2<double> pos;
    double target_y;
    double vel{0};
};

class Water
{
private:
    vec2<int> _pos; // relative tile pos
    vec2<int> _dimensions; // relative tile dimensions
    double _spacing;

    double _tension{0.15};

    std::vector<WaterSpring*> _Springs{};
    SDL_Rect _Rect;

    Timer timer{};

public:
    Water(vec2<int> pos, vec2<int> dimensions, double spacing);
    ~Water();

    void free();
    void loadSprings();

    SDL_Rect* getRect();

    void updateSpring(WaterSpring* spring, WaterSpring* left, WaterSpring* right, const double& time_step);
    void update(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, TexMan* texman, Player* player);
};

// class WaterManager
// {
// private:

// };

#endif
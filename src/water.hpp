#ifndef WATER_H
#define WATER_H

#include "JSON/json.hpp"

#include "./constants.hpp"
#include "./texman.hpp"
#include "./vec2.hpp"
#include "./util.hpp"
#include "./polygons.hpp"
#include "./player.hpp"
#include "./timer.hpp"

#include <vector>
#include <cmath>
#include <string>
#include <fstream>

using json = nlohmann::json;

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

class WaterManager
{
private:
    std::vector<Water*> _Water{};

public:
    WaterManager()
    {
    }

    WaterManager(const char* path);

    ~WaterManager();

    void loadFromFile(const char* path);

    void update(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, TexMan* texman, Player* player);
};

class Lava
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
    Lava(vec2<int> pos, vec2<int> dimensions, double spacing);
    ~Lava();

    void free();
    void loadSprings();

    SDL_Rect* getRect();

    void updateSpring(WaterSpring* spring, WaterSpring* left, WaterSpring* right, const double& time_step);
    void update(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, TexMan* texman, Player* player);
};

class LavaManager
{
private:
    std::vector<Lava*> _Lava{};

public:
    LavaManager()
    {
    }

    LavaManager(const char* path);

    ~LavaManager();

    void loadFromFile(const char* path);

    void update(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, TexMan* texman, Player* player);
};

#endif
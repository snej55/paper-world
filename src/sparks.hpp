#ifndef SPARKS_H
#define SPARKS_H

#include "./util.hpp"
#include "./polygons.hpp"
#include "./texture.hpp"
#include <vector>
#include <cmath>
#include <algorithm>

struct Spark
{
    vec2<double> pos;
    double angle;
    double speed;
    bool alive{true};
};

class SparkManager
{
private:
    std::vector<Spark*> _Sparks;
    const double _gravity;
    const double _decay;
    const double _size; // size multiplier
    Texture* _particleTexture;

public:
    SparkManager(const double gravity, const double decay, const double size, Texture* texture);
    ~SparkManager();
    void free();

    void addSpark(Spark* spark);

    void point_towards(Spark* spark, const double angle, const double rate, const double& time_step);

    void updateSpark(Spark* spark, const double& time_step);
    void renderSpark(Spark* spark, const int scrollX, const int scrollY, SDL_Renderer* renderer);
    void update(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer); // draw and update
};

#endif
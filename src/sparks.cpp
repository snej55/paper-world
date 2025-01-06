#include "./sparks.hpp"

SparkManager::SparkManager(const double gravity, const double decay, const double size, Texture* texture)
 : _gravity{gravity}, _decay{decay}, _size{size}, _particleTexture{texture}
{
}

SparkManager::~SparkManager()
{
    free();
}

void SparkManager::free()
{
    for (Spark* e : _Sparks)
    {
        if (e != nullptr)
        {
            delete e;
        }
    }
    _Sparks.clear();
}

void SparkManager::addSpark(Spark* spark)
{
    _Sparks.push_back(spark);
}

void SparkManager::point_towards(Spark* spark, const double angle, const double rate, const double& time_step)
{
    double rotate_direction {std::fmod(angle - spark->angle + M_PI * 3.0, M_PI * 2) - M_PI};
    double rotate_sign{(rotate_direction < 0.0) ? -1.0 : 1.0};
    if (std::abs(rotate_direction) < rate)
    {
        spark->angle = angle;
    } else {
        spark->angle += rate * rotate_sign * time_step;
    }
}

void SparkManager::updateSpark(Spark* spark, const double& time_step)
{
    spark->pos.x += std::cos(spark->angle) * spark->speed * time_step;
    spark->pos.y += std::sin(spark->angle) * spark->speed * time_step;

    point_towards(spark, M_PI / 2.0, 0.02, time_step); // gravity

    // vec2<double> movement {std::cos(spark->angle) * spark->speed, std::sin(spark->angle) * spark->speed}; // calculate movement for spark
    // movement.y = std::min(1.0, movement.y); // cap at terminal velocity
    // movement.x += (movement.x * 0.975 - movement.x) * time_step; // add some friction
    // spark->angle = std::atan2(movement.y, movement.x);

    spark->speed -= _decay * time_step;

    spark->alive = spark->speed > 0.0; // check if the spark is dead
}

void SparkManager::renderSpark(Spark* spark, const int scrollX, const int scrollY, SDL_Renderer* renderer)
{
    SDL_Color col {0xFF, 0xFF, 0xFF, 0xCC};
    std::vector<SDL_Vertex> vertices {
        SDL_Vertex{{static_cast<float>(spark->pos.x) - static_cast<float>(scrollX) + static_cast<float>(std::cos(spark->angle) * spark->speed * _size), static_cast<float>(spark->pos.y) - static_cast<float>(scrollY) + static_cast<float>(std::sin(spark->angle) * spark->speed * _size)},
        col, {0.0f, 0.0f}},
        SDL_Vertex{{static_cast<float>(spark->pos.x) - static_cast<float>(scrollX) + static_cast<float>(std::cos(spark->angle + M_PI * 0.5) * spark->speed * _size * 0.5), static_cast<float>(spark->pos.y) - static_cast<float>(scrollY) + static_cast<float>(std::sin(spark->angle + M_PI * 0.5) * spark->speed * _size * 0.5)},
        col, {0.0f, 1.0f}},
        SDL_Vertex{{static_cast<float>(spark->pos.x) - static_cast<float>(scrollX) - static_cast<float>(std::cos(spark->angle) * spark->speed * _size * 3.5), static_cast<float>(spark->pos.y) - static_cast<float>(scrollY) - static_cast<float>(std::sin(spark->angle) * spark->speed * _size * 3.5)},
        col, {1.0f, 1.0f}},
        SDL_Vertex{{static_cast<float>(spark->pos.x) - static_cast<float>(scrollX) + static_cast<float>(std::cos(spark->angle - M_PI * 0.5) * spark->speed * _size * 0.5), static_cast<float>(spark->pos.y) - static_cast<float>(scrollY) - static_cast<float>(std::sin(spark->angle + M_PI * 0.5) * spark->speed * _size * 0.5)},
        col, {1.0f, 0.0f}}
    };
    std::vector<int> indices {0, 1, 2, 2, 3, 1}; // indices for quad
    Polygons::renderPolygon(renderer, _particleTexture->getTexture(), vertices, indices);
}

void SparkManager::update(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer)
{
    for (std::size_t i{0}; i < _Sparks.size(); ++i)
    {
        Spark* spark{_Sparks[i]};
        if (spark != nullptr)
        {
            updateSpark(spark, time_step);
            if (spark->alive)
            {
                renderSpark(spark, scrollX, scrollY, renderer);
            } else {
                delete spark;
                spark = nullptr;
            }
        }
    }
    _Sparks.erase(std::remove_if(_Sparks.begin(), _Sparks.end(), [](Spark* spark){return !(spark->alive);}), _Sparks.end());
}
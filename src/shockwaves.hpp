#ifndef SHOCKWAVES_H
#define SHOCKWAVES_H

#include "./texture.hpp"
#include "./vec2.hpp"
#include "./anim.hpp"

#include <vector>

struct ShockWave
{
    vec2<double> pos;
    Anim* anim;
};

class ShockWaveManager
{
private:
    Texture* _tex;
    std::vector<ShockWave*> _Waves{};

public:
    ShockWaveManager()
    {
    }

    ShockWaveManager(Texture* tex)
     : _tex{tex}
    {
    }

    ~ShockWaveManager()
    {
        free();
    }

    void free()
    {
        for (std::size_t i{0}; i < _Waves.size(); ++i)
        {
            ShockWave* wave{_Waves[i]};
            if (wave != nullptr)
            {
                delete wave->anim;
                delete wave;
                _Waves[i] = nullptr;
            }
        }
        _Waves.clear();
    }

    void setTex(Texture* tex)
    {
        _tex = tex;
    }

    void addShockWave(vec2<double> pos)
    {
        Anim* anim{new Anim{24, 24, 6, 0.5, false, _tex}};
        _Waves.push_back(new ShockWave{{pos.x - 12.0, pos.y - 12.0}, anim});
    }

    void update(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer)
    {
        for (std::size_t i{0}; i < _Waves.size(); ++i)
        {
            ShockWave* wave{_Waves[i]};
            wave->anim->tick(time_step);
            wave->anim->render(static_cast<int>(wave->pos.x), static_cast<int>(wave->pos.y), scrollX, scrollY, renderer);
            if (wave->anim->getFinished())
            {
                delete wave->anim;
                delete wave;
                _Waves[i] = nullptr;
            }
        }
        _Waves.erase(std::remove_if(_Waves.begin(), _Waves.end(), [](ShockWave* wave){return (wave == nullptr);}), _Waves.end());
    }
};

#endif
#ifndef CLOUDS_H
#define CLOUDS_H

#include "./texman.hpp"
#include "./vec2.hpp"
#include "./util.hpp"

#include <vector>

struct Cloud
{
    vec2<double> pos;
    double speed;
    double depth;
    int frame;
};

class CloudManager
{
private:
    std::vector<Cloud*> _Clouds{};
    Texture* _tex{nullptr};

public:
    CloudManager(int cloudiness)
    {
        generate(cloudiness);
    }

    CloudManager(int cloudiness, TexMan* texman)
    {
        generate(cloudiness);
        setTex(&(texman->cloud_light));
    }

    ~CloudManager()
    {
        free();
    }

    void generate(int cloudiness)
    {
        free();
        for (int i{0}; i < cloudiness; ++i)
        {
            _Clouds.push_back(new Cloud{{Util::random() * 10000.0, Util::random() * 10000.0}, Util::random() * 0.1 + 0.1, static_cast<double>(i) / static_cast<double>(cloudiness) * 0.6 + 0.2, static_cast<int>(Util::random())});
        }
    }

    void free()
    {
        for (std::size_t i{0}; i < _Clouds.size(); ++i)
        {
            Cloud* cloud{_Clouds[i]};
            if (cloud != nullptr)
            {
                delete cloud;
                _Clouds[i] = nullptr;
            }
        }
        _Clouds.clear();
    }

    void setTex(Texture* tex)
    {
        _tex = tex;
    }

    void update(const double& time_step, const int scrollX, const int scrollY, const int width, const int height, SDL_Renderer* renderer)
    {
        for (std::size_t i{0}; i < _Clouds.size(); ++i)
        {
            Cloud* cloud{_Clouds[i]};
            cloud->pos.x += cloud->speed * time_step;
            vec2<double> render_pos{cloud->pos.x - static_cast<double>(scrollX) * cloud->depth, cloud->pos.y - static_cast<double>(scrollY) * cloud->depth};
            SDL_Rect clipRect{cloud->frame * 64, 0, 64, 32};
            _tex->render((static_cast<int>(render_pos.x) % (width + 64)) - 64, (static_cast<int>(render_pos.y) % (height + 64)) - 64, renderer, &clipRect);
        }
    }
};

#endif
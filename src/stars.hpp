#ifndef STARS_H
#define STARS_H

#include "./texman.hpp"
#include "./vec2.hpp"
#include "./util.hpp"
#include "./timer.hpp"

#include <vector>

struct Star
{
    vec2<double> pos;
    double speed;
    double depth;
    double frame;
};

class StarManager
{
private:
    std::vector<Star*> _Stars{};
    Texture* _tex{nullptr};
    Timer timer{};

public:
    StarManager(int stariness)
    {
        generate(stariness);
        timer.start();
    }

    StarManager(int stariness, TexMan* texman)
    {
        generate(stariness);
        setTex(&(texman->lightTex));
        timer.start();
    }

    ~StarManager()
    {
        free();
    }

    void generate(int stariness)
    {
        free();
        for (int i{0}; i < stariness; ++i)
        {
            _Stars.push_back(new Star{{Util::random() * 10000.0, Util::random() * 10000.0}, Util::random() * 0.1 + 0.1, static_cast<double>(i) / static_cast<double>(stariness) * 0.1, static_cast<double>(Util::random() * 10000)});
        }
    }

    void free()
    {
        for (std::size_t i{0}; i < _Stars.size(); ++i)
        {
            Star* Star{_Stars[i]};
            if (Star != nullptr)
            {
                delete Star;
                _Stars[i] = nullptr;
            }
        }
        _Stars.clear();
    }

    void setTex(Texture* tex)
    {
        _tex = tex;
    }

    void update(const double& time_step, const int scrollX, const int scrollY, const int width, const int height, SDL_Renderer* renderer)
    {
        for (std::size_t i{0}; i < _Stars.size(); ++i)
        {
            Star* star{_Stars[i]};
            // Star->pos.x += Star->speed * time_step;
            vec2<double> render_pos{star->pos.x - static_cast<double>(scrollX) * star->depth * 0.1, star->pos.y - static_cast<double>(scrollY) * star->depth * 0.1};
            _tex->setBlendMode(SDL_BLENDMODE_ADD);
            _tex->setAlpha(static_cast<Uint8>(static_cast<int>(std::sin(static_cast<double>(timer.getTicks()) * 0.0005 + star->frame) * 50.0 + 50.0)));
            SDL_Rect renderQuad{(static_cast<int>(render_pos.x) % (width + 64)) - 64, (static_cast<int>(render_pos.y) % (height + 64)) - 64, 3, 3};
            SDL_RenderCopyEx(renderer, _tex->getTexture(), NULL, &renderQuad, 0, NULL, SDL_FLIP_NONE);
        }
    }
};

#endif
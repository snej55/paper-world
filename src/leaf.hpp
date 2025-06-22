#ifndef LEAF_H
#define LEAF_H

#include "./texman.hpp"
#include "./anim.hpp"
#include "./vec2.hpp"
#include "./util.hpp"

#include <vector>
#include <array>

struct Leaf
{
    vec2<double> pos;
    vec2<double> vel;
    Anim* anim;
    bool solid{false};
};

struct LeafSpawner
{
    SDL_Rect rect;
    bool solid;
};

class LeafManager
{
private:
    std::vector<Leaf*> _leaves{};
    std::vector<LeafSpawner> _spawn_rects{};

    std::array<std::array<double, 2>, 3> _wind {{
        {0.0, 10.0},
        {0.0, 15.0},
        {0.0, 5.0}
    }};

public:
    LeafManager()
    {
    }

    ~LeafManager()
    {
        free();
    }

    void free()
    {
        for (std::size_t i{0}; i < _leaves.size(); ++i)
        {
            Leaf* leaf = _leaves[i];
            if (leaf != nullptr)
            {
                delete leaf->anim;
                delete leaf;
                _leaves[i] = nullptr;
            }
        }
        _leaves.clear();
        _spawn_rects.clear();
    }

    void loadRects(std::vector<LeafSpawner>& rects)
    {
        for (std::size_t i{0}; i < rects.size(); ++i)
        {
            _spawn_rects.push_back(rects[i]);
        }
    }

    void update(const double& time_step, const int scrollX, const int scrollY, const int width, const int height, TexMan* texman, SDL_Renderer* renderer)
    {
        // wind? wrote this ages ago
        double average_gust{0.0};
        for (std::size_t i{0}; i < _wind.size(); ++i)
        {
            _wind[i][0] -= (_wind[i][1] + std::sin(_wind[i][0] * 0.025) * 0.3) * time_step * 0.5;
            if (scrollX < static_cast<int>(_wind[i][0]) && static_cast<int>(_wind[i][0]) < scrollX + width)
            {
                _wind[i][1] = 5.0 * (Util::random() + 0.5) * 2.0;
                _wind[i][0] = static_cast<double>(scrollX + width) - _wind[i][1] * time_step;
            }
            average_gust += _wind[i][1];
        }
        average_gust *= 0.5;

        SDL_Rect screen_rect{scrollX - 64, scrollY - 64, width + 128, height + 128};
        for (std::size_t i{0}; i < _spawn_rects.size(); ++i)
        {
            // don't ask
            LeafSpawner spawner{_spawn_rects[i]};
            if (Util::random() * 20000.0 / (average_gust * 0.15) / time_step < static_cast<double>(spawner.rect.w * spawner.rect.h))
            {
                if (Util::checkCollision(&(spawner.rect), &screen_rect))
                {
                    vec2<double> pos{static_cast<double>(spawner.rect.x) + Util::random() * static_cast<double>(spawner.rect.w), static_cast<double>(spawner.rect.y) + Util::random() * static_cast<double>(spawner.rect.h)};
                    Leaf* leaf{new Leaf{pos, {-0.1, 0.2}, new Anim{8, 8, 17, 0.1, false, &(texman->leafTex)}, spawner.solid}};
                    leaf->anim->setFrame(static_cast<int>(Util::random() * 15.0));
                    _leaves.push_back(leaf);
                }
            }
        }

        // update the leaves
        for (std::size_t i{0}; i < _leaves.size(); ++i)
        {
            Leaf* leaf{_leaves[i]};
            if (leaf != nullptr)
            {
                leaf->pos.x += leaf->vel.x * time_step;
                leaf->pos.y += leaf->vel.y * time_step;
                leaf->pos.x += std::sin(leaf->anim->getFrame() * 0.08) * 0.8 * time_step - 0.5 * time_step * average_gust * 0.1;
                leaf->vel.y = std::min(0.2, leaf->vel.y + 0.005 / (average_gust * 0.1) * time_step);
                leaf->anim->tick(time_step);
                leaf->anim->getTex()->setAlpha(static_cast<Uint8>(static_cast<int>((17.0 - leaf->anim->getFrame()) / 17.0 * 255.0)));
                leaf->anim->getTex()->setBlendMode(SDL_BLENDMODE_BLEND);
                leaf->anim->render(static_cast<int>(leaf->pos.x), static_cast<int>(leaf->pos.y), scrollX, scrollY, renderer);
                if (leaf->anim->getFinished())
                {
                    delete leaf->anim;
                    delete leaf;
                    _leaves[i] = nullptr;
                }
            }
        }
        // black magic
        _leaves.erase(std::remove_if(_leaves.begin(), _leaves.end(), [](Leaf* leaf){return (leaf == nullptr);}), _leaves.end());
    }
};

#endif
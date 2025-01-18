#ifndef LEAF_H
#define LEAF_H

#include "./texman.hpp"
#include "./anim.hpp"
#include "./vec2.hpp"

#include <vector>

struct Leaf
{
    vec2<double> pos;
    Anim* anim;
};

class LeafSpawner
{
private:
    std::vector<Leaf*> leaves{};
    std::vector<SDL_Rect> spawn_rects{};

public:
    LeafSpawner()
    {
    }

    ~LeafSpawner()
    {
        free();
    }

    void free()
    {
        for (std::size_t i{0}; i < leaves.size(); ++i)
        {
            Leaf* leaf = leaves[i];
            delete leaf->anim;
            delete leaf;
            leaves[i] = nullptr;
        }
        leaves.clear();
    }

    void loadRects(std::vector<SDL_Rect>& rects)
    {
        for (std::size_t i{0}; i < rects.size(); ++i)
        {
            spawn_rects.push_back(rects[i]);
        }
    }

    void update(const int scrollX, const int scrollY, TexMan* texman, SDL_Renderer renderer)
    {

    }
};

#endif
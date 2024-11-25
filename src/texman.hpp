#ifndef TEXMAN_H
#define TEXMAN_H

#include "./texture.hpp"

class TexMan
{
public:
    // Tiles
    Texture tileGrassTex{};
    Texture tileRockTex{};

    TexMan()
    {
    }

    ~TexMan()
    {
        tileGrassTex.free();
        tileRockTex.free();
    }

    void confirm(bool val, bool& success)
    {
        if (success)
        {
            success = val;
        }
    }

    bool load(SDL_Window* window, SDL_Renderer* renderer)
    {
        bool success{true};
        confirm(tileGrassTex.loadFromFile("data/images/tiles/grass.png", window, renderer), success);
        confirm(tileRockTex.loadFromFile("data/images/tiles/rock.png", window, renderer), success);
        return success;
    }
};

#endif
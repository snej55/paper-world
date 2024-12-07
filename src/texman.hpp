#ifndef TEXMAN_H
#define TEXMAN_H

#include "./texture.hpp"

class TexMan
{
public:
    // Tiles
    Texture tileGrassTex{};
    Texture tileRockTex{};
    Texture tileSpikeTex{};

    Texture particle{};
    Texture particleFire{};

    // Textures
    Texture slimeIdle{};
    Texture slimeRun{};
    Texture slimeJump{};

    TexMan()
    {
    }

    ~TexMan()
    {
        tileGrassTex.free();
        tileRockTex.free();
        tileSpikeTex.free();
        particle.free();
        particleFire.free();
        slimeIdle.free();
        slimeRun.free();
        slimeJump.free();
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
        confirm(tileSpikeTex.loadFromFile("data/images/tiles/spike.png", window, renderer), success);
        confirm(particle.loadFromFile("data/images/particles/particle.png", window, renderer), success);
        confirm(particleFire.loadFromFile("data/images/particles/fire.png", window, renderer), success);
        confirm(slimeIdle.loadFromFile("data/images/entities/slime/idle.png", window, renderer), success);
        confirm(slimeRun.loadFromFile("data/images/entities/slime/run.png", window, renderer), success);
        confirm(slimeJump.loadFromFile("data/images/entities/slime/jump.png", window, renderer), success);
        return success;
    }
};

#endif
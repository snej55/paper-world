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
    Texture tileSpringTex{};

    // Particle Textures
    Texture particle{};
    Texture particleFire{};
    Texture lightTex{};

    // Entity Textures
    Texture slimeIdle{};
    Texture slimeRun{};
    Texture slimeJump{};
    Texture slimeFlash{};
    Texture bat{};
    Texture batFlash{};

    // Player Textures
    Texture playerIdle{};
    Texture playerRun{};
    Texture playerJump{};
    Texture playerLand{};
    Texture playerFlash{};

    TexMan()
    {
    }

    ~TexMan()
    {
        tileGrassTex.free();
        tileRockTex.free();
        tileSpikeTex.free();
        tileSpringTex.free();
        particle.free();
        particleFire.free();
        lightTex.free();
        slimeIdle.free();
        slimeRun.free();
        slimeJump.free();
        slimeFlash.free();
        bat.free();
        batFlash.free();
        playerIdle.free();
        playerRun.free();
        playerJump.free();
        playerLand.free();
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
        confirm(tileSpringTex.loadFromFile("data/images/tiles/spring.png", window, renderer), success);
        confirm(particle.loadFromFile("data/images/particles/particle.png", window, renderer), success);
        confirm(particleFire.loadFromFile("data/images/particles/fire.png", window, renderer), success);
        confirm(slimeIdle.loadFromFile("data/images/entities/slime/idle.png", window, renderer), success);
        confirm(slimeRun.loadFromFile("data/images/entities/slime/run.png", window, renderer), success);
        confirm(slimeJump.loadFromFile("data/images/entities/slime/jump.png", window, renderer), success);
        confirm(bat.loadFromFile("data/images/entities/bat/bat.png", window, renderer), success);
        confirm(batFlash.loadFromFile("data/images/entities/bat/flash.png", window, renderer), success);
        confirm(slimeFlash.loadFromFile("data/images/entities/slime/flash.png", window, renderer), success);
        confirm(lightTex.loadFromFile("data/images/particles/light.png", window, renderer), success);
        confirm(playerIdle.loadFromFile("data/images/entities/player/idle.png", window, renderer), success);
        confirm(playerRun.loadFromFile("data/images/entities/player/run.png", window, renderer), success);
        confirm(playerJump.loadFromFile("data/images/entities/player/jump.png", window, renderer), success);
        confirm(playerLand.loadFromFile("data/images/entities/player/land.png", window, renderer), success);
        confirm(playerFlash.loadFromFile("data/images/entities/player/flash.png", window, renderer), success);
        return success;
    }
};

#endif
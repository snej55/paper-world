#ifndef TEXMAN_H
#define TEXMAN_H

#include "SDL2/SDL_mixer.h"
#include "./texture.hpp"
#include "./audio.hpp"

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
    Texture turtleIdle{};
    Texture turtleRun{};
    Texture turtleJump{};
    Texture turtleLand{};
    Texture turtleFlash{};

    // Player Textures
    Texture playerIdle{};
    Texture playerRun{};
    Texture playerJump{};
    Texture playerLand{};
    Texture playerFlash{};

    // Weapons
    Texture swordBase{};
    Texture slash{};
    Texture blasterBase{};
    Texture laserBlue{};
    Texture laserRed{};
    // Grass!
    Texture grass{};

    // HUD
    Texture enemyHealthBar{};
    Texture playerHealthBar{};

    // Decor!
    Texture tileTrees{};

    // -------- Sounds --------- //
    Sound SFX_death_0{};
    Sound SFX_hit_0{};
    Sound SFX_hit_1{};
    Sound SFX_hit_2{};
    Sound SFX_hit_3{};
    Sound SFX_player_hit{};
    Sound SFX_sword_slash{};
    Sound SFX_sword_swoosh{};
    Sound SFX_landing{};
    Sound SFX_turtle{};
    Sound SFX_spring{};

    TexMan()
    {
    }

    ~TexMan()
    {
        tileGrassTex.free();
        tileRockTex.free();
        tileSpikeTex.free();
        tileSpringTex.free();
        tileTrees.free();
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
        grass.free();
        swordBase.free();
        slash.free();
        blasterBase.free();
        laserBlue.free();
        laserRed.free();
        enemyHealthBar.free();
        playerHealthBar.free();
        turtleIdle.free();
        turtleRun.free();
        turtleJump.free();
        turtleLand.free();
        turtleFlash.free();
        SFX_death_0.free();
        SFX_hit_0.free();
        SFX_hit_1.free();
        SFX_hit_2.free();
        SFX_hit_3.free();
        SFX_player_hit.free();
        SFX_sword_slash.free();
        SFX_sword_swoosh.free();
        SFX_landing.free();
        SFX_turtle.free();
        SFX_spring.free();
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
        confirm(tileTrees.loadFromFile("data/images/tiles/trees.png", window, renderer), success);
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
        confirm(grass.loadFromFile("data/images/grass/grass.png", window, renderer), success);
        confirm(swordBase.loadFromFile("data/images/entities/sword.png", window, renderer), success);
        confirm(slash.loadFromFile("data/images/vfx/slash.png", window, renderer), success);
        confirm(blasterBase.loadFromFile("data/images/blasters/blaster.png", window, renderer), success);
        confirm(laserBlue.loadFromFile("data/images/blasters/laser.png", window, renderer), success);
        confirm(laserRed.loadFromFile("data/images/blasters/laser_red.png", window, renderer), success);
        confirm(enemyHealthBar.loadFromFile("data/images/entities/enemy_health_bar.png", window, renderer), success);
        confirm(playerHealthBar.loadFromFile("data/images/entities/health_bar.png", window, renderer), success);
        confirm(turtleIdle.loadFromFile("data/images/entities/turtle/idle.png", window, renderer), success);
        confirm(turtleRun.loadFromFile("data/images/entities/turtle/run.png", window, renderer), success);
        confirm(turtleJump.loadFromFile("data/images/entities/turtle/jump.png", window, renderer), success);
        confirm(turtleLand.loadFromFile("data/images/entities/turtle/land.png", window, renderer), success);
        confirm(turtleFlash.loadFromFile("data/images/entities/turtle/flash.png", window, renderer), success);
        SFX_death_0.loadFromFileWAV("data/audio/death/death_0.wav");
        SFX_hit_0.loadFromFileWAV("data/audio/hit/hit_0.wav");
        SFX_hit_1.loadFromFileWAV("data/audio/hit/hit_1.wav");
        SFX_hit_2.loadFromFileWAV("data/audio/hit/hit_2.wav");
        SFX_hit_3.loadFromFileWAV("data/audio/hit/hit_3.wav");
        SFX_player_hit.loadFromFileWAV("data/audio/hit/player_hit.wav");
        SFX_sword_slash.loadFromFileWAV("data/audio/sword/slash.wav");
        SFX_sword_swoosh.loadFromFileWAV("data/audio/sword/swoosh.wav");
        SFX_landing.loadFromFileWAV("data/audio/misc/footstep01.wav");
        SFX_turtle.loadFromFileWAV("data/audio/misc/turtle.wav");
        SFX_spring.loadFromFileWAV("data/audio/misc/spring.wav");
        return success;
    }

    void playDamageSound()
    {
        int num{static_cast<int>(std::rand()) % 4};
        switch (num)
        {
            case 0:
                SFX_hit_0.play();
            case 1:
                SFX_hit_1.play();
            case 2:
                SFX_hit_3.play();
            case 3:
                SFX_hit_3.play();
            default:
                return;
        }
    }
};

#endif
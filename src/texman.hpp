#ifndef TEXMAN_H
#define TEXMAN_H

#include "SDL2/SDL_mixer.h"
#include "SDL2/SDL_ttf.h"

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

    Texture portalTex{};

    // Particle Textures
    Texture particle{};
    Texture particleFire{};
    Texture lightTex{};
    Texture leafTex{};

    Texture coin{};
    Texture circle{};

    Texture shockwave{};

    Texture black{};
    Texture moon{};

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
    Texture uiPlay{};
    Texture logo{};
    Texture buttonMusic{};

    // Background
    Texture backdropTex{};
    Texture cloud_light{};
    Texture cloud_dark{};

    // Decor!
    Texture tileTrees{};
    Texture largeDecor{};

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
    Sound SFX_water_in{};
    Sound SFX_water_out{};
    Sound SFX_coin_collect{};
    Sound SFX_money_gain{};
    Sound SFX_fire{};
    Sound SFX_portal_0{};
    Sound SFX_portal_1{};
    Sound SFX_intro{};
    Sound SFX_button{};

    Music MUS_Menu{};
    Music MUS_Level1{};

    TTF_Font* baseFont{nullptr};
    TTF_Font* baseFontBold{nullptr};

    bool freed{false};

    TexMan()
    {
    }

    ~TexMan()
    {
        free();
    }

    void free()
    {
        if (!freed)
        {
            tileGrassTex.free();
            tileRockTex.free();
            tileSpikeTex.free();
            tileSpringTex.free();
            tileTrees.free();
            largeDecor.free();
            portalTex.free();
            particle.free();
            particleFire.free();
            lightTex.free();
            shockwave.free();
            circle.free();
            backdropTex.free();
            buttonMusic.free();
            cloud_dark.free();
            cloud_light.free();
            leafTex.free();
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
            uiPlay.free();
            logo.free();
            black.free();
            moon.free();
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
            SFX_water_in.free();
            SFX_water_out.free();
            SFX_money_gain.free();
            SFX_coin_collect.free();
            SFX_fire.free();
            SFX_portal_0.free();
            SFX_portal_1.free();
            SFX_intro.free();
            SFX_button.free();
            MUS_Level1.free();
            MUS_Menu.free();

            TTF_CloseFont(baseFont);
            TTF_CloseFont(baseFontBold);
            std::cout << "closed fonts\n";
            freed = true;
        }
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
        freed = false;
        confirm(tileGrassTex.loadFromFile("data/images/tiles/grass.png", window, renderer), success);
        confirm(tileRockTex.loadFromFile("data/images/tiles/rock.png", window, renderer), success);
        confirm(tileSpikeTex.loadFromFile("data/images/tiles/spike.png", window, renderer), success);
        confirm(tileSpringTex.loadFromFile("data/images/tiles/spring.png", window, renderer), success);
        confirm(tileTrees.loadFromFile("data/images/tiles/trees.png", window, renderer), success);
        confirm(largeDecor.loadFromFile("data/images/tiles/large_decor.png", window, renderer), success);
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
        confirm(leafTex.loadFromFile("data/images/particles/leaf.png", window, renderer), success);
        confirm(coin.loadFromFile("data/images/collectables/coin.png", window, renderer), success);
        confirm(circle.loadFromFile("data/images/particles/circle.png", window, renderer), success);
        confirm(uiPlay.loadFromFile("data/images/ui/play.png", window, renderer), success);
        confirm(shockwave.loadFromFile("data/images/vfx/shockwave.png", window, renderer), success);
        confirm(portalTex.loadFromFile("data/images/tiles/portal.png", window, renderer), success);
        confirm(logo.loadFromFile("data/images/ui/logo.png", window, renderer), success);
        confirm(backdropTex.loadFromFile("data/images/background/backdrop.png", window, renderer), success);
        confirm(cloud_dark.loadFromFile("data/images/background/clouds_dark.png", window, renderer), success);
        confirm(cloud_light.loadFromFile("data/images/background/clouds_light.png", window, renderer), success);
        confirm(black.loadFromFile("data/images/vfx/black.png", window, renderer), success);
        confirm(moon.loadFromFile("data/images/background/moon.png", window, renderer), success);
        confirm(buttonMusic.loadFromFile("data/images/ui/music.png", window, renderer), success);
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
        SFX_water_in.loadFromFileWAV("data/audio/misc/water_in.wav");
        SFX_water_out.loadFromFileWAV("data/audio/misc/water_out.wav");
        SFX_money_gain.loadFromFileWAV("data/audio/misc/money_gain.wav");
        SFX_coin_collect.loadFromFileWAV("data/audio/misc/coin_collect.wav");
        SFX_fire.loadFromFileWAV("data/audio/misc/fire.wav");
        SFX_portal_0.loadFromFileWAV("data/audio/misc/next_level_0.wav");
        SFX_portal_1.loadFromFileWAV("data/audio/misc/next_level_1.wav");
        SFX_intro.loadFromFileWAV("data/audio/misc/intro.wav");
        SFX_button.loadFromFileWAV("data/audio/misc/button.wav");
        MUS_Menu.loadFromFile("data/audio/music/menu.wav");
        MUS_Level1.loadFromFile("data/audio/music/level1.wav");

        baseFont = TTF_OpenFont("data/fonts/PixelOperator.ttf", 20);
        baseFontBold = TTF_OpenFont("data/fonts/PixelOperator-Bold.ttf", 20);
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
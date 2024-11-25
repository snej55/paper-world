#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
//#include <SDL2/SDL_mixer.h>

#include "./constants.hpp"
#include "./texture.hpp"
#include "./timer.hpp"

#include "./texman.hpp"
#include "./tiles.hpp"

class Game
{
private:
    SDL_Window* _Window {NULL};
    SDL_Renderer* _Renderer {NULL};
    Texture _Screen{};

    TexMan _TexMan{};
    World _World{};

public:
    Game()
    {
        _Window = nullptr;
        _Renderer = nullptr;
    }

    ~Game()
    {
        close();
    }

    void start()
    {
        if (!init())
        {
            std::cerr << "GAME::ERROR Failed to initialize!" << std::endl;
        } else {
            if (!loadMedia())
            {
                std::cerr << "GAME::ERROR Failed to load media!\n";
            } else {
                std::cout << "Ready!\n";
                run();
            }
        }
    }

    void close()
    {
        std::cout << "Closing\n";
        SDL_DestroyRenderer(_Renderer);
        _Renderer = NULL;
        SDL_DestroyWindow(_Window);
        _Window = NULL;
        SDL_Quit();
    }

    bool init()
    {
        std::cout << "Initializing...\n";
        bool success {true};
        if (SDL_Init(SDL_INIT_VIDEO))
        {
            std::cout << "INIT::ERROR Failed to initialize SDL! SDL_Error: " << SDL_GetError() << '\n';
            success = false;
        } else {
            _Window = SDL_CreateWindow("Paper World", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
            if (_Window == NULL)
            {
                std::cout << "INIT::ERROR Failed to create SDL_Window! SDL_Error: " << SDL_GetError() << '\n';
                success = false;
            } else {
                int imgFlags {IMG_INIT_PNG};
                if ((!(IMG_Init(imgFlags))) & imgFlags)
                {
                    std::cout << "INIT::ERROR Failed to initialize SDL_Image! SDL_Image Error: " << IMG_GetError() << '\n';
                    success = false;
                }
            }
            _Renderer = SDL_CreateRenderer(_Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (_Renderer == NULL)
            {
                std::cout << "INIT::ERROR Failed to create SDL_Renderer! SDL_Error: " << SDL_GetError() << '\n';
                success = false;
            } else {
                SDL_SetRenderDrawColor(_Renderer, 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
            }
        }
        if (success)
            std::cout << "Successfully Initialized!" << std::endl;
        return success;
    }

    bool loadMedia()
    {
        std::cout << "Loading assets...\n";
        bool success{true};
        if (!_Screen.createBlank(SCR_WIDTH, SCR_HEIGHT, _Renderer, SDL_TEXTUREACCESS_TARGET))
        {
            std::cout << "Failed to create blank target texture for screen!\n";
            success = false;
        }
        _TexMan.load(_Window, _Renderer);
        _World.loadFromFile("data/maps/0.json");
        if (success)
            std::cout << "Loaded!\n";
        return success;
    }

    void run()
    {
        SDL_Event e;
        bool running {true};

        Timer timer;
        timer.start();
        Timer fpsTimer;
        fpsTimer.start();
        float time_step {1.0};

        float frames{1.0f};
        do {
            while (SDL_PollEvent(&e) != 0)
            {
                if (e.type == SDL_QUIT)
                {
                    running = false;
                }
            }

            // calculate dt
            // timer.getTicks() and last_time are both Uint32 so must cast to float
            // divide by 1000.0f to convert from millis to sec
            // times by 60 for 60fps
            // reset last_time
            time_step = timer.getTicks() / 1000.0f * 60.0f;
            timer.start();
            // set screen as render target
            _Screen.setAsRenderTarget(_Renderer);
            // clear screen
            SDL_SetRenderDrawColor(_Renderer, 0x00, 0x00, 0x00, 0xFF);
            SDL_RenderClear(_Renderer);

            // do rendering here

            _World.render(0, 0, _Window, _Renderer, &_TexMan);

            // render screen
            SDL_SetRenderTarget(_Renderer, NULL);
            _Screen.renderClean(0, 0, _Renderer);

            SDL_RenderPresent(_Renderer);

            float avgFPS {frames / (fpsTimer.getTicks() / 1000.0f)};
            std::cout << avgFPS << '\n';

            ++frames;
        } while (running);
    }
};

#endif
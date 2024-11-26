#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
//#include <SDL2/SDL_mixer.h>

#include "./constants.hpp"
#include "./texture.hpp"
#include "./timer.hpp"
#include "./vec2.hpp"

#include "./texman.hpp"
#include "./tiles.hpp"
#include "./player.hpp"

class Game
{
private:
    SDL_Window* _Window {NULL};
    SDL_Renderer* _Renderer {NULL};
    Texture _Screen{};

    TexMan _TexMan{};
    World _World{};
    // ignore error squiggle
    Player _Player{{40.0, 40.0}, {6.0, 8.0}};

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

        int mouseX, mouseY;
        int windowX, windowY;

        vec2<double> scroll;

        float frames{1.0f};
        do {
            while (SDL_PollEvent(&e) != 0)
            {
                if (e.type == SDL_QUIT)
                {
                    running = false;
                } else if (e.type == SDL_MOUSEMOTION)
                {
                    SDL_GetGlobalMouseState(&mouseX, &mouseY);
                    mouseX -= windowX;
                    mouseY -= windowY;
                } else if (e.type == SDL_KEYDOWN)
                {
                    Controller* controller {_Player.getController()};
                    switch (e.key.keysym.sym)
                    {
                        case SDLK_UP:
                            if (!(controller->getControl(Control::UP)))
                            {
                                controller->setJumping(0.0);
                                controller->setControl(Control::UP, true);
                            }
                            break;
                        case SDLK_DOWN:
                            controller->setControl(Control::DOWN, true);
                            break;
                        case SDLK_LEFT:
                            controller->setControl(Control::LEFT, true);
                            break;
                        case SDLK_RIGHT:
                            controller->setControl(Control::RIGHT, true);
                            break;
                        default:
                            break;
                    }
                } else if (e.type == SDL_KEYUP)
                {
                    Controller* controller {_Player.getController()};
                    switch (e.key.keysym.sym)
                    {
                        case SDLK_UP:
                            controller->setControl(Control::UP, false);
                            break;
                        case SDLK_DOWN:
                            controller->setControl(Control::DOWN, false);
                            break;
                        case SDLK_LEFT:
                            controller->setControl(Control::LEFT, false);
                            break;
                        case SDLK_RIGHT:
                            controller->setControl(Control::RIGHT, false);
                            break;
                        default:
                            break;
                    }
                }
            }

            SDL_GetWindowPosition(_Window, &windowX, &windowY);

            // calculate dt
            // timer.getTicks() and last_time are both Uint32 so must cast to float
            // divide by 1000.0f to convert from millis to sec
            // times by 60 for 60fps
            // reset last_time
            time_step = timer.getTicks() / 1000.0f * 60.0f;
            timer.start();
            // set screen as render target
            _Screen.setAsRenderTarget(_Renderer);
            // clear screen (0x1f, 0x24, 0x4b)
            SDL_SetRenderDrawColor(_Renderer, 0x00, 0x00, 0x00, 0xFF);
            SDL_RenderClear(_Renderer);

            // do updateing here
            vec2<int> render_scroll{static_cast<int>(scroll.x), static_cast<int>(scroll.y)};
            _Player.update(time_step, _World);
            // do rendering here

            
            _World.render(render_scroll.x, render_scroll.y, _Window, _Renderer, &_TexMan);
            _Player.render(render_scroll.x, render_scroll.y, _Renderer);
            /*std::array<SDL_Rect, 9> rects;
            vec2<double> pos {(double)mouseX / 2, (double)mouseY / 2};
            _World.getTilesAroundPos(pos, rects);
            for (int i{0}; i < 9; ++i)
            {
                SDL_SetRenderDrawColor(_Renderer, 0xFF, 0x00, 0x00, 0xFF);
                SDL_RenderFillRect(_Renderer, &(rects[i]));
            }*/
            // render screen
            SDL_SetRenderTarget(_Renderer, NULL);
            _Screen.renderClean(0, 0, _Renderer, 2);

            SDL_RenderPresent(_Renderer);

            float avgFPS {frames / (fpsTimer.getTicks() / 1000.0f)};
            //std::cout << avgFPS << '\n';

            ++frames;
        } while (running);
    }
};

#endif
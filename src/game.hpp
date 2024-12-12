// TODO:
// Attacks & Health Bars :( Boring
// Sparks?! Yay!
// Grass! :) Yay!
// Water! :) Yay!

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
#include "./particles.hpp"
#include "./entities.hpp"

constexpr SDL_Color PALETTE[8] {{0xa8, 0x60, 0x5d}, {0xd1, 0xa6, 0x7e}, {0xf6, 0xe7, 0x9c}, {0xb6, 0xcf, 0x8e}, {0x60, 0xae, 0x7b}, {0x3c, 0x6b, 0x64}, {0x1f, 0x24, 0x4b}, {0x65, 0x40, 0x53}};

class Game
{
private:
    SDL_Window* _Window {NULL};
    SDL_Renderer* _Renderer {NULL};
    Texture _Screen{};

    TexMan _TexMan{};
    World _World{};
    Player _Player{{40.0, 40.0}, {4, 8}};
    EMManager _EMManager{}; // this is the entity manager :)

    int _Width {SCR_WIDTH};
    int _Height {SCR_HEIGHT};

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
            _Window = SDL_CreateWindow("Paper World", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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
            _Renderer = SDL_CreateRenderer(_Window, -1, SDL_RENDERER_ACCELERATED);// | SDL_RENDERER_PRESENTVSYNC);
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
        _EMManager.loadFromPath("data/maps/0.json", &_TexMan);
        _Player.loadAnim(&_TexMan);
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
        double time_step {1.0};

        int mouseX, mouseY;
        int windowX, windowY;

        vec2<double> scroll;

        double screen_shake{0};

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
                        case SDLK_p:
                            //Entity* _Entity = new Entity{{static_cast<double>(std::rand() % 100 + 50), 20.0}, {0.0, 0.0}, {8, 8}, 0.2, false, "default"};
                            //_EMManager.addEntity(_Entity);
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
                } else if (e.type == SDL_WINDOWEVENT)
                {
                    // handle window events
                    switch (e.window.event)
                    {
                        case (SDL_WINDOWEVENT_RESIZED):
                            _Width = e.window.data1 / 3;
                            _Height = e.window.data2 / 3;
                            _Screen.free();
                            _Screen.createBlank(_Width, _Height, _Renderer, SDL_TEXTUREACCESS_TARGET);
                            SDL_RenderPresent(_Renderer);
                            break;
                        case (SDL_WINDOWEVENT_SIZE_CHANGED):
                            _Width = e.window.data1 / 3;
                            _Height = e.window.data2 / 3;
                            _Screen.free();
                            _Screen.createBlank(_Width, _Height, _Renderer, SDL_TEXTUREACCESS_TARGET);
                            SDL_RenderPresent(_Renderer);
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
            time_step = timer.getTicks() / 1000.0 * 60.0;
            time_step = std::min(time_step, 3.0);
            timer.start();
            // set screen as render target
            _Screen.setAsRenderTarget(_Renderer);
            // clear screen (0x1f, 0x24, 0x4b)
            SDL_SetRenderDrawColor(_Renderer, 0x00, 0x00, 0x00, 0xFF);
            SDL_RenderClear(_Renderer);

            // do updateing here
            vec2<double> player_pos{_Player.getPos()};
            if (_Player.getAd() > 120)
            {
                scroll.x += (player_pos.x - static_cast<double>(_Width) / 2.0 - scroll.x) / 40.0 * time_step;
                scroll.y += (player_pos.y - static_cast<double>(_Height) / 2.0 - scroll.y) / 50.0 * time_step;
            }
            _Player.update(time_step, _World, &screen_shake);
            _Player.tickAd(time_step);

            _EMManager.update(time_step, _World, &screen_shake, &_Player);
            // do rendering here

            screen_shake = std::max(0.0, screen_shake - time_step);
            vec2<int> render_scroll{static_cast<int>(scroll.x + Util::random() * screen_shake - screen_shake / 2.0), static_cast<int>(scroll.y + Util::random() * screen_shake - screen_shake / 2.0)};
            // fairly obvious what this does
            _World.handleSprings(time_step);
            _World.render(render_scroll.x, render_scroll.y, _Window, _Renderer, &_TexMan, _Width, _Height);
            _EMManager.render(render_scroll.x, render_scroll.y, _Renderer, time_step, &_World, &_TexMan);
            // check if the player is not dead. ad stands for 'after death'
            if (_Player.getAd() > 120)
                _Player.render(render_scroll.x, render_scroll.y, _Renderer);
            _Player.updateParticles(time_step, render_scroll.x, render_scroll.y, _Renderer, &_World, &_TexMan);
            /*std::array<SDL_Rect, 9> rects;
            vec2<double> pos {(double)mouseX / 2 + scroll.x, (double)mouseY / 2 + scroll.y};
            _World.getDangerAroundPos(pos, rects);
            for (int i{0}; i < 9; ++i)
            {
                SDL_SetRenderDrawColor(_Renderer, 0xFF, 0x00, 0x00, 0xFF);
                rects[i].x -= scroll.x;
                rects[i].y -= scroll.y;
                SDL_RenderFillRect(_Renderer, &(rects[i]));
            }*/
           renderPlayerHealthBar();
            // render screen
            SDL_SetRenderTarget(_Renderer, NULL);
            _Screen.renderClean(0, 0, _Renderer, 3);

            SDL_RenderPresent(_Renderer);

            float avgFPS {frames / (fpsTimer.getTicks() / 1000.0f)};
            setWindowTitle(avgFPS);
            ++frames;
        } while (running);
    }

    void setWindowTitle(float avgFPS)
    {
        std::stringstream caption;
        caption << "Paper World at " << avgFPS << " FPS (Avg)";
        SDL_SetWindowTitle(_Window, caption.str().c_str());
    }

    void renderPlayerHealthBar()
    {
        SDL_SetRenderDrawColor(_Renderer, 0x1f, 0x24, 0x4b, 0xFF);
        SDL_Rect fillRect {4, 4, 100, 10};
        SDL_RenderFillRect(_Renderer, &fillRect);
        SDL_SetRenderDrawColor(_Renderer, 0xb6, 0xcf, 0x8e, 0xFF);
        fillRect = SDL_Rect{5, 5, static_cast<int>(98.0 * _Player.getHealth() / 100.0), 4};
        SDL_RenderFillRect(_Renderer, &fillRect);
        SDL_SetRenderDrawColor(_Renderer, 0x3c, 0x6b, 0x64, 0xFF);
        fillRect = SDL_Rect{5, 9, static_cast<int>(98.0 * _Player.getHealth() / 100.0), 4};
        SDL_RenderFillRect(_Renderer, &fillRect);
    }
};

#endif
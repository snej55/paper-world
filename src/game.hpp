#ifndef GAME_H
#define GAME_H

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_mixer.h"
#include "SDL2/SDL_ttf.h"

#include "JSON/json.hpp"

#include "./constants.hpp"
#include "./texture.hpp"
#include "./timer.hpp"
#include "./vec2.hpp"

#include "./texman.hpp"
#include "./tiles.hpp"
#include "./player.hpp"
#include "./particles.hpp"
#include "./entities.hpp"
#include "./sparks.hpp"
#include "./water.hpp"
#include "./coin.hpp"
#include "./buttons.hpp"
#include "./shockwaves.hpp"
#include "./stars.hpp"
#include "./audio.hpp"
#include "./popups.hpp"
// #include "./clouds.hpp"

using json = nlohmann::json;

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
    EMManager _EMManager{}; // this is the entity manager :) "The Manager of the Managers"
    WaterManager* _WaterManager{nullptr};
    LavaManager* _LavaManager{nullptr};
    CoinManager _CoinManager{};
    ShockWaveManager _ShockWaveManager{};
    StarManager _StarManager{100};
    PopUpManager _PopUpManager{};

    std::vector<std::string> _levels {"data/maps/0.json", "data/maps/1.json", "data/maps/2.json"}, "data/maps/4.json", "data/maps/5.json", "data/maps/6.json", "data/maps/7.json", "data/maps/8.json", "data/maps/9.json", "data/maps/10.json", "data/maps/11.json", "data/maps/12.json", "data/maps/13.json"};
    int _level{0};

    int _Width {SCR_WIDTH};
    int _Height {SCR_HEIGHT};

    Music* _Music{nullptr};

    SDL_Surface* _windowIcon{nullptr};

    bool _closed{false};

    bool _volume{true};

    double _playerHealth{100.0};
    vec2<double> _portal_pos{0.0, 0.0};

public:
    Game()
    {
        _Window = nullptr;
        _Renderer = nullptr;
    }

    ~Game()
    {
        if (!_closed)
        {
            Close();
        }
    }

    void start()
    {
        if (!Init())
        {
            std::cerr << "GAME::ERROR Failed to initialize!" << std::endl;
        } else {
            if (!loadMedia())
            {
                std::cerr << "GAME::ERROR Failed to load media!\n";
            } else {
                std::cout << "Ready!\n";
                bool playAgain{true};
                while (playAgain)
                {
                    if (!menu())
                    {
                        playAgain = run();
                        reset();
                    } else {
                        playAgain = false;
                    }
                }
            }
        }
    }

    void Close()
    {
        delete _WaterManager;
        delete _LavaManager;
        _TexMan.free();
        std::cout << "Closing\n";
        SDL_DestroyRenderer(_Renderer);
        std::cout << "Destroyed renderer!\n";
        _Renderer = NULL;
        SDL_DestroyWindow(_Window);
        std::cout << "Destroyed window!\n";
        _Window = NULL;
        std::cout << "Closed\n";
        _closed = true;
    }

    bool Init()
    {
        std::cout << "Initializing...\n";
        bool success {true};
        _Window = SDL_CreateWindow("Defblade", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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
            
            // Initialize SDL_ttf
            if (TTF_Init() == -1)
            {
                std::cout << "INIT::ERROR Failed to initialize SDL_ttf! SDL_ttf Error: " << TTF_GetError() << '\n';
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
        if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 )
        {
            success = false;
        }
        if (success)
            std::cout << "Successfully Initialized!" << std::endl;
        return success;
    }

    void loadLevel(int level)
    {
        std::string path{_levels[level % static_cast<int>(_levels.size())]};
        _World.loadFromFile(path.c_str());
        std::cout << "loaded world!\n";
        _EMManager.loadFromPath(path, &_TexMan);
        std::cout << "loaded entities!\n";

        // Water & Lava
        if (_WaterManager == nullptr)
        {
            _WaterManager = new WaterManager{};
        }
        if (_LavaManager == nullptr)
        {
            _LavaManager = new LavaManager{};
        }
        _WaterManager->loadFromFile(path.c_str());
        std::cout << "loaded water!\n";
        _LavaManager->loadFromFile(path.c_str());
        std::cout << "loaded lava\n";
        _CoinManager.free();
        setPlayerSpawnPos(path.c_str());
    }

    void setPlayerSpawnPos(const char* path)
    {
        std::ifstream f{path};
        json data = json::parse(f);
        _Player.setSpawnPos({static_cast<double>(data["player_spawn_pos"][0]), static_cast<double>(data["player_spawn_pos"][1])});
        _portal_pos = vec2<double>{static_cast<double>(data["portal_pos"][0]), static_cast<double>(data["portal_pos"][1])};
    }

    void reset()
    {
        _level = 0;
        _Player.reset();
        loadLevel(_level);
        _CoinManager.setScore(START_COINS);
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
        _Player.loadAnim(&_TexMan);
        _ShockWaveManager.setTex(&(_TexMan.shockwave));

        _windowIcon = IMG_Load("data/images/ui/icon.png");
        SDL_SetWindowIcon(_Window, _windowIcon);

        // _World.loadFromFile("data/maps/1.json");
        // setPlayerSpawnPos("data/maps/1.json");
        // _EMManager.loadFromPath("data/maps/1.json", &_TexMan);
        // _WaterManager = new WaterManager{};
        // _WaterManager->loadFromFile("data/maps/1.json");
        // _LavaManager = new LavaManager{};
        // _LavaManager->loadFromFile("data/maps/1.json");
        loadLevel(_level);
        
        _Music = &(_TexMan.MUS_Menu);
        //loadLevel(0);
        _StarManager.setTex(&(_TexMan.lightTex));

        _CoinManager.setTex(&(_TexMan.coin), &(_TexMan.lightTex));

        if (success)
            std::cout << "Loaded!\n";
        return success;
    }

    void nextLevel()
    {
        ++_level;
        loadLevel(_level);
    }

    bool run()
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

        vec2<double> player_pos{_Player.getCenter()};
        vec2<double> scroll{player_pos.x - static_cast<double>(_Width) / 2.0, player_pos.y - static_cast<double>(_Height) / 2.0};

        _CoinManager.setScore(START_COINS);

        double screen_shake{0};
        double slomo{1.0};

        float frames{1.0f};

        float last_damaged{100.0f};

        double last_coin{0.0};

        double fade{static_cast<double>(_Height * 3)};
        bool fading{false};
        bool changing{false};
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
                        case SDLK_w:
                            if (!(controller->getControl(Control::UP)))
                            {
                                controller->setJumping(0.0);
                                controller->setControl(Control::UP, true);
                            }
                            break;
                        case SDLK_s:
                            controller->setControl(Control::DOWN, true);
                            break;
                        case SDLK_a:
                            controller->setControl(Control::LEFT, true);
                            break;
                        case SDLK_d:
                            controller->setControl(Control::RIGHT, true);
                            break;
                        case SDLK_p:
                            //Entity* _Entity = new Entity{{static_cast<double>(std::rand() % 100 + 50), 20.0}, {0.0, 0.0}, {8, 8}, 0.2, false, "default"};
                            //_EMManager.addEntity(_Entity);
                            //fading = !fading;
                            _ShockWaveManager.addShockWave(_Player.getCenter());
                            break;
                        case SDLK_x:
                            _Player.attackSword(&_TexMan);
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
                        case SDLK_w:
                            controller->setControl(Control::UP, false);
                            break;
                        case SDLK_s:
                            controller->setControl(Control::DOWN, false);
                            break;
                        case SDLK_a:
                            controller->setControl(Control::LEFT, false);
                            break;
                        case SDLK_d:
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
            time_step = timer.getTicks() / 1000.0 * 60.0 * slomo;
            time_step = std::min(time_step, 3.0);
            timer.start();

            slomo += (1.0 - slomo) / 20.0 * (time_step / slomo);
            // set screen as render target
            _Screen.setAsRenderTarget(_Renderer);
            // clear screen (0x1f, 0x24, 0x4b)
            SDL_SetRenderDrawColor(_Renderer, 0x00, 0x00, 0x00, 0xFF);
            SDL_RenderClear(_Renderer);

            SDL_SetRenderDrawBlendMode(_Renderer, SDL_BLENDMODE_NONE);

            // SDL_Rect stretchRect{0, 0, _Width, _Height};
            // SDL_RenderCopyEx(_Renderer, _TexMan.black.getTexture(), NULL, &stretchRect, 0, NULL, SDL_FLIP_NONE);

            // do updateing here
            vec2<double> player_pos{_Player.getCenter()};
            if (_Player.getAd() > 120)
            {
                scroll.x += (player_pos.x - static_cast<double>(_Width) / 2.0 - scroll.x) / 40.0 * time_step;
                scroll.y += (player_pos.y - static_cast<double>(_Height) / 2.0 - scroll.y) / 50.0 * time_step;
            }
            scroll.x = std::max(static_cast<double>(TILE_SIZE), std::min(scroll.x, static_cast<double>(LEVEL_WIDTH * CHUNK_SIZE * TILE_SIZE - TILE_SIZE - _Width)));
            scroll.y = std::max(0.0, std::min(scroll.y, static_cast<double>(LEVEL_HEIGHT * CHUNK_SIZE * TILE_SIZE - _Height)));
            _Player.update(time_step, _World, &screen_shake, &_TexMan, _ShockWaveManager);
            if (_Player.getAd() == 0 && !changing)
            {
                _TexMan.SFX_death_0.play();
                _TexMan.SFX_sword_slash.play();
                last_coin = -1;
                int num{(std::rand() % 10) + 5};
                for (int i{0}; i < num; ++i)
                {
                    _CoinManager.addCoin(_Player.getLastPos(), {Util::random() * 2.0 - 1.0, Util::random() * -1.0});
                    _CoinManager.setScore(_CoinManager.getScore() - (static_cast<int>(Util::random() * 5.0) + 10));
                }
                _TexMan.SFX_money_gain.play();
            }
            _Player.tickAd(time_step);

            _EMManager.update(time_step, _World, &screen_shake, &_Player, &slomo, &_TexMan, &_CoinManager, _ShockWaveManager);
            // do rendering here

            screen_shake = std::max(0.0, screen_shake - time_step);
            vec2<int> render_scroll{static_cast<int>(scroll.x + Util::random() * screen_shake - screen_shake / 2.0), static_cast<int>(scroll.y + Util::random() * screen_shake - screen_shake / 2.0)};

            _StarManager.update(time_step, render_scroll.x, render_scroll.y, _Width, _Height, _Renderer);
            _TexMan.moon.render(_Width - 32, 32, _Renderer);

            // _TexMan.black.setAlpha(150);
            // _TexMan.black.setBlendMode(SDL_BLENDMODE_BLEND);
            // SDL_RenderCopyEx(_Renderer, _TexMan.black.getTexture(), NULL, &stretchRect, 0, NULL, SDL_FLIP_NONE);

            // fairly obvious what this does
            _World.handleSprings(time_step);
            _World.render(render_scroll.x, render_scroll.y, _Window, _Renderer, &_TexMan, _Width, _Height);
            _World.handleGrass(render_scroll.x, render_scroll.y, _Renderer, &_TexMan, _Width, _Height, _Player.getRect(), time_step);
            _EMManager.render(render_scroll.x, render_scroll.y, _Renderer, time_step, &_World, &_TexMan);
            // check if the player is not dead. ad stands for 'after death'
            if (_Player.getAd() > 120)
                _Player.render(render_scroll.x, render_scroll.y, _Renderer);
            
            if (_Player.getShouldDamage()) // do it before we reset it in Player.updateParticles();
            {
                last_damaged = 1.0f;
            }
            last_damaged += 0.03f;

            // handle portal
            _TexMan.portalTex.render(static_cast<int>(_portal_pos.x) - render_scroll.x, static_cast<int>(_portal_pos.y + std::sin(fpsTimer.getTicks() * 0.001) * 4.0) - render_scroll.y, _Renderer);
            SDL_Rect portalRect{static_cast<int>(_portal_pos.x), static_cast<int>(_portal_pos.y + std::sin(fpsTimer.getTicks() * 0.001) * 4.0), 13, 21};
            if (Util::checkCollision(_Player.getRect(), &portalRect))
            {
                if (!changing)
                {
                    changing = true;
                    screen_shake = std::max(screen_shake, 32.0);
                    _TexMan.SFX_portal_0.play();
                    // _Player.setAd(1.0);
                    fading = true;
                }
            }

            _World.updateLeaves(time_step, render_scroll.x, render_scroll.y, _Width, _Height, &_TexMan, _Renderer);
            _Player.updateParticles(time_step, render_scroll.x, render_scroll.y, _Renderer, &_World, &_TexMan);
            // for testing
            _CoinManager.update(time_step, render_scroll.x, render_scroll.y, _Renderer, &_World, &_TexMan, _Player.getRect(), last_coin);
            _WaterManager->update(time_step, render_scroll.x, render_scroll.y, _Renderer, &_TexMan, &_Player);
            _LavaManager->update(time_step, render_scroll.x, render_scroll.y, _Renderer, &_TexMan, &_Player);
            _ShockWaveManager.update(time_step, render_scroll.x, render_scroll.y, _Renderer);

            _playerHealth += (_Player.getHealth() - _playerHealth) * 0.12 * time_step;
            if (_Player.getHealth() == _Player.getMaxHealth())
            {
                _playerHealth = _Player.getHealth();
            }
            SDL_Rect uiRect{0, 0, _Width, 14};
            SDL_SetRenderDrawColor(_Renderer, 0x1f, 0x24, 0x4b, 0xFF);
            SDL_RenderFillRect(_Renderer, &uiRect);
            SDL_SetRenderDrawColor(_Renderer, 0xF6, 0xe7, 0x9c, 0xFF);
            SDL_RenderDrawLine(_Renderer, 0, 14, _Width, 14);
            renderPlayerHealthBar();
            if (last_damaged < 10.0f)
            {
                float offset {last_damaged * last_damaged};
                SDL_Rect alert{2 - static_cast<int>(offset), 2 - static_cast<int>(offset), _Width - 4 + static_cast<int>(offset) * 2, _Height - 4 + static_cast<int>(offset) * 2};
                SDL_SetRenderDrawBlendMode(_Renderer, SDL_BLENDMODE_BLEND);
                SDL_SetRenderDrawColor(_Renderer, 0xFF, 0x00, 0x00, static_cast<int>(255.0 - _Player.getHealth() / _Player.getMaxHealth() * 200.0));
                SDL_RenderDrawRect(_Renderer, &alert);
                SDL_SetRenderDrawColor(_Renderer, 0xFF, 0xFF, 0xFF, static_cast<int>(255.0 - _Player.getHealth() / _Player.getMaxHealth() * 200.0));
                alert = SDL_Rect{1 - static_cast<int>(offset), 1 - static_cast<int>(offset), _Width - 2 + static_cast<int>(offset) * 2, _Height - 2 + static_cast<int>(offset) * 1};
                SDL_RenderDrawRect(_Renderer, &alert);
                SDL_SetRenderDrawBlendMode(_Renderer, SDL_BLENDMODE_NONE);
            }

            // Just testing lol
            // SDL_Color col{0x00, 0xaa, 0xFF, 0xaa};
            // std::vector<SDL_Vertex> vertices
            // {
            //     {{0.0f, 0.0f}, col, {0.0f, 0.0f}},
            //     {{50.0f, 20.0f}, col, {0.5f, 0.0f}},
            //     {{100.0f, 0.0f}, col, {1.0f, 0.0f}},
            //     {{0.0f, 100.0f}, col, {0.0f, 1.0f}},
            //     {{50.0f, 100.0f}, col, {0.5f, 1.0f}},
            //     {{100.0f, 100.0f}, col, {1.0f, 1.0f}}
            // };
            // std::vector<int> indices{Util::get_water_indices<SDL_Vertex>(vertices)};
            // Polygons::renderPolygon(_Renderer, _TexMan.particle.getTexture(), vertices, indices);
            // render screen
            SDL_SetRenderTarget(_Renderer, NULL);
            _Screen.renderClean(0, 0, _Renderer, 3);
            Texture fontTex{};
            std::stringstream text{};
            text << static_cast<int>(_playerHealth);
            text << "/";
            text << static_cast<int>(_Player.getMaxHealth());
            fontTex.loadFromRenderedText(text.str().c_str(), {0xF6, 0xe7, 0x9c, 0xFF}, _TexMan.baseFontBold, _Renderer);
            fontTex.render(110, 10, _Renderer);
            std::stringstream score{};
            score << "$" << _CoinManager.getScore();

            last_coin += (0.0 - last_coin) * 0.01 * time_step;

            SDL_Color moneyBaseColor{0xF6, 0xe7, 0x9c, 0xFF};
            SDL_Color moneyRedColor{0xa8, 0x60, 0x5d, 0xFF};
            SDL_Color moneyGreenColor{0x60, 0xae, 0x7b, 0xFF};
            SDL_Color moneyBlend{Util::lerpColor(moneyBaseColor, moneyRedColor, std::abs(std::min(0.0, last_coin)))};
            SDL_Color moneyColor{Util::lerpColor(moneyBlend, moneyGreenColor, std::max(0.0, last_coin))};

            fontTex.loadFromRenderedText(score.str().c_str(), moneyColor, _TexMan.baseFontBold, _Renderer);
            fontTex.render(static_cast<int>((double)_Width * 3.0 / 2.0), 10, _Renderer);

            if (fading)
            {
                fade = std::max(-10.0, std::min(static_cast<double>(_Height) * 3.0 + 10.0, fade + time_step * static_cast<double>(_Height * 3) / 60.0));
            } else {
                fade = std::max(-10.0, std::min(static_cast<double>(_Height) * 3.0 + 10.0, fade - time_step * static_cast<double>(_Height * 3) / 60.0));
            }
            SDL_Rect fadeRect{0, 0, _Width * 3, static_cast<int>(fade)};
            SDL_SetRenderDrawColor(_Renderer, 0x1f, 0x24, 0x4b, 0xFF);
            SDL_RenderFillRect(_Renderer, &fadeRect);

            if (changing)
            {
                if (fade > static_cast<double>(_Height) * 3.0 + 5.0)
                {
                    fading = false;
                    changing = false;
                    _TexMan.SFX_portal_0.play();
                    nextLevel();
                    scroll = {_Player.getCenter().x - static_cast<double>(_Width) / 2.0, _Player.getCenter().y - static_cast<double>(_Height) / 2.0};
                    _Player.setHealth(_Player.getMaxHealth());
                    if (checkWin())
                    {
                        bool play_again{win()};
                        running = false;
                        return play_again;
                    }
                }
            }

            std::stringstream levelText{};
            levelText << "Level " << _level + 1;
            fontTex.loadFromRenderedText(levelText.str().c_str(), {0xF6, 0xe7, 0x9c, 0xFF}, _TexMan.baseFontBold, _Renderer);
            fontTex.render(static_cast<int>((double)_Width * 3.0 / 2.0) - fontTex.getWidth() / 2, std::min(_Height * 3 / 2, static_cast<int>(fade) - 32), _Renderer);

            SDL_RenderPresent(_Renderer);

            float avgFPS {frames / (fpsTimer.getTicks() / 1000.0f)};
            setWindowTitle(avgFPS); 
            ++frames;
            if (_CoinManager.getScore() < 0)
            {
                bool play_again{gameover()};
                running = false;
                return play_again;
            }
            handleVolume();
        } while (running);
        return false;
    }

    void setWindowTitle(float avgFPS)
    {
        std::stringstream caption;
        caption << "Defblade";
        SDL_SetWindowTitle(_Window, caption.str().c_str());
    }

    void renderPlayerHealthBar()
    {
        _TexMan.playerHealthBar.render(2, 3, _Renderer, nullptr);
        SDL_Color greenDark{0x32, 0x6b, 0x64, 0xFF};
        SDL_Color greenLight{0x60, 0xae, 0x7b, 0xFF};
        SDL_Color redDark{0xa8, 0x60, 0x5d, 0xFF};
        SDL_Color redLight{0xd1, 0xa7, 0x7e, 0xFF};
        SDL_Color lightColor {Util::lerpColor(redLight, greenLight, _playerHealth / _Player.getMaxHealth())};
        SDL_Color darkColor {Util::lerpColor(redDark, greenDark, _playerHealth / _Player.getMaxHealth())};
        SDL_SetRenderDrawColor(_Renderer, lightColor.r, lightColor.g, lightColor.b, 0xFF);
        SDL_Rect fillRect = SDL_Rect{5, 5, static_cast<int>(26.0 * _playerHealth / _Player.getMaxHealth()), 2};
        SDL_RenderFillRect(_Renderer, &fillRect);
        SDL_SetRenderDrawColor(_Renderer, darkColor.r, darkColor.g, darkColor.b, 0xFF);
        fillRect = SDL_Rect{5, 7, static_cast<int>(26.0 * _playerHealth / _Player.getMaxHealth()), 2};
        SDL_RenderFillRect(_Renderer, &fillRect);
    }

    bool checkWin()
    {
        if (_level >= _levels.size())
        {
            return true;
        }
        return false;
    }

    bool menu()
    {
        SDL_Event e;
        bool running {true};

        Timer timer;
        timer.start();
        double time_step {1.0};

        int mouseX, mouseY;
        int windowX, windowY;

        Timer animTimer{};
        animTimer.start();
        bool logo_shown{false};
        bool play_shown{false};
        bool music_shown{false};

        Timer popUpTimer{};
        popUpTimer.start();

        Button playButton{{0, 0}, &(_TexMan.uiPlay)};
        Button musicButton{{0, 0}, &(_TexMan.buttonMusic)};

        _Music->stop();
        _Music = &(_TexMan.MUS_Menu);

        do {
            while (SDL_PollEvent(&e) != 0)
            {
                if (e.type == SDL_QUIT)
                {
                    return true;
                } else if (e.type == SDL_MOUSEMOTION)
                {
                    SDL_GetGlobalMouseState(&mouseX, &mouseY);
                    mouseX -= windowX;
                    mouseY -= windowY;
                } else if (e.type == SDL_KEYDOWN)
                {
                    switch (e.key.keysym.sym)
                    {
                        case (SDLK_RETURN):
                            running = false;
                        default:
                            break;
                    }
                } else if (e.type == SDL_KEYUP)
                {
                    switch (e.key.keysym.sym)
                    {
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
                } else if (e.type == SDL_MOUSEBUTTONDOWN)
                {
                    SDL_Rect mouseRect{mouseX / 3, mouseY / 3, 1, 1};
                    if (Util::checkCollision(&mouseRect, playButton.getRect()))
                    {
                        running = false;
                    }
                    if (Util::checkCollision(&mouseRect, musicButton.getRect()))
                    {
                        _volume = !_volume;
                        std::cout << _volume << "\n";
                    }
                }
            }

            time_step = timer.getTicks() / 1000.0 * 60.0;
            time_step = std::min(time_step, 3.0);
            timer.start();


            SDL_GetWindowPosition(_Window, &windowX, &windowY);
            // set screen as render target
            _Screen.setAsRenderTarget(_Renderer);

            // clear screen (0x1f, 0x24, 0x4b)
            SDL_SetRenderDrawColor(_Renderer, 0x00, 0x00, 0x00, 0xFF);
            SDL_RenderClear(_Renderer);
            
            if (!logo_shown)
            {
                if (animTimer.getTicks() > 1000)
                {
                    logo_shown = true;
                    _TexMan.SFX_intro.play();
                }
            } else {
                _TexMan.logo.render(_Width / 2 - 60, _Height / 2 - 60, _Renderer);
            }

            if (animTimer.getTicks() > 3000)
            {
                _Music->play();
            }
            
            if (play_shown)
            {
                playButton.setPos(vec2<int>{_Width / 2 - 35, _Height / 2 - 5});
                playButton.update(time_step, mouseX / 3, mouseY / 3, _Renderer);
                SDL_Rect mouseRect{mouseX / 3, mouseY / 3, 1, 1};
                if (popUpTimer.getTicks() > 700)
                {
                    popUpTimer.start();
                    std::vector<std::string> phrases {"Certain death!", "AAAARRGGHH!!!", "Beware!", "Pain!", "Enter at own risk!", "NOOO!", "Help!", "You will die.", "RUN!", "You will NOT survive.", "Don't click it!", "Uh Oh.", "Save yourself!", "Bolt!", "Scram!", "Flee!", "Escape!", "Retreat!", "Stay away!", "Die!", "Steer Clear!", "2.5m Social Distancing please!", "Don't touch this!"};
                    _PopUpManager.addPopUp({static_cast<double>(_Width * 2) * Util::random() + static_cast<double>(_Width / 6), static_cast<double>(_Height * 3 - 100)}, phrases[std::rand() % static_cast<int>(phrases.size())]);
                }
            } else {
                playButton.setExpand(-35.0);
                if (animTimer.getTicks() > 2000)
                {
                    play_shown = true;
                    _TexMan.SFX_button.play();
                }
            }

            if (music_shown)
            {
                musicButton.setPos(vec2<int>{_Width - 35, _Height - 100 / 3});
                musicButton.update(time_step, 0, 0, _Renderer);
            } else {
                musicButton.setExpand(-35.0);
                if (animTimer.getTicks() > 3000)
                {
                    music_shown = true;
                    _TexMan.SFX_button.play();
                }
            }

            SDL_SetRenderTarget(_Renderer, NULL);
            _Screen.renderClean(0, 0, _Renderer, 3);

            _PopUpManager.update(time_step, _Renderer, _TexMan.baseFontBold);

            Texture fontTex{};
            double faded{static_cast<double>(std::min(static_cast<Uint32>(5000), animTimer.getTicks())) / 5000.0};
            fontTex.loadFromRenderedText("A game by @snej55", SDL_Color{static_cast<Uint8>(static_cast<int>(faded * 246.0)), static_cast<Uint8>(static_cast<int>(faded * 231.0)), static_cast<Uint8>(static_cast<int>(faded * 156.0)), 0xFF}, _TexMan.baseFontBold, _Renderer);
            fontTex.render(static_cast<int>((double)_Width * 3.0 / 2.0) - fontTex.getWidth() / 2, _Height * 3 - 50, _Renderer);

            SDL_RenderPresent(_Renderer);
            handleVolume();
        } while (running);
        _TexMan.SFX_portal_0.play();
        _Music->stop();
        _Music = &(_TexMan.MUS_Level1);
        _Music->play();
        return false;
    }

    bool gameover()
    {
        SDL_Event e;
        bool running {true};

        Timer timer;
        timer.start();
        double time_step {1.0};

        int mouseX, mouseY;
        int windowX, windowY;

        Timer animTimer{};
        animTimer.start();
        bool play_shown{false};

        Button playButton{{0, 0}, &(_TexMan.uiPlay)};

        bool playAgain{false};

        do {
            while (SDL_PollEvent(&e) != 0)
            {
                if (e.type == SDL_QUIT)
                {
                    return false;
                } else if (e.type == SDL_MOUSEMOTION)
                {
                    SDL_GetGlobalMouseState(&mouseX, &mouseY);
                    mouseX -= windowX;
                    mouseY -= windowY;
                } else if (e.type == SDL_KEYDOWN)
                {
                    switch (e.key.keysym.sym)
                    {
                        case (SDLK_RETURN):
                            running = false;
                            playAgain = true;
                        default:
                            break;
                    }
                } else if (e.type == SDL_KEYUP)
                {
                    switch (e.key.keysym.sym)
                    {
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
                } else if (e.type == SDL_MOUSEBUTTONDOWN)
                {
                    SDL_Rect mouseRect{mouseX / 3, mouseY / 3, 1, 1};
                    if (Util::checkCollision(&mouseRect, playButton.getRect()))
                    {
                        playAgain = true;
                        running = false;
                    }
                }
            }

            time_step = timer.getTicks() / 1000.0 * 60.0;
            time_step = std::min(time_step, 3.0);
            timer.start();

            SDL_GetWindowPosition(_Window, &windowX, &windowY);
            // set screen as render target
            _Screen.setAsRenderTarget(_Renderer);

            // clear screen (0x1f, 0x24, 0x4b)
            SDL_SetRenderDrawColor(_Renderer, 0x00, 0x00, 0x00, 0xFF);
            SDL_RenderClear(_Renderer);
            
            if (play_shown)
            {
                playButton.setPos(vec2<int>{_Width / 2 - 35, _Height / 2 - 5});
                playButton.update(time_step, mouseX / 3, mouseY / 3, _Renderer);
            } else {
                playButton.setExpand(-35.0);
                if (animTimer.getTicks() > 2000)
                {
                    play_shown = true;
                    _TexMan.SFX_button.play();
                }
            }

            SDL_SetRenderTarget(_Renderer, NULL);
            _Screen.renderClean(0, 0, _Renderer, 3);

            Texture fontTex{};
            double faded{static_cast<double>(std::min(static_cast<Uint32>(5000), animTimer.getTicks())) / 5000.0};
            fontTex.loadFromRenderedText("A game by that guy -> @snej55", SDL_Color{static_cast<Uint8>(static_cast<int>(faded * 246.0)), static_cast<Uint8>(static_cast<int>(faded * 231.0)), static_cast<Uint8>(static_cast<int>(faded * 156.0)), 0xFF}, _TexMan.baseFontBold, _Renderer);
            fontTex.render(static_cast<int>((double)_Width * 3.0 / 2.0) - fontTex.getWidth() / 2, _Height * 3 - 50, _Renderer);

            fontTex.loadFromRenderedText("Uh oh. You have $0 left.", SDL_Color{static_cast<Uint8>(static_cast<int>(faded * 246.0)), static_cast<Uint8>(static_cast<int>(faded * 231.0)), static_cast<Uint8>(static_cast<int>(faded * 156.0)), 0xFF}, _TexMan.baseFontBold, _Renderer);
            fontTex.render(static_cast<int>((double)_Width * 3.0 / 2.0) - fontTex.getWidth() / 2, 50, _Renderer);

            fontTex.loadFromRenderedText("GAME OVER!", SDL_Color{static_cast<Uint8>(static_cast<int>(faded * 246.0)), static_cast<Uint8>(static_cast<int>(faded * 231.0)), static_cast<Uint8>(static_cast<int>(faded * 156.0)), 0xFF}, _TexMan.baseFontBold, _Renderer);
            fontTex.render(static_cast<int>((double)_Width * 3.0 / 2.0) - fontTex.getWidth() / 2, 80, _Renderer);

            fontTex.loadFromRenderedText("Play again?", SDL_Color{static_cast<Uint8>(static_cast<int>(faded * 246.0)), static_cast<Uint8>(static_cast<int>(faded * 231.0)), static_cast<Uint8>(static_cast<int>(faded * 156.0)), 0xFF}, _TexMan.baseFontBold, _Renderer);
            fontTex.render(static_cast<int>((double)_Width * 3.0 / 2.0) - fontTex.getWidth() / 2, 150, _Renderer);

            SDL_RenderPresent(_Renderer);
            handleVolume();
        } while (running);
        return playAgain;
    }

    bool win()
    {
        SDL_Event e;
        bool running {true};

        Timer timer;
        timer.start();
        double time_step {1.0};

        int mouseX, mouseY;
        int windowX, windowY;

        Timer animTimer{};
        animTimer.start();
        bool play_shown{false};

        Button playButton{{0, 0}, &(_TexMan.uiPlay)};

        bool playAgain{false};

        do {
            while (SDL_PollEvent(&e) != 0)
            {
                if (e.type == SDL_QUIT)
                {
                    return false;
                } else if (e.type == SDL_MOUSEMOTION)
                {
                    SDL_GetGlobalMouseState(&mouseX, &mouseY);
                    mouseX -= windowX;
                    mouseY -= windowY;
                } else if (e.type == SDL_KEYDOWN)
                {
                    switch (e.key.keysym.sym)
                    {
                        case (SDLK_RETURN):
                            running = false;
                            playAgain = true;
                        default:
                            break;
                    }
                } else if (e.type == SDL_KEYUP)
                {
                    switch (e.key.keysym.sym)
                    {
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
                } else if (e.type == SDL_MOUSEBUTTONDOWN)
                {
                    SDL_Rect mouseRect{mouseX / 3, mouseY / 3, 1, 1};
                    if (Util::checkCollision(&mouseRect, playButton.getRect()))
                    {
                        playAgain = true;
                        running = false;
                    }
                }
            }

            time_step = timer.getTicks() / 1000.0 * 60.0;
            time_step = std::min(time_step, 3.0);
            timer.start();

            SDL_GetWindowPosition(_Window, &windowX, &windowY);
            // set screen as render target
            _Screen.setAsRenderTarget(_Renderer);

            // clear screen (0x1f, 0x24, 0x4b)
            SDL_SetRenderDrawColor(_Renderer, 0x00, 0x00, 0x00, 0xFF);
            SDL_RenderClear(_Renderer);
            
            if (play_shown)
            {
                playButton.setPos(vec2<int>{_Width / 2 - 35, _Height / 2 - 5});
                playButton.update(time_step, mouseX / 3, mouseY / 3, _Renderer);
            } else {
                playButton.setExpand(-35.0);
                if (animTimer.getTicks() > 2000)
                {
                    play_shown = true;
                    _TexMan.SFX_button.play();
                }
            }

            SDL_SetRenderTarget(_Renderer, NULL);
            _Screen.renderClean(0, 0, _Renderer, 3);

            Texture fontTex{};
            double faded{static_cast<double>(std::min(static_cast<Uint32>(5000), animTimer.getTicks())) / 5000.0};
            fontTex.loadFromRenderedText("A game by @snej55", SDL_Color{static_cast<Uint8>(static_cast<int>(faded * 246.0)), static_cast<Uint8>(static_cast<int>(faded * 231.0)), static_cast<Uint8>(static_cast<int>(faded * 156.0)), 0xFF}, _TexMan.baseFontBold, _Renderer);
            fontTex.render(static_cast<int>((double)_Width * 3.0 / 2.0) - fontTex.getWidth() / 2, _Height * 3 - 50, _Renderer);

            fontTex.loadFromRenderedText("'Imagine dying.'.", SDL_Color{static_cast<Uint8>(static_cast<int>(faded * 246.0)), static_cast<Uint8>(static_cast<int>(faded * 231.0)), static_cast<Uint8>(static_cast<int>(faded * 156.0)), 0xFF}, _TexMan.baseFontBold, _Renderer);
            fontTex.render(static_cast<int>((double)_Width * 3.0 / 2.0) - fontTex.getWidth() / 2, 50, _Renderer);
            fontTex.loadFromRenderedText(" - @snej55 c. 2025", SDL_Color{static_cast<Uint8>(static_cast<int>(faded * 246.0)), static_cast<Uint8>(static_cast<int>(faded * 231.0)), static_cast<Uint8>(static_cast<int>(faded * 156.0)), 0xFF}, _TexMan.baseFontBold, _Renderer);
            fontTex.render(static_cast<int>((double)_Width * 3.0 / 2.0) - fontTex.getWidth() / 2 + 20, 70, _Renderer);

            fontTex.loadFromRenderedText("You Win!", SDL_Color{static_cast<Uint8>(static_cast<int>(faded * 246.0)), static_cast<Uint8>(static_cast<int>(faded * 231.0)), static_cast<Uint8>(static_cast<int>(faded * 156.0)), 0xFF}, _TexMan.baseFontBold, _Renderer);
            fontTex.render(static_cast<int>((double)_Width * 3.0 / 2.0) - fontTex.getWidth() / 2, 100, _Renderer);

            fontTex.loadFromRenderedText("Play again?", SDL_Color{static_cast<Uint8>(static_cast<int>(faded * 246.0)), static_cast<Uint8>(static_cast<int>(faded * 231.0)), static_cast<Uint8>(static_cast<int>(faded * 156.0)), 0xFF}, _TexMan.baseFontBold, _Renderer);
            fontTex.render(static_cast<int>((double)_Width * 3.0 / 2.0) - fontTex.getWidth() / 2, 150, _Renderer);

            SDL_RenderPresent(_Renderer);
            handleVolume();
        } while (running);
        return playAgain;
    }

    void handleVolume()
    {
        if (!_volume)
        {
            Mix_VolumeMusic(0);
        } else {
            Mix_VolumeMusic(MIX_MAX_VOLUME);
        }
    }
};

#endif
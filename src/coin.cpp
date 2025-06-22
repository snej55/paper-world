#include "./coin.hpp"
#include "./tiles.hpp"

CoinManager::CoinManager()
{
    timer.start();
}

CoinManager::CoinManager(Texture* coinTex, Texture* glowTex)
{
    timer.start();
    setTex(coinTex, glowTex);
}

CoinManager::~CoinManager()
{
    free();
}

void CoinManager::setTex(Texture* coinTex, Texture* glowTex)
{
    _coinTex = coinTex;
    _glowTex = glowTex;
}

void CoinManager::free()
{
    for (std::size_t i{0}; i < _Coins.size(); ++i)
    {
        Coin* coin{_Coins[i]};
        if (coin != nullptr)
        {
            delete coin->anim;
            delete coin;
            _Coins[i] = nullptr;
        }
    }
    _Coins.clear();
}

void CoinManager::addGlow(vec2<double> pos, vec2<double> vel)
{
    _Glow.push_back(new Glow{pos, vel, 10.0 - Util::random()});
}

void CoinManager::addCoin(vec2<double> pos, vec2<double> vel)
{
    Anim* anim{new Anim{3, 4, 2, 0.2, true, _coinTex}};
    anim->setFrame(static_cast<int>(Util::random() * 4.0));
    Coin* coin{new Coin{pos, vel, anim}};
    coin->timer.start();
    _Coins.push_back(coin);
}

void CoinManager::updateCoin(Coin* coin, const double& time_step, void* world)
{

    // ------------------------ Physics ------------------------ //

    coin->pos.x += coin->vel.x * time_step;
    SDL_Rect coinRect {static_cast<int>(coin->pos.x), static_cast<int>(coin->pos.y), 3, 4};
    std::array<SDL_Rect, 9> rects;
    static_cast<World*>(world)->getTilesAroundPos(coin->pos, rects);
    for (int i{0}; i < 9; ++i)
    {
        SDL_Rect* tileRect{&(rects[i])};
        if (Util::checkCollision(&coinRect, tileRect))
        {
            if (coin->vel.x > 0)
            {
                coinRect.x = tileRect->x - coinRect.w;
            } else {
                coinRect.x = tileRect->x + tileRect->w;
            }
            coin->pos.x = coinRect.x;
            coin->vel.x *= -0.5; // bounce
            coin->vel.y *= 0.9; // friction
        }
    }

    // repeat for vel-y
    coin->vel.y += 0.07 * time_step;
    coin->pos.y += coin->vel.y * time_step;
    coinRect = {static_cast<int>(coin->pos.x), static_cast<int>(coin->pos.y), 3, 4};
    static_cast<World*>(world)->getTilesAroundPos(coin->pos, rects);
    for (int i{0}; i < 9; ++i)
    {
        SDL_Rect* tileRect{&(rects[i])};
        if (Util::checkCollision(&coinRect, tileRect))
        {
            if (coin->vel.y > 0)
            {
                coinRect.y = tileRect->y - coinRect.h;
            } else {
                coinRect.y = tileRect->y + tileRect->h;
            }
            coin->pos.y = coinRect.y;
            coin->vel.y *= -0.5; // bounce
            coin->vel.x *= 0.9; // friction
        }
    }

    static_cast<World*>(world)->getDangerAroundPos(coin->pos, rects);
    for (int i{0}; i < 9; ++i)
    {
        SDL_Rect* tileRect{&(rects[i])};
        if (Util::checkCollision(&coinRect, tileRect))
        {
            coin->dead = true;
        }
    }
    // ------------------------ Other Stuff ------------------------ //

    coin->anim->tick(time_step);
}

void CoinManager::renderCoin(Coin* coin, const int scrollX, const int scrollY, SDL_Renderer* renderer)
{
    coin->anim->render(static_cast<int>(coin->pos.x), static_cast<int>(coin->pos.y), scrollX, scrollY, renderer);
}

void CoinManager::update(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, void* world, TexMan* texman, SDL_Rect* player_rect, double& last_coin)
{
    for (std::size_t i{0}; i < _Coins.size(); ++i)
    {
        Coin* coin{_Coins[i]};
        if (coin != nullptr)
        {
            updateCoin(coin, time_step, world);
            renderCoin(coin, scrollX, scrollY, renderer);
            SDL_Rect coinRect {static_cast<int>(coin->pos.x), static_cast<int>(coin->pos.y), 3, 4};
            if (Util::checkCollision(&coinRect, player_rect))
            {
                int num{(std::rand() % 5) + 10};
                for (int i{0}; i < num; ++i)
                {
                    _SparkManager.addSpark(new Spark{coin->pos, Util::random() * M_PI * 2.0, Util::random() * 2.0 + 0.5});
                }
                num = static_cast<int>(Util::random() * 5.0) + 10;
                for (int i{0}; i < num; ++i)
                {
                    double angle{Util::random() * M_PI * 2.0};
                    double speed(Util::random() * 2.0 + 1.0);
                    addGlow(coin->pos, {std::cos(angle) * speed, std::sin(angle) * speed});
                }
                last_coin = 1.0;
                delete coin->anim;
                delete coin;
                _Coins[i] = nullptr;
                texman->SFX_coin_collect.play();
            } else if (coin->dead)
            {
                int num{(std::rand() % 5) + 10};
                for (int i{0}; i < num; ++i)
                {
                    _SparkManager.addSpark(new Spark{coin->pos, Util::random() * M_PI * 2.0, Util::random() * 2.0 + 0.5});
                }
                delete coin->anim;
                delete coin;
                _Coins[i] = nullptr;
            } else if (coin->timer.getTicks() > 20000)
            {
                delete coin->anim;
                delete coin;
                _Coins[i] = nullptr;
            }
        }
    }
    _Coins.erase(std::remove_if(_Coins.begin(), _Coins.end(), [](Coin* coin){return (coin == nullptr);}), _Coins.end());

    for (std::size_t i{0}; i < _Glow.size(); ++i)
    {
        Glow* glow{_Glow[i]};
        glow->vel.x *= 0.9;
        glow->vel.y *= 0.9;
        glow->pos.x += glow->vel.x * time_step;
        glow->pos.y += glow->vel.y * time_step;
        glow->size -= 0.4 * time_step; // decay
        if (glow->size <= 0.0)
        {
            if (glow->size < -0.5 * Util::random())
            {
                ++_score;
                // flash effect
                _glowTex->setBlendMode(SDL_BLENDMODE_ADD);
                _glowTex->setAlpha(static_cast<Uint8>(static_cast<int>(255.0)));
                SDL_Rect renderQuad{static_cast<int>(glow->pos.x) - 1 - scrollX, static_cast<int>(glow->pos.y) - 1 - scrollY, 3, 3};
                SDL_RenderCopyEx(renderer, _glowTex->getTexture(), NULL, &renderQuad, 0, NULL, SDL_FLIP_NONE);
                delete glow;
                _Glow[i] = nullptr;
            }
        } else {
            _glowTex->setBlendMode(SDL_BLENDMODE_ADD);
            _glowTex->setAlpha(static_cast<Uint8>(static_cast<int>(glow->size / 10.0 * 255.0)));
            SDL_Rect renderQuad{static_cast<int>(glow->pos.x) - 2 - scrollX, static_cast<int>(glow->pos.y) - 2 - scrollY, 5, 5};
            SDL_RenderCopyEx(renderer, _glowTex->getTexture(), NULL, &renderQuad, 0, NULL, SDL_FLIP_NONE);
        }
    }
    _Glow.erase(std::remove_if(_Glow.begin(), _Glow.end(), [](Glow* glow){return (glow == nullptr);}), _Glow.end());
    _SparkManager.setTexture(&(texman->particle));
    _SparkManager.update(time_step, scrollX, scrollY, renderer);
}
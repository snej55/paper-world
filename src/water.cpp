#include "./water.hpp"

Water::Water(vec2<int> pos, vec2<int> dimensions, double spacing)
 : _pos{pos}, _dimensions{dimensions}, _spacing{spacing}
{
    _Rect = SDL_Rect{pos.x * TILE_SIZE, pos.y * TILE_SIZE, dimensions.x * TILE_SIZE, dimensions.y * TILE_SIZE};
    loadSprings();
    timer.start();
}

Water::~Water()
{
    free();
}

void Water::free()
{
    // dirty >:)
    for (WaterSpring*& spring : _Springs)
    {
        delete spring;
    }
    _Springs.clear();
    std::cout << "Water freed!\n";
}

void Water::loadSprings()
{
    free();
    int num{static_cast<int>(std::floor(static_cast<double>(_dimensions.x * TILE_SIZE) / _spacing)) + 1}; // remember relative tile dimensions
    for (int i{0}; i < num; ++i)
    {
        _Springs.push_back(new WaterSpring{{static_cast<double>(_pos.x * TILE_SIZE) + _spacing * (double)i, static_cast<double>(_pos.y * TILE_SIZE) + 4.0}, static_cast<double>(_pos.y * TILE_SIZE) + 4.0});
    }
    std::cout << "yo again\n";
}

SDL_Rect* Water::getRect()
{
    return &_Rect;
}

void Water::updateSpring(WaterSpring* spring, WaterSpring* left, WaterSpring* right, const double& time_step)
{
    double dh{spring->target_y - ((spring->pos.y - left->pos.y) + (spring->pos.y - right->pos.y)) - spring->pos.y};
    if (std::abs(dh) < 0.01)
    {
        spring->pos.y = spring->target_y;
    }
    spring->vel += (dh - spring->vel) * time_step * _tension * 1.1;
    spring->pos.y += spring->vel / 2.0 * time_step;
    spring->vel += (spring->vel * 0.95 - spring->vel) * time_step;
}

void Water::update(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, TexMan* texman, Player* player)
{
    std::vector<SDL_Vertex> points{};
    SDL_Color col{0x28, 0xca, 0xb1, 0xaa};
    for (int i{0}; i < static_cast<int>(std::size(_Springs)); ++i)
    {
        WaterSpring* spring{_Springs[i]};
        SDL_Rect spring_rect {static_cast<int>(spring->pos.x), static_cast<int>(spring->pos.y), 1, 1};
        double time{static_cast<double>(timer.getTicks())};
        if (Util::checkCollision(&spring_rect, player->getRect()))
        {
            if (std::abs(player->getVelY()) > 0.5 || std::abs(player->getVelX()) > 0.5)
            {
                if (std::abs(spring->target_y - spring->pos.y) < 3.0)
                    spring->vel += (std::max(-3.0, std::min(8.0, player->getVelY() * 3.0)) + -std::abs(std::max(-3.0, std::min(3.0, player->getVelX())))) * 0.5 * time_step;
            }
        }
        if (std::abs(spring->target_y - spring->pos.y) < 3.0)
        {
            spring->vel += std::sin(time * 0.002 + spring->pos.x * 0.6) * 0.05 * time_step;
            spring->vel += std::cos(time * 0.005 + spring->pos.x * 0.8) * 0.02 * time_step;
            spring->vel += std::cos(time * 0.006 + spring->pos.x * 0.7) * 0.03 * time_step;
            // spring->vel += std::sin(time * 0.004 + spring->pos.x * 0.6) * 0.08;
            // spring->vel += std::cos(time * 0.007 + spring->pos.x * 0.8) * 0.04;
            // spring->vel += std::cos(time * 0.008 + spring->pos.x * 0.7) * 0.06;
        }

        updateSpring(spring, _Springs[std::max(0, std::min(static_cast<int>(std::size(_Springs)) - 1, i - 1))], _Springs[std::max(0, std::min(static_cast<int>(std::size(_Springs)) - 1, i + 1))], time_step);
        points.push_back(SDL_Vertex{{static_cast<float>(spring->pos.x) - static_cast<float>(scrollX), static_cast<float>(spring->pos.y) - static_cast<float>(scrollY)}, col, {static_cast<float>(i) / static_cast<float>(static_cast<int>(std::size(_Springs)) - 1), 0.0f}});
    }

    for (int i{0}; i < static_cast<int>(std::size(_Springs)); ++i)
    {
        WaterSpring* spring{_Springs[i]};
        points.push_back(SDL_Vertex{{static_cast<float>(spring->pos.x) - static_cast<float>(scrollX), static_cast<float>((_pos.y + _dimensions.y) * TILE_SIZE) - static_cast<float>(scrollY)}, col, {static_cast<float>(i) / static_cast<float>(static_cast<int>(std::size(_Springs)) - 1), 0.0f}});
    }
    std::vector<int> indices{Util::get_water_indices<SDL_Vertex>(points)};
    // Util::printVector(indices);
    // for (int i{0}; i < std::size(points); ++i)
    // {
    //     SDL_SetRenderDrawColor(renderer, 0x28, 0xca, 0xb1, 0xaa);
    //     SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    //     SDL_RenderDrawPointF(renderer, points[i].position.x, points[i].position.y - 3.0f);
    // }
    SDL_SetRenderDrawColor(renderer, 0x28, 0xca, 0xb1, 0xaa);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    texman->particle.setBlendMode(SDL_BLENDMODE_BLEND);
    Polygons::renderPolygon(renderer, texman->particle.getTexture(), points, indices);
    SDL_Rect fillRect{_pos.x * TILE_SIZE - scrollX, _pos.y * TILE_SIZE - scrollY, _dimensions.x * TILE_SIZE, _dimensions.y * TILE_SIZE};
    SDL_SetRenderDrawColor(renderer, 0x28, 0xca, 0xb1, 0xaa);
    texman->particle.setBlendMode(SDL_BLENDMODE_NONE);
    
    std::vector<SDL_Point> line;
    line.resize(std::size(_Springs));
    for (int i{0}; i < static_cast<int>(std::size(_Springs)); ++i)
    {
        line[i] = SDL_Point{static_cast<int>(_Springs[i]->pos.x) - scrollX, static_cast<int>(_Springs[i]->pos.y) - scrollY};
    }
    SDL_SetRenderDrawColor(renderer, 0xb2, 0xde, 0xd8, 0x88);
    SDL_RenderDrawLines(renderer, line.data(), static_cast<int>(line.size()));
    //SDL_RenderFillRect(renderer, &fillRect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    const bool in_water{Util::checkCollision(getRect(), player->getRect())};
    if (in_water)
    {
        if (player->getInWater() > 120.0)
        {
            texman->SFX_water_in.play();
        }
        player->setInWater(0.0);
        return;
    } else {
        if (player->getInWater() > 6.0 && player->getInWater() < 120.0)
        {
            texman->SFX_water_out.play();
            player->setInWater(130.0);
        }
    }
}

WaterManager::WaterManager(const char* path)
{
    loadFromFile(path);
}

WaterManager::~WaterManager()
{
    for (std::size_t i{0}; i < _Water.size(); ++i)
    {
        _Water[i]->free();
        delete _Water[i];
    }
    _Water.clear();
    std::cout << "Freed water manager!\n";
}

void WaterManager::loadFromFile(const char* path)
{
    std::ifstream f{path};
    json data = json::parse(f);

    for (std::size_t i{0}; i < _Water.size(); ++i)
    {
        _Water[i]->free();
        delete _Water[i];
    }
    _Water.clear();

    for (const auto& rect : data["level"]["water"])
    {
        _Water.push_back(new Water{vec2<int>{rect[0], rect[1]}, vec2<int>{rect[2], rect[3]}, 1.0});
    }

    f.close();
}

void WaterManager::update(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, TexMan* texman, Player* player)
{
    for (std::size_t i{0}; i < _Water.size(); ++i)
    {
        _Water[i]->update(time_step, scrollX, scrollY, renderer, texman, player);
    }
}

Lava::Lava(vec2<int> pos, vec2<int> dimensions, double spacing)
 : _pos{pos}, _dimensions{dimensions}, _spacing{spacing}
{
    _Rect = SDL_Rect{pos.x * TILE_SIZE, pos.y * TILE_SIZE, dimensions.x * TILE_SIZE, dimensions.y * TILE_SIZE};
    loadSprings();
    timer.start();
}

Lava::~Lava()
{
    free();
}

void Lava::free()
{
    // dirty >:)
    for (WaterSpring*& spring : _Springs)
    {
        delete spring;
    }
    _Springs.clear();
    std::cout << "Lava freed!\n";
}

void Lava::addGlow(vec2<double> pos, vec2<double> vel)
{
    _Glow.push_back(new LavaGlow{pos, vel, 10.0 - Util::random()});
}

void Lava::loadSprings()
{
    free();
    int num{static_cast<int>(std::floor(static_cast<double>(_dimensions.x * TILE_SIZE) / _spacing)) + 1}; // remember relative tile dimensions
    for (int i{0}; i < num; ++i)
    {
        _Springs.push_back(new WaterSpring{{static_cast<double>(_pos.x * TILE_SIZE) + _spacing * (double)i, static_cast<double>(_pos.y * TILE_SIZE) + 4.0}, static_cast<double>(_pos.y * TILE_SIZE) + 4.0});
    }
    std::cout << "yo again\n";
}

SDL_Rect* Lava::getRect()
{
    return &_Rect;
}

void Lava::updateSpring(WaterSpring* spring, WaterSpring* left, WaterSpring* right, const double& time_step)
{
    double dh{spring->target_y - ((spring->pos.y - left->pos.y) + (spring->pos.y - right->pos.y)) * 1.5 - spring->pos.y};
    if (std::abs(dh) < 0.01)
    {
        spring->pos.y = spring->target_y;
    }
    spring->vel += (dh - spring->vel) * time_step * _tension * 1.1;
    spring->pos.y += spring->vel / 2.0 * time_step;
    spring->vel += (spring->vel * 0.95 - spring->vel) * time_step;
}

void Lava::update(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, TexMan* texman, Player* player)
{
    for (std::size_t i{0}; i < _Glow.size(); ++i)
    {
        LavaGlow* glow{_Glow[i]};
        glow->vel.x *= 0.9;
        glow->vel.y *= 0.9;
        glow->pos.x += glow->vel.x * time_step;
        glow->pos.y += glow->vel.y * time_step;
        glow->size -= 0.4 * time_step; // decay
        if (glow->size <= 0.0)
        {
            if (glow->size < -0.5 * Util::random())
            {
                // flash effect
                // texman->lightTex.setBlendMode(SDL_BLENDMODE_ADD);
                // texman->lightTex.setAlpha(static_cast<Uint8>(static_cast<int>(255.0)));
                // texman->lightTex.setColor(0xff, 0x53, 0x53);
                // SDL_Rect renderQuad{static_cast<int>(glow->pos.x) - 1 - scrollX, static_cast<int>(glow->pos.y) - 1 - scrollY, 3, 3};
                // SDL_RenderCopyEx(renderer, texman->lightTex.getTexture(), NULL, &renderQuad, 0, NULL, SDL_FLIP_NONE);
                delete glow;
                _Glow[i] = nullptr;
            }
        } else {
            texman->lightTex.setBlendMode(SDL_BLENDMODE_ADD);
            texman->lightTex.setAlpha(static_cast<Uint8>(static_cast<int>(glow->size / 10.0 * 255.0)));
            texman->lightTex.setColor(0xff, 0x53, 0x53); //0xd1, 0xa6, 0x7e
            SDL_Rect renderQuad{static_cast<int>(glow->pos.x) - 1 - scrollX, static_cast<int>(glow->pos.y) - 1 - scrollY, 3, 3};
            SDL_RenderCopyEx(renderer, texman->lightTex.getTexture(), NULL, &renderQuad, 0, NULL, SDL_FLIP_NONE);
        }
    }
    // reset color
    //texman->circle.setColor(246, 231, 156);
    texman->lightTex.setColor(246, 231, 156);
    _Glow.erase(std::remove_if(_Glow.begin(), _Glow.end(), [](LavaGlow* glow){return (glow == nullptr);}), _Glow.end());
    std::vector<SDL_Vertex> points{};
    SDL_Color col{192, 41, 49, 150};//{0xff, 0x53, 0x53, 0xbb};
    for (int i{0}; i < static_cast<int>(std::size(_Springs)); ++i)
    {
        WaterSpring* spring{_Springs[i]};
        SDL_Rect spring_rect {static_cast<int>(spring->pos.x), static_cast<int>(spring->pos.y), 1, 1};
        double time{static_cast<double>(timer.getTicks())};
        if (Util::checkCollision(&spring_rect, player->getRect()))
        {
            if (std::abs(player->getVelY()) > 0.5 || std::abs(player->getVelX()) > 0.5)
            {
                if (std::abs(spring->target_y - spring->pos.y) < 3.0)
                    spring->vel += (std::max(-3.0, std::min(8.0, player->getVelY() * 3.0)) + -std::abs(std::max(-3.0, std::min(3.0, player->getVelX())))) * 0.5 * time_step;
            }
        }
        if (Util::random() * 7000.0 / time_step < 128.0)
        {
            addGlow(spring->pos, {0.0, Util::random() * -1.0});
        }
        if (std::abs(spring->target_y - spring->pos.y) < 3.0)
        {
            spring->vel += std::sin(time * 0.002 + spring->pos.x * 0.6) * 0.05 * time_step;
            spring->vel += std::cos(time * 0.005 + spring->pos.x * 0.8) * 0.02 * time_step;
            spring->vel += std::cos(time * 0.006 + spring->pos.x * 0.7) * 0.03 * time_step;
            // spring->vel += std::sin(time * 0.004 + spring->pos.x * 0.6) * 0.08;
            // spring->vel += std::cos(time * 0.007 + spring->pos.x * 0.8) * 0.04;
            // spring->vel += std::cos(time * 0.008 + spring->pos.x * 0.7) * 0.06;
        }

        updateSpring(spring, _Springs[std::max(0, std::min(static_cast<int>(std::size(_Springs)) - 1, i - 1))], _Springs[std::max(0, std::min(static_cast<int>(std::size(_Springs)) - 1, i + 1))], time_step);
        points.push_back(SDL_Vertex{{static_cast<float>(spring->pos.x) - static_cast<float>(scrollX), static_cast<float>(spring->pos.y) - static_cast<float>(scrollY)}, col, {static_cast<float>(i) / static_cast<float>(static_cast<int>(std::size(_Springs)) - 1), 0.0f}});
    }

    for (int i{0}; i < static_cast<int>(std::size(_Springs)); ++i)
    {
        WaterSpring* spring{_Springs[i]};
        points.push_back(SDL_Vertex{{static_cast<float>(spring->pos.x) - static_cast<float>(scrollX), static_cast<float>((_pos.y + _dimensions.y) * TILE_SIZE) - static_cast<float>(scrollY)}, col, {static_cast<float>(i) / static_cast<float>(static_cast<int>(std::size(_Springs)) - 1), 0.0f}});
    }
    std::vector<int> indices{Util::get_water_indices<SDL_Vertex>(points)};
    // Util::printVector(indices);
    // for (int i{0}; i < std::size(points); ++i)
    // {
    //     SDL_SetRenderDrawColor(renderer, 0x28, 0xca, 0xb1, 0xaa);
    //     SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    //     SDL_RenderDrawPointF(renderer, points[i].position.x, points[i].position.y - 3.0f);
    // }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
    texman->particle.setBlendMode(SDL_BLENDMODE_ADD);
    Polygons::renderPolygon(renderer, texman->particle.getTexture(), points, indices);
    SDL_Rect fillRect{_pos.x * TILE_SIZE - scrollX, _pos.y * TILE_SIZE - scrollY, _dimensions.x * TILE_SIZE, _dimensions.y * TILE_SIZE};
    texman->particle.setBlendMode(SDL_BLENDMODE_NONE);
    
    std::vector<SDL_Point> line;
    line.resize(std::size(_Springs));
    for (int i{0}; i < static_cast<int>(std::size(_Springs)); ++i)
    {
        line[i] = SDL_Point{static_cast<int>(_Springs[i]->pos.x) - scrollX, static_cast<int>(_Springs[i]->pos.y) - scrollY};
    }
    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xaa);
    SDL_RenderDrawLines(renderer, line.data(), static_cast<int>(line.size()));
    for (int j{0}; j < _dimensions.y * TILE_SIZE - 8; ++j)
    {
        line.clear();
        line.resize(std::size(_Springs));
        for (int i{0}; i < static_cast<int>(std::size(_Springs)); ++i)
        {
            line[i] = SDL_Point{static_cast<int>(_Springs[i]->pos.x) - scrollX, static_cast<int>(_Springs[i]->pos.y) - scrollY + 1 + j};
        }
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_ADD);
        SDL_SetRenderDrawColor(renderer, 0xff, 0x76, 0x00, static_cast<Uint8>(static_cast<int>(static_cast<double>(_dimensions.y * TILE_SIZE - 8 - j) / static_cast<double>(_dimensions.y * TILE_SIZE - 8) * 200.0)));
        SDL_RenderDrawLines(renderer, line.data(), static_cast<int>(line.size()));
    }
    //SDL_RenderFillRect(renderer, &fillRect);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    if (Util::checkCollision(player->getRect(), getRect()))
    {
        double player_x{player->getCenter().x - static_cast<double>(_pos.x * TILE_SIZE)};
        player_x = player_x / _spacing;
        int spring_x {std::min(static_cast<int>(_Springs.size() - 1), std::max(0, static_cast<int>(player_x)))};
        _Springs[spring_x]->vel = -30;
        player->setLavaStruck(true);
        texman->SFX_water_out.play();
        texman->SFX_fire.play();
    }
}

LavaManager::LavaManager(const char* path)
{
    loadFromFile(path);
}

LavaManager::~LavaManager()
{
    for (std::size_t i{0}; i < _Lava.size(); ++i)
    {
        _Lava[i]->free();
        delete _Lava[i];
    }
    _Lava.clear();
    std::cout << "Freed Lava manager!\n";
}

void LavaManager::loadFromFile(const char* path)
{
    std::ifstream f{path};
    json data = json::parse(f);

    for (std::size_t i{0}; i < _Lava.size(); ++i)
    {
        _Lava[i]->free();
        delete _Lava[i];
    }
    _Lava.clear();

    for (const auto& rect : data["level"]["lava"])
    {
        _Lava.push_back(new Lava{vec2<int>{rect[0], rect[1]}, vec2<int>{rect[2], rect[3]}, 1.0});
    }

    f.close();
}

void LavaManager::update(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, TexMan* texman, Player* player)
{
    for (std::size_t i{0}; i < _Lava.size(); ++i)
    {
        _Lava[i]->update(time_step, scrollX, scrollY, renderer, texman, player);
    }
}
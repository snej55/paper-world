#ifndef TILES_H
#define TILES_H

#include "SDL2/SDL.h"
#include "JSON/json.hpp"
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <iostream>
#include <cmath> // for calculating tile/chunk coords (std::floor)

#include "./vec2.hpp"
#include "./util.hpp"
#include "./constants.hpp"

#include "./texman.hpp"
#include "./timer.hpp"

#include "./leaf.hpp"

using json = nlohmann::json;

constexpr int GRASS_VARIATIONS{18}; // number of different types of grass

struct Grass
{
    uint8_t variant;
    vec2<double> pos; // absolute position
    double angle{0};
    double target_angle{0};
    double turn_vel{0};
};

struct GrassTile
{
    Grass** grass;
    vec2<int> pos; // relative tile pos
    int total{0};
};

class GrassManager
{
private:
    const double _tension;
    GrassTile** _GrassTiles;
    int _total{0};

    // for wind
    Timer windTimer{};

public:
    // we don't load the grass immediately
    GrassManager(const double tension, const int total)
     : _tension{tension}
    {
        _GrassTiles = new GrassTile*[total];
        windTimer.start();
    }

    ~GrassManager()
    {
        free();
    }

    void free()
    {
        for (int i{0}; i < _total; ++i)
        {
            GrassTile* tile{_GrassTiles[i]};
            for (int j{0}; j < tile->total; ++j)
            {
                delete tile->grass[j];
            }
            delete tile;
        }
        delete _GrassTiles;
    }

    void addGrassTile(vec2<int> pos, const int density = 8) // relative tile pos
    {
        GrassTile* grassTile {new GrassTile};
        grassTile->pos = pos;
        // we need to allocate memory for the grass first, or we'll get a seg fault...
        grassTile->grass = new Grass*[density];
        // NOTE: double not std::size_t
        for (double i{0.0}; i < density; i += 1.0)
        {
            Grass* grass {new Grass};
            grass->variant = static_cast<uint8_t>(std::rand() % GRASS_VARIATIONS);
            grass->pos = vec2<double>{static_cast<double>(pos.x * TILE_SIZE) + (double)TILE_SIZE / (double)density * i, static_cast<double>(pos.y * TILE_SIZE)};
            grass->pos.x += Util::random() * M_PI;
            grass->pos.x = std::max(static_cast<double>(grassTile->pos.x * TILE_SIZE), std::min(static_cast<double>(grassTile->pos.x * TILE_SIZE + TILE_SIZE - 1), grass->pos.x));
            grassTile->grass[grassTile->total] = grass; // here
            grassTile->total += 1;
        }
        _GrassTiles[_total] = grassTile;
        ++_total;
    }

    void updateGrass(Grass* grass, const double& time_step, SDL_Rect* rect, bool check_collision = true)
    {
        double target_angle = 0.0;
        SDL_Rect grassRect{static_cast<int>(grass->pos.x), static_cast<int>(grass->pos.y) + 4, 4, 5};
        if (Util::checkCollision(&grassRect, rect))
        {
            double distance {std::pow(static_cast<double>(grassRect.x + grassRect.w / 2) - static_cast<double>(rect->x + rect->w / 2), 2) + std::pow(static_cast<double>(grassRect.y + grassRect.h / 2) - static_cast<double>(rect->y + rect->h / 2), 2)};
            double hd = static_cast<double>(grassRect.x + grassRect.w / 2) - static_cast<double>(rect->x + rect->w / 2);
            if (distance < 1600.0)
            {
                double temp_target{0};
                if (hd <= 0.0)
                {
                    temp_target = -70.0 - hd * 3.5;
                }
                if (hd > 0)
                {
                    temp_target = 70 - hd * 3.5;
                }
                target_angle = std::min(target_angle + temp_target, 90.0);
                target_angle = std::max(target_angle, -90.0);
            }
        }
        grass->target_angle += (target_angle - grass->target_angle) * 0.5 * time_step;
    }

    void renderGrass(const int scrollX, const int scrollY, SDL_Renderer* renderer, TexMan* texman, const int width, const int height, SDL_Rect* player_rect, const double& time_step)
    {
        double time{static_cast<double>(windTimer.getTicks())};
        for (std::size_t i{0}; i < _total; ++i)
        {
            GrassTile* grassTile {_GrassTiles[i]};
            // check if it is on the screen
            if (-TILE_SIZE * 2 < grassTile->pos.x * TILE_SIZE - scrollX && grassTile->pos.x * TILE_SIZE - scrollX < width + TILE_SIZE * 2 && -TILE_SIZE * 2 < grassTile->pos.y * TILE_SIZE - scrollY && grassTile->pos.y * TILE_SIZE - scrollY < height + TILE_SIZE * 2)
            {
                // iterate through grass in grassTile
                for (std::size_t g{0}; g < grassTile->total; ++g)
                {
                    Grass* grass {grassTile->grass[g]};
                    SDL_Rect tileRect {grassTile->pos.x * TILE_SIZE - 8, grassTile->pos.y * TILE_SIZE - 8, TILE_SIZE + 16, TILE_SIZE + 16};
                    updateGrass(grass, time_step, player_rect, Util::checkCollision(&tileRect, player_rect));
                    grass->target_angle += std::sin(time * 0.001 + (grass->pos.x + grass->pos.y) / 10.0) * (std::sin(time * 0.003 + (grass->pos.x + grass->pos.y) * 0.1) + 1.0) / 2 * time_step;
                    grass->target_angle += std::cos(time * (0.01 + 0.01 * (std::sin(grass->pos.x + grass->pos.y) + 1.0)) + (grass->pos.x + grass->pos.y) / 5.0) * 0.2 * (std::sin(time * 0.003 + (grass->pos.x + grass->pos.y) * 0.1) + 1.0) / 2 * time_step;
                    double force {grass->target_angle - grass->angle / _tension};
                    grass->turn_vel += force * time_step;
                    grass->angle += grass->turn_vel * time_step;
                    grass->turn_vel += (grass->turn_vel * 0.8 - grass->turn_vel) * time_step;
                    grass->angle = std::max(-90.0, std::min(90.0, grass->angle));
                    SDL_Rect clipRect{grass->variant * 9, 0, 9, 9};
                    SDL_Point center{5, 5};
                    texman->grass.render(static_cast<int>(grass->pos.x) - scrollX - 2.5, static_cast<int>(grass->pos.y) - scrollY + 3, renderer, grass->angle, &center, SDL_FLIP_NONE, &clipRect);
                }
            }
        }
    }
};

class Spring
{
private:
    vec2<double> _pos; // absolute tile pos
    double _vel{0.0};
    double _spring_factor{0.0};
    const double _dampening{0.9};
    SDL_Rect _rect{0, 0, 8, 5};

public:
    Spring(vec2<double> pos)
    {
        _pos = pos;
        _rect.x = pos.x;
        _rect.y = pos.y - 1;
    }

    void updateRect()
    {
        _rect.x = _pos.x;
        _rect.y = _pos.y - 1 + _spring_factor;
    }

    SDL_Rect* getRect()
    {
        updateRect();
        return &_rect;
    }

    void setVel(const double val)
    {
        _vel = val;
    }

    void update(const double& time_step)
    {
        _vel += (-_spring_factor) * 0.1 * time_step;
        _spring_factor += _vel * time_step;
        _vel += (_vel * _dampening - _vel) * time_step;
    }

    void render(const int scrollX, const int scrollY, SDL_Renderer* renderer, TexMan* texman)
    {
        texman->tileSpringTex.render(static_cast<int>(_pos.x) - scrollX, static_cast<int>(_pos.y + _spring_factor) - scrollY, renderer);
    }
};

enum class TileType
{
    GRASS,
    ROCK,
    SPIKE,
    NONE,
};

enum class DecorType
{
    TREE,
    LARGE_DECOR,
    NONE,
};

constexpr TileType SOLID_TILES[2] {TileType::GRASS, TileType::ROCK};
constexpr TileType DANGER_TILES[1] {TileType::SPIKE};

struct Tile
{
    vec2<int> pos; // relative position. real position = pos.x * TILE_SIZE, pos.y * TILE_SIZE
    TileType type; // tile type. grass, etc
    uint8_t variant; // variant in tileset 0-15
};

// the offgrid tiles
struct Decor
{
    vec2<int> pos;
    DecorType type;
    uint8_t variant;
};

struct Chunk
{
    vec2<int> pos; // relaative position. real position = pos.x * TILE_SIZE * CHUNK_SIZE, pos.y * TILE_SIZE * CHUNK_SIZE
    std::vector<Tile> tiles{}; // vec.pushback(tile) in World::load. tiles[CHUNK_sIZE * CHUNK_SIZE] gives a bunch of wasted space. most tiles are air
};

struct DecorChunk
{
    vec2<int> pos; // relative position. see above ^^^^^ for Chunk
    std::vector<Decor> decor{};
};

class World
{
private:
    Chunk _Chunks[LEVEL_WIDTH * LEVEL_HEIGHT];
    DecorChunk _DecorChunks[LEVEL_WIDTH * LEVEL_HEIGHT];
    
    GrassManager* _GrassManager {nullptr};
    LeafManager _LeafManager{};
    
    std::vector<Spring*> _Springs;


public:

    World()
    {
    }

    ~World()
    {
        for (std::size_t i{0}; i < _Springs.size(); ++i)
        {
            delete _Springs[i];
        }
        if (_GrassManager != nullptr)
        {
            delete _GrassManager;
        }
    }

    std::vector<Spring*>& getSprings()
    {
        return _Springs;
    }

    Chunk* getChunkAt(const double x, const double y)
    {
        vec2<int> chunk_loc {static_cast<int>(std::floor(x / (double)TILE_SIZE / (double)CHUNK_SIZE)), static_cast<int>(std::floor(y / (double)TILE_SIZE / (double)CHUNK_SIZE))};
        if (0 <= chunk_loc.x && chunk_loc.x < LEVEL_WIDTH && 0 <= chunk_loc.y && chunk_loc.y < LEVEL_HEIGHT)
        {
            int chunk_idx {chunk_loc.y * LEVEL_WIDTH + chunk_loc.x};
            Chunk* chunk {&(_Chunks[chunk_idx])};
            return chunk;
        }
        return nullptr;
    }

    Tile* getTileAt(const double x, const double y)
    {
        Chunk* chunk{getChunkAt(x, y)};
        if (chunk != nullptr)
        {
            for (std::size_t i{0}; i < chunk->tiles.size(); ++i)
            {
                Tile* tile = &(chunk->tiles[i]);
                int tileX {static_cast<int>(std::floor(x / (double)TILE_SIZE))};
                int tileY {static_cast<int>(std::floor(y / (double)TILE_SIZE))};
                //std::cout << tileX << '\t' << tileY << '\n';
                if (tileX == tile->pos.x && tileY == tile->pos.y)
                {
                    return tile;
                }
            }
        }
        return nullptr;
    }

    void getTilesAroundPos(vec2<double>& pos, std::array<SDL_Rect, 9>& rects)
    {
        for (auto& e : rects)
        {
            // some random thing player will never collide with - hopefully :)
            e = SDL_Rect{-99, -99, 1, 1};
        }
        for (int y{0}; y < 3; ++y)
        {
            for (int x{0}; x < 3; ++x)
            {
                Tile* tile {getTileAt(pos.x - TILE_SIZE + TILE_SIZE * x, pos.y - TILE_SIZE + TILE_SIZE * y)};
                if (tile != nullptr)
                {
                    if (Util::elementIn<TileType, std::size(SOLID_TILES)>(tile->type, SOLID_TILES))
                    {
                        rects[y * 3 + x] = SDL_Rect{tile->pos.x * TILE_SIZE, tile->pos.y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                    }
                }
                //rects[y * 3 + x] = 
            }
        }
    }

    void getDangerAroundPos(vec2<double>& pos, std::array<SDL_Rect, 9>& rects)
    {
        for (auto& e : rects)
        {
            // some random thing player will never collide with - hopefully :)
            e = SDL_Rect{-99, -99, 1, 1};
        }
        for (int y{0}; y < 3; ++y)
        {
            for (int x{0}; x < 3; ++x)
            {
                Tile* tile {getTileAt(pos.x - TILE_SIZE + TILE_SIZE * x, pos.y - TILE_SIZE + TILE_SIZE * y)};
                if (tile != nullptr)
                {
                    // Util::elementIn<T, N>(val, arr) -> bool
                    if (Util::elementIn<TileType, std::size(DANGER_TILES)>(tile->type, DANGER_TILES))
                    {
                        rects[y * 3 + x] = getDangerRect(tile);
                    }
                }
            }
        }
    }

    SDL_Rect getDangerRect(Tile* tile)
    {
        SDL_Rect rect;
        if (tile != nullptr)
        {
            switch (tile->type)
            {
                case (TileType::SPIKE):
                    switch (tile->variant) 
                    {
                        case (0):
                            rect.x = tile->pos.x * TILE_SIZE;
                            rect.y = tile->pos.y * TILE_SIZE + 5;
                            rect.w = 8;
                            rect.h = 3;
                            return rect;
                        case (1):
                            rect.x = tile->pos.x * TILE_SIZE + 5;
                            rect.y = tile->pos.y * TILE_SIZE;
                            rect.w = 3;
                            rect.h = 8;
                            return rect;
                        case (2):
                            rect.x = tile->pos.x * TILE_SIZE;
                            rect.y = tile->pos.y * TILE_SIZE;
                            rect.w = 8;
                            rect.h = 3;
                            return rect;
                        case (3):
                            rect.x = tile->pos.x * TILE_SIZE;
                            rect.y = tile->pos.y * TILE_SIZE;
                            rect.w = 3;
                            rect.h = 8;
                            return rect;
                        default:
                            return SDL_Rect{-110, -110, 1, 1};
                    }
                default:
                    return SDL_Rect{-110, -110, 1, 1};
            }
        }
        return SDL_Rect{-110, -110, 1, 1};
    }

    TileType getTileType(int type)
    {
        switch(type)
        {
            case 0:
                return TileType::GRASS;
            case 1:
                return TileType::ROCK;
            case 2:
                return TileType::SPIKE;
            default:
                return TileType::NONE;
        }
    }

    DecorType getDecorType(int type)
    {
        switch (type)
        {
            case 4:
                return DecorType::TREE;
            case 5:
                return DecorType::LARGE_DECOR;
            default:
                return DecorType::NONE;
        }
    }

    void loadFromFile(const char* path)
    {
        // load data
        std::ifstream f{path};
        json data = json::parse(f);

        // clear chunks
        for (std::size_t i{0}; i < LEVEL_WIDTH * LEVEL_HEIGHT; ++i)
        {
            _Chunks[i] = Chunk{{0, 0}};
        }
        for (std::size_t i{0}; i < LEVEL_WIDTH * LEVEL_HEIGHT; ++i)
        {
            _DecorChunks[i] = DecorChunk{{0, 0}};
        }

        _Springs.clear();

        // the leaf spawner rects
        std::vector<LeafSpawner> leaf_spawner_rects{};        
        
        int grass_total{0}; // keep track of how much grass there is

        // handle tiles that are on the grid
        for (const auto& tile : data["level"]["tiles"])
        {
            vec2<int> chunk_loc {static_cast<int>(std::floor((double)tile["pos"][0] / (double)CHUNK_SIZE)), static_cast<int>(std::floor((double)tile["pos"][1] / (double)CHUNK_SIZE))};
            if (0 <= chunk_loc.x && chunk_loc.x < LEVEL_WIDTH && 0 <= chunk_loc.y && chunk_loc.y < LEVEL_HEIGHT)
            {
                // calc chunk index
                int chunk_idx {chunk_loc.y * LEVEL_WIDTH + chunk_loc.x};
                Chunk* chunk {&(_Chunks[chunk_idx])};
                if (tile["type"] != 3) // grass key
                {
                    chunk->tiles.push_back(Tile{{tile["pos"][0], tile["pos"][1]}, getTileType(tile["type"]), tile["variant"]});
                    if (tile["type"] == 0 && (tile["variant"] == 1 || tile["variant"] == 13))
                    {
                        leaf_spawner_rects.push_back(LeafSpawner{{static_cast<int>(tile["pos"][0]) * TILE_SIZE, static_cast<int>(tile["pos"][1]) * TILE_SIZE, TILE_SIZE, TILE_SIZE}, false});
                    }
                } else {
                    ++grass_total;
                }
                chunk->pos = chunk_loc;
            }
        }

        // handle decor (offgrid tiles)
        for (const auto& tile : data["level"]["off_grid"])
        {
            vec2<int> chunk_loc {static_cast<int>(std::floor((double)tile["pos"][0] / (double)TILE_SIZE / (double)CHUNK_SIZE)), static_cast<int>(std::floor((double)tile["pos"][1] / (double)TILE_SIZE / (double)CHUNK_SIZE))};
            if (0 <= chunk_loc.x && chunk_loc.x < LEVEL_WIDTH && 0 <= chunk_loc.y && chunk_loc.y < LEVEL_HEIGHT)
            {
                int chunk_idx {chunk_loc.y * LEVEL_WIDTH + chunk_loc.x};
                DecorChunk* chunk {&(_DecorChunks[chunk_idx])};
                if (tile["type"] == 4)
                {
                    if (tile["variant"] == 0)
                    {
                        leaf_spawner_rects.push_back(LeafSpawner{{static_cast<int>(tile["pos"][0]) + 2, static_cast<int>(tile["pos"][1]) + TILE_SIZE, TILE_SIZE * 2, TILE_SIZE + 4}, false});
                    } else if (tile["variant"] == 1)
                    {
                        leaf_spawner_rects.push_back(LeafSpawner{{static_cast<int>(tile["pos"][0]) + TILE_SIZE, static_cast<int>(tile["pos"][1]) + TILE_SIZE * 3, TILE_SIZE * 2, TILE_SIZE}, false});
                    }
                } else if (tile["type"] == 5)
                {
                    if (tile["variant"] == 0)
                    {
                        leaf_spawner_rects.push_back(LeafSpawner{{static_cast<int>(tile["pos"][0]) + 4, static_cast<int>(tile["pos"][1]) + 4, TILE_SIZE * 5, TILE_SIZE + 3}, false});
                    } else if (tile["variant"] == 1)
                    {
                        leaf_spawner_rects.push_back(LeafSpawner{{static_cast<int>(tile["pos"][0]) + TILE_SIZE, static_cast<int>(tile["pos"][1]) + TILE_SIZE, TILE_SIZE * 2, TILE_SIZE}, false});
                    } else if (tile["variant"] == 2)
                    {
                        leaf_spawner_rects.push_back(LeafSpawner{{static_cast<int>(tile["pos"][0]) + TILE_SIZE, static_cast<int>(tile["pos"][1]) + TILE_SIZE * 2, TILE_SIZE * 2, TILE_SIZE}, false});
                    } else if (tile["variant"] == 3)
                    {
                        leaf_spawner_rects.push_back(LeafSpawner{{static_cast<int>(tile["pos"][0]) + TILE_SIZE, static_cast<int>(tile["pos"][1]) + TILE_SIZE, TILE_SIZE * 2, TILE_SIZE}, false});
                    } else if (tile["variant"] == 4)
                    {
                        leaf_spawner_rects.push_back(LeafSpawner{{static_cast<int>(tile["pos"][0]) + TILE_SIZE, static_cast<int>(tile["pos"][1]) + TILE_SIZE * 3, TILE_SIZE * 2, TILE_SIZE}, false});
                    }
                }
                chunk->decor.push_back(Decor{{tile["pos"][0], tile["pos"][1]}, getDecorType(tile["type"]), tile["variant"]});
                chunk->pos = chunk_loc;
            }
        }

        // handle grass
        if (_GrassManager != nullptr)
        {
            delete _GrassManager;
        }
        _GrassManager = new GrassManager{8.0, grass_total};
        for (const auto& tile : data["level"]["tiles"])
        {
            if (tile["type"] == 3) // grass key
            {
               _GrassManager->addGrassTile({tile["pos"][0], tile["pos"][1]}, 4);
            }
        }

        // handle springs
        for (const auto& spring : data["level"]["springs"])
        {
            _Springs.push_back(new Spring{vec2<double>{(double)spring["pos"][0], (double)spring["pos"][1]}});
        }

        _LeafManager.free();
        _LeafManager.loadRects(leaf_spawner_rects);

        f.close();
    }

    void render(const int scrollX, const int scrollY, SDL_Window* window, SDL_Renderer* renderer, TexMan* texman, const int width, const int height)
    {
        // location in relative chunk coords
        int chunkX {static_cast<int>(std::floor((double)scrollX / (double)TILE_SIZE / (double)CHUNK_SIZE))};
        int chunkY {static_cast<int>(std::floor((double)scrollY / (double)TILE_SIZE / (double)CHUNK_SIZE))};
        for (int y{0}; y < std::floor(height / TILE_SIZE) + 1; ++y)
        {
            for (int x{0}; x < std::floor(width / TILE_SIZE) + 1; ++x)
            {
                int targetX = chunkX - 1 + x;
                int targetY = chunkY - 1 + y;
                if (0 <= targetX && targetX < LEVEL_WIDTH && 0 <= targetY && targetY < LEVEL_HEIGHT)
                {
                    int chunk_idx{targetY * LEVEL_WIDTH + targetX};
                    DecorChunk* decor{&(_DecorChunks[chunk_idx])};
                    renderDecorChunk(decor, scrollX, scrollY, window, renderer, texman);
                    Chunk* chunk{&(_Chunks[chunk_idx])};
                    renderChunk(chunk, scrollX, scrollY, window, renderer, texman);
                }
            }
        }

        for (Spring* spring : _Springs)
        {
            spring->render(scrollX, scrollY, renderer, texman);
        }
    }

    void updateLeaves(const double& time_step, const int scrollX, const int scrollY, const int width, const int height, TexMan* texman, SDL_Renderer* renderer)
    {
        _LeafManager.update(time_step, scrollX, scrollY, width, height, texman, renderer);
    }

    void handleSprings(const double& time_step)
    {
        for (Spring* spring : _Springs)
        {
            spring->update(time_step);
        }
    }

    Texture* getTileTex(const Tile& tile, TexMan* texman)
    {
        switch (tile.type)
        {
            case (TileType::GRASS):
                return &(texman->tileGrassTex);
            case (TileType::ROCK):
                return &(texman->tileRockTex);
            case (TileType::SPIKE):
                return &(texman->tileSpikeTex);
            default:
                return nullptr;
        }
    }

    Texture* getTileTex(const Decor& tile, TexMan* texman)
    {
        switch (tile.type)
        {
            case (DecorType::TREE):
                return &(texman->tileTrees);
            case (DecorType::LARGE_DECOR):
                return &(texman->largeDecor);
            default:
                // std::cout << static_cast<int>(tile.type) << '\n';
                return nullptr;
        }
    }

    SDL_Rect getClipRect(const Tile& tile)
    {
        if (tile.type == TileType::SPIKE)
        {
            SDL_Rect clip{tile.variant * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE};
            return clip;
        }
        SDL_Rect clip{(tile.variant % 4) * TILE_SIZE, static_cast<int>((tile.variant - (tile.variant % 4)) / 4) * TILE_SIZE, TILE_SIZE, TILE_SIZE};
        return clip;
    }

    SDL_Rect getDecorClipRect(const Decor& tile)
    {
        SDL_Rect clip{};
        if (tile.type == DecorType::TREE)
        {
            clip = {tile.variant * 32, 0, 32, 32}; // these small trees are 32 * 32
        } else if (tile.type == DecorType::LARGE_DECOR)
        {
            clip = {tile.variant * 50, 0, 50, 50}; // big boys
        }
        return clip;
    }

    void renderChunk(Chunk* chunk, const int scrollX, const int scrollY, SDL_Window* window, SDL_Renderer* renderer, TexMan* texman)
    {
        for (const auto& tile : chunk->tiles)
        {
            /*
            variant = w * y + x
            x = variant % w
            y = (variant - x) / w
            */
            SDL_Rect clip{getClipRect(tile)};
            getTileTex(tile, texman)->render(tile.pos.x * TILE_SIZE - scrollX, tile.pos.y * TILE_SIZE - scrollY, renderer, &clip);
        }
    }

    void renderDecorChunk(DecorChunk* chunk, const int scrollX, const int scrollY, SDL_Window* window, SDL_Renderer* renderer, TexMan* texman)
    {
        for (const Decor& tile : chunk->decor)
        {
            SDL_Rect clip{getDecorClipRect(tile)};
            getTileTex(tile, texman)->render(tile.pos.x - scrollX, tile.pos.y - scrollY, renderer, &clip);
        }
    }

    void handleGrass(const int scrollX, const int scrollY, SDL_Renderer* renderer, TexMan* texman, const int width, const int height, SDL_Rect* entity_rect, const double& time_step)
    {
        _GrassManager->renderGrass(scrollX, scrollY, renderer, texman, width, height, entity_rect, time_step);
    }
};

#endif
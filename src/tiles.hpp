#ifndef TILES_H
#define TILES_H

#include <SDL2/SDL.h>
#include <JSON/json.hpp>
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

using json = nlohmann::json;

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
        _rect.y = _pos.y - 1;
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
        _vel *= _dampening;
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

constexpr TileType SOLID_TILES[2] {TileType::GRASS, TileType::ROCK};
constexpr TileType DANGER_TILES[1] {TileType::SPIKE};

struct Tile
{
    vec2<int> pos; // relative position. real position = pos.x * TILE_SIZE, pos.y * TILE_SIZE
    TileType type; // tile type. grass, etc
    uint8_t variant; // variant in tileset 0-15
};

struct Chunk
{
    vec2<int> pos; // relaative position. real position = pos.x * TILE_SIZE * CHUNK_SIZE, pos.y * TILE_SIZE * CHUNK_SIZE
    std::vector<Tile> tiles{}; // vec.pushback(tile) in World::load. tiles[CHUNK_sIZE * CHUNK_SIZE] gives a bunch of wasted space. most tiles are air
};

class World
{
private:
    Chunk _Chunks[LEVEL_WIDTH * LEVEL_HEIGHT];
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

    void loadFromFile(const char* path)
    {
        std::ifstream f{path};
        json data = json::parse(f);
        // clear chunks
        for (std::size_t i{0}; i < LEVEL_WIDTH * LEVEL_HEIGHT; ++i)
        {
            _Chunks[i] = Chunk{{0, 0}};
        }
        _Springs.clear();
        for (const auto& tile : data["level"]["tiles"])
        {
            vec2<int> chunk_loc {static_cast<int>(std::floor((double)tile["pos"][0] / (double)CHUNK_SIZE)), static_cast<int>(std::floor((double)tile["pos"][1] / (double)CHUNK_SIZE))};
            if (0 <= chunk_loc.x && chunk_loc.x < LEVEL_WIDTH && 0 <= chunk_loc.y && chunk_loc.y < LEVEL_HEIGHT)
            {
                // calc chunk index
                int chunk_idx {chunk_loc.y * LEVEL_WIDTH + chunk_loc.x};
                Chunk* chunk {&(_Chunks[chunk_idx])};
                chunk->tiles.push_back(Tile{{tile["pos"][0], tile["pos"][1]}, getTileType(tile["type"]), tile["variant"]});
                chunk->pos = chunk_loc;
            }
        }
        for (const auto& spring : data["level"]["springs"])
        {
            _Springs.push_back(new Spring{vec2<double>{(double)spring["pos"][0], (double)spring["pos"][1]}});
        }
        f.close();
    }

    void render(const int scrollX, const int scrollY, SDL_Window* window, SDL_Renderer* renderer, TexMan* texman, const int width, const int height)
    {
        // location in relative chunk coords
        int chunkX {std::floor((double)scrollX / (double)TILE_SIZE / (double)CHUNK_SIZE)};
        int chunkY {std::floor((double)scrollY / (double)TILE_SIZE / (double)CHUNK_SIZE)};
        for (int y{0}; y < std::floor(height / TILE_SIZE) + 1; ++y)
        {
            for (int x{0}; x < std::floor(width / TILE_SIZE) + 1; ++x)
            {
                int targetX = chunkX - 1 + x;
                int targetY = chunkY - 1 + y;
                if (0 <= targetX && targetX < LEVEL_WIDTH && 0 <= targetY && targetY < LEVEL_HEIGHT)
                {
                    int chunk_idx{targetY * LEVEL_WIDTH + targetX};
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

    SDL_Rect getClipRect(const Tile& tile)
    {
        if (tile.type == TileType::SPIKE)
        {
            SDL_Rect clip{tile.variant * TILE_SIZE, 0, TILE_SIZE, TILE_SIZE};
        }
        SDL_Rect clip{(tile.variant % 4) * TILE_SIZE, static_cast<int>((tile.variant - (tile.variant % 4)) / 4) * TILE_SIZE, TILE_SIZE, TILE_SIZE};
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
            getTileTex(tile, texman)->render(tile.pos.x * TILE_SIZE * SCALE_FACTOR - scrollX, tile.pos.y * TILE_SIZE * SCALE_FACTOR  - scrollY, renderer, &clip);
        }
    }
};

#endif
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
#include "./constants.hpp"

#include "./texman.hpp"

using json = nlohmann::json;

enum class TileType
{
    GRASS,
    ROCK,
    NONE,
};

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

public:

    World()
    {
    }

    ~World()
    {
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
                    rects[y * 3 + x] = SDL_Rect{tile->pos.x * TILE_SIZE, tile->pos.y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                }
                //rects[y * 3 + x] = 
            }
        }
    }

    TileType getTileType(int type)
    {
        switch(type)
        {
            case 0:
                return TileType::GRASS;
            case 1:
                return TileType::ROCK;
            default:
                return TileType::NONE;
        }
    }

    void loadFromFile(const char* path, bool clear = false)
    {
        std::ifstream f{path};
        json data = json::parse(f);
        // clear chunks
        for (std::size_t i{0}; i < LEVEL_WIDTH * LEVEL_HEIGHT; ++i)
        {
            _Chunks[i] = Chunk{{0, 0}};
        }
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
        f.close();
    }

    void render(const int scrollX, const int scrollY, SDL_Window* window, SDL_Renderer* renderer, TexMan* texman)
    {
        // location in relative chunk coords
        int chunkX {std::floor((double)scrollX / (double)TILE_SIZE / (double)CHUNK_SIZE)};
        int chunkY {std::floor((double)scrollY / (double)TILE_SIZE / (double)CHUNK_SIZE)};
        for (int y{0}; y < std::floor(SCR_HEIGHT / TILE_SIZE) + 1; ++y)
        {
            for (int x{0}; x < std::floor(SCR_WIDTH / TILE_SIZE) + 1; ++x)
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
    }

    Texture* getTileTex(const Tile& tile, TexMan* texman)
    {
        switch (tile.type)
        {
            case (TileType::GRASS):
                return &(texman->tileGrassTex);
            case (TileType::ROCK):
                return &(texman->tileRockTex);
            default:
                return nullptr;
        }
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
            SDL_Rect clip{(tile.variant % 4) * TILE_SIZE, static_cast<int>((tile.variant - (tile.variant % 4)) / 4) * TILE_SIZE, TILE_SIZE, TILE_SIZE};
            getTileTex(tile, texman)->render(tile.pos.x * TILE_SIZE * SCALE_FACTOR - scrollX, tile.pos.y * TILE_SIZE * SCALE_FACTOR  - scrollY, renderer, &clip);
        }
    }
};

#endif
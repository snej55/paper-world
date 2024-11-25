#ifndef TILES_H
#define TILES_H

#include <SDL2/SDL.h>

#include "./vec2.hpp"
#include "./constants.hpp"

struct Tile
{
    vec2<int> pos; // relative position. real position = pos.x * TILE_SIZE, pos.y * TILE_SIZE
    uint8_t type; // tile type. grass, etc
    uint8_t variant; // variant in tileset 0-15
};

struct Chunk
{
    vec2<int> pos; // relaative position. real position = pos.x * TILE_SIZE * CHUNK_SIZE, pos.y * TILE_SIZE * CHUNK_SIZE
    Tile* tiles[CHUNK_SIZE * CHUNK_SIZE];
};

class World
{
    
};

#endif
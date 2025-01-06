//
// Created by jenskromdijk3 on 1/2/25.
//

#ifndef POLYGONS_H
#define POLYGONS_H

#include "SDL2/SDL.h"
#include <vector>
#include <array>

namespace Polygons {
      template<int NUM_INDICES>
      inline void renderPolygon(SDL_Renderer* renderer, SDL_Texture* texture, std::vector<SDL_Vertex>& vertices, std::array<int, NUM_INDICES>& indices)
      {
            SDL_RenderGeometry(renderer, texture, vertices.data(), static_cast<int>(vertices.size()), indices.data(), NUM_INDICES);
      }

      inline void renderPolygon(SDL_Renderer* renderer, SDL_Texture* texture, const std::vector<SDL_Vertex>& vertices, const std::vector<int>& indices) {
            SDL_RenderGeometry(renderer, texture, vertices.data(), static_cast<int>(vertices.size()), indices.data(), static_cast<int>(indices.size()));
      }
}

#endif //POLYGONS_H

#include <iostream>
#include <fstream>

#define SDL_MAIN_HANDLED

#include "SDL2/SDL.h"
#include "src/game.hpp"

int main(int argc, char* argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "Failed to initialize SDL! SDL_Error: " << SDL_GetError() << '\n';
        return 0;
    }
    Game game{};
    game.start();
    game.Close();
    std::cout << "Finished!\n";

    // Quit SDL subsystems
    TTF_Quit();
    IMG_Quit();
    Mix_Quit();
    SDL_Quit();
    std::cout << "Quit SDL!\n";
    return 0;
}
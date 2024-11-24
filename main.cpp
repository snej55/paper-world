#include <iostream>
#include <SDL2/SDL.h>

#include "src/game.hpp"

int main(int argc, char* argv[])
{
    Game game{};
    game.start();
    return 0;
}
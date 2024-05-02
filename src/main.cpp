#include "Game.h"

#ifndef NDEBUG
#include <iostream>
#endif //NDEBUG

int main()
{
#ifndef NDEBUG
    std::cerr << "WARNING: Running debug build, expect reduced performance." << std::endl;
#endif //NDEBUG

    Game game;
    game.run();
}

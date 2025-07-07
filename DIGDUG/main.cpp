#include <iostream>
#include "Game.h"

int main()
{
    Game game;

    if (!game.initialise()) {
        std::cerr << "Failed to initialise game!" << std::endl;
        return -1;
    }

    game.run();
    game.cleanup();

    return 0;
}
#ifndef ROBOTSGAME_FIELD_H
#define ROBOTSGAME_FIELD_H

#include <SFML/Graphics.hpp>

void createField(sf::RenderWindow &window);
void startGame();

typedef struct gameField{
    char field[36][64];
    int borderX, borderY;
}GAMEFIELD;

#endif //ROBOTSGAME_FIELD_H

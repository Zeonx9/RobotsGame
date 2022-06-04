#ifndef ROBOTSGAME_GAME_H
#define ROBOTSGAME_GAME_H

typedef struct gameField{
    char gameBoard[18][32]; // игровое поле
    int positionFirst; // место расположения игрока 1
    int positionSecond; // место расположения игрока 2
    int leftCorX;
    int leftCorY;
}GameField;

#endif //ROBOTSGAME_GAME_H

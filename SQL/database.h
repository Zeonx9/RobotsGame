#ifndef ROBOTSGAME_DATABASE_H
#define ROBOTSGAME_DATABASE_H

#include "data_scheme.h"

typedef enum {
    games,
    highScore,
    wins
}Categories;

typedef struct pair{
    int *number;
    PlayerData **pd;
}Pair;

PlayerData *findPlayer(char* login);
int registerUser(char * login, char * password);
PlayerData **findBestPlayers(int count);
int updateData(int ID, Categories category, int value);
int deletePlayer(int ID);


#endif //ROBOTSGAME_DATABASE_H

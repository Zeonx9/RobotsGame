#ifndef ROBOTSGAME_DATABASE_H
#define ROBOTSGAME_DATABASE_H

typedef enum {
    games,
    highScore,
    wins
}Categories;

typedef struct player_data{
    int ID;
    char login[21], password[21];
    int highScore, gamesPlayed, wins;
} PlayerData;

typedef struct pair{
    int *number;
    PlayerData **pd;
}Pair;

PlayerData *findPlayer(char* login);
int registerUser(char * login, char * password);
PlayerData **findBestPlayers(int count);
int updateData(int ID, Categories category, int value);

#endif //ROBOTSGAME_DATABASE_H
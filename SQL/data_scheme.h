//
// Created by musht on 2022-06-03.
//

#ifndef ROBOTSGAME_DATA_SCHEME_H
#define ROBOTSGAME_DATA_SCHEME_H

typedef struct player_data{
    int ID;
    char login[21], password[21];
    int highScore, gamesPlayed, wins;
} PlayerData;

void playerToString(PlayerData *pd, char *dest);
void playerFromStr(PlayerData *pd, char *src);

#endif //ROBOTSGAME_DATA_SCHEME_H

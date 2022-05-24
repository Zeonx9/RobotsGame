//
// Created by Шадрин Антон Альберт on 24.05.2022.
//

#ifndef ROBOTSGAME_DATABASE_H
#define ROBOTSGAME_DATABASE_H


typedef struct player_data{
    int ID;
    char login[21], password[21];
    int highScore, gamesPlayed, wins;
} PlayerData;
PlayerData *findPlayer(char* login);
void registerUser(char * login, char * password);

#endif //ROBOTSGAME_DATABASE_H

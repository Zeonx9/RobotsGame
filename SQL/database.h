//
// Created by Шадрин Антон Альберт on 24.05.2022.
//

#ifndef ROBOTSGAME_DATABASE_H
#define ROBOTSGAME_DATABASE_H

void registerUser(char * login, char * password);
void findPlayer(char* login);


typedef struct player_data{
    int ID;
    char login[21], password[21];
} PlayerData;

#endif //ROBOTSGAME_DATABASE_H

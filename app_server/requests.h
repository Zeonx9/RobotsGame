#ifndef ROBOTSGAME_REQUESTS_H
#define ROBOTSGAME_REQUESTS_H

#include "../SQL/database.h"

#define JOIN_TO_GAME 555 // возвращается, если поступил запрос на создание/подключение к игре

// переводит объект игрока в строку
void StringToPlayer(PlayerData *pd, char *src);

// заполняет переданную по указатель структуру игрока по заданной строке
void PlayerToString(PlayerData *pd, char *dest);

// метод вызывает соответствующий обработчик запроса
int handleRequest(char *in, char *out);

#endif //ROBOTSGAME_REQUESTS_H

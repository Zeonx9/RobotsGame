#ifndef ROBOTSGAME_REQUESTS_H
#define ROBOTSGAME_REQUESTS_H

#include "../SQL/database.h"
#include "server.h"

#define JOIN_TO_GAME 555 // возвращается, если поступил запрос на создание/подключение к игре

// метод вызывает соответствующий обработчик запроса
int handleRequest(char *in, char *out, SharedData *shd);

#endif //ROBOTSGAME_REQUESTS_H

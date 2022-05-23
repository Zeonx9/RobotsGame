/// необходимые элементы интерфейса
/// активности и связь интерфейса с частью работающей над связью с сервером

#ifndef ROBOTSGAME_INTERFACE_H
#define ROBOTSGAME_INTERFACE_H

#include "client.h"
#include <SFML/Graphics.hpp>
#include <pthread.h>

typedef enum activities {
    mainMenu, logIn, gameLobby, play, closeApp, closeApproved
} Activities;

// структура для связи потока связи с сервером с интерфейсом
typedef struct shared_state {
    pthread_mutex_t mutex;
    int connected; // подключен ли клиент к серверу?
    SOCKET sock;   // сокет подключения
    int logged;    // выполнен ли вход?
    Activities currentActivity; // текущая активность
} SharedState;

void * requestsRoutine(void * dta);

// здесь начинаются активности
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void createMenuApp(sf::RenderWindow &window, SharedState * shs);

#endif //ROBOTSGAME_INTERFACE_H

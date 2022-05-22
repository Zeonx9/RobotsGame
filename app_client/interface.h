#ifndef ROBOTSGAME_INTERFACE_H
#define ROBOTSGAME_INTERFACE_H

#include "client.h"
#include <SFML/Graphics.hpp>

typedef enum activities {
    mainMenu, logIn, registration, gameLobby, play, closeApp, closeApproved
} Activities;

// структура для связи потока связи с сервером с интерфейсом
typedef struct shared_state {
    pthread_mutex_t mutex;
    int connected; // подключен ли клиент к серверу?
    SOCKET sock;   // сокет подключения
    int logged;    // выполнен ли вход?
    Activities currentActivity; // текущая активность
} SharedState;

void createMenuApp(sf::RenderWindow &window, SharedState * shs);

#endif //ROBOTSGAME_INTERFACE_H

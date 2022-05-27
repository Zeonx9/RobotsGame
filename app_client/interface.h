/// необходимые элементы интерфейса
/// активности и связь интерфейса с частью работающей над связью с сервером

#ifndef ROBOTSGAME_INTERFACE_H
#define ROBOTSGAME_INTERFACE_H

#include "client.h"
#include <SFML/Graphics.hpp>
#include <pthread.h>

typedef enum activities {
    mainMenu, LogHub, logIn, registering, gameLobby, play, closeApp, closeApproved
} Activities;

typedef enum login_states {
    noSuchUser = -11, wrongPassword = -12, alreadyExists = -2, notLogged = 0, success = 1
} LoginStates;

// структура для связи потока связи с сервером с интерфейсом
typedef struct shared_state {
    pthread_mutex_t mutex;
    int connected; // подключен ли клиент к серверу?
    SOCKET sock;   // сокет подключения
    LoginStates logged;    // выполнен ли вход?
    Activities currentActivity; // текущая активность
} SharedState;

// для связи с сервером, запускается в отдельном потоке
void * requestsRoutine(void * dta);

// здесь начинаются активности
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// создает экран для главного меню
void createMenuApp(sf::RenderWindow &window, SharedState * shs);

// создает экран для меню входа и регистрации
void createRegWindow(sf::RenderWindow &window, SharedState * shs);

#endif //ROBOTSGAME_INTERFACE_H

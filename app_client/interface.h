/// необходимые элементы интерфейса
/// активности и связь интерфейса с частью работающей над связью с сервером

#ifndef ROBOTSGAME_INTERFACE_H
#define ROBOTSGAME_INTERFACE_H

#include "client.h"
#include <SFML/Graphics.hpp>
#include <pthread.h>

// константы показывающие, какое на данный момент действие выполняет приложение
typedef enum activities {
    // константы с явно указанным значениям обозначают экраны, эти значения - индексы в массиве функций отрисовки
    mainMenu = 0, logHub = 1, gameLobby = 2, play = 3, logIn, registering, closeApp = -1, closeApproved = -2
} Activities;

// состояния входа в аккаунт, успешно ли ? или ошибки
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

void windowDispatcher(SharedState * shs);

#endif //ROBOTSGAME_INTERFACE_H

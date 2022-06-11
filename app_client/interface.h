/// необходимые элементы интерфейса
/// активности и связь интерфейса с частью работающей над связью с сервером

#ifndef ROBOTSGAME_INTERFACE_H
#define ROBOTSGAME_INTERFACE_H

extern "C" {
    #include "../SQL/data_scheme.h"
    #include "client.h"
    #include "game.h"
};
#include <SFML/Graphics.hpp>
#include <pthread.h>

// константы показывающие, какое на данный момент действие выполняет приложение
typedef enum activities {
    // константы с явно указанным значениям обозначают экраны, эти значения - индексы в массиве функций отрисовки
    mainMenu = 0, logHub = 1, gameLobby = 2, play = 3, gameOver = 4, // экраны приложения
    logIn, registering, // действия в окне регистрации (logHub)
    getRating, joinGame, cancelGame, // действие в лобби (gameLobby
    updateData, // )
    closeApp = -1, closeApproved = -2 // закрытие окна
} Activities;

// состояния входа в аккаунт, успешно ли ? или ошибки
typedef enum login_states {
    noSuchUser = -11, wrongPassword = -12, alreadyExists = -2, notLogged = 0, success = 1
} LoginStates;

typedef struct game_result {
    char opponentLogin[21];
    int time, winner, hits;
} GameResult;

// структура для связи потока связи с сервером с интерфейсом
typedef struct shared_state {
    char * logInfo, * rating;
    int connected, gameStarted; // подключен ли клиент к серверу?
    pthread_mutex_t mutex;
    SOCKET sock;   // сокет подключения
    LoginStates logged;    // выполнен ли вход?
    Activities act; // текущая активность
    PlayerData * player; // информация об игроке
    GameResult * gameResult;
} SharedState;

// для связи с сервером, запускается в отдельном потоке
void * requestsRoutine(void * dta);

void windowDispatcher(SharedState * shs);

#endif //ROBOTSGAME_INTERFACE_H

#include "interface.h"

// точка входа в клиентское приложение
int main() {
    // подключение библиотеки ws2_32.lib
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(2, 2), &wsd)) {
        printf("!! CANNOT CONNECT TO THE LIB");
        return 1;
    }

    // выделить память для объекта связи с потоком работающим с сетью
    SharedState * shs = (SharedState *) malloc(sizeof(SharedState));
    shs->connected = shs->gameStarted = 0, shs->act = mainMenu, shs->logged = notLogged;
    shs->player = NULL;
    shs->gameResult.opponentLogin = (char *) malloc(21 * sizeof(char));
    pthread_mutex_init(&(shs->mutex), NULL);

    // создать поток для работы с сетью
    pthread_t tid;
    pthread_create(&tid, NULL, requestsRoutine, (void *) shs);
    pthread_detach(tid);

    // вызов диспетчера окон, он управляет переходами
    windowDispatcher(shs);

    // подождать пока поток связи закончит работу и закроется
    while(shs->act != closeApproved);

    // очистить память при выходе из приложения
    pthread_mutex_destroy(&(shs->mutex));
    free(shs->gameResult.opponentLogin);
    if (shs->player)
        free(shs->player);
    free(shs);
    return 0;
}
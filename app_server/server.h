#ifndef ROBOTSGAME_SERVER_H
#define ROBOTSGAME_SERVER_H

#include <stdio.h>
#include <winsock2.h>
#include <pthread.h>

// = структуры =
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// узел связанного списка подключенных клиентов сервера
typedef struct client_node_ {
    struct client_node_ * next;
    struct {
        SOCKET sock;
        SOCKADDR_IN addr;
        pthread_t tid;
    } data;
} ClientNode;

// список клиентов сервера
typedef struct list_clients_ {
    int count;
    ClientNode * self;
} ClientsList;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
typedef enum joinStates {
    justCreated, justJoined, waiting, completed
} JoinStates;

typedef struct game {
    int id1, id2;
    SOCKET client1, client2;
} Game;

typedef struct games_manager {
    int hasActiveGame;
    Game *game;
    int notifyFirst;
} GamesManager;

// содержит указатели на объекты связи между потоками клиентов и сервером
// упаковка нужных аргументов для потока создания клиентов
typedef struct shared_data {
    ClientsList *list;
    SOCKET sock;
    pthread_mutex_t mutex;
    int shutdown;
    GamesManager gManager;
} SharedData;

// = функции для списка =
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// функция добавления
void addClient(ClientsList * list, SOCKET s, SOCKADDR_IN a, pthread_t t);

// функция удаления + возврат
ClientNode * popClient(ClientsList * list, SOCKET s);

// функция удаления всего списка
void deleteList(ClientsList * list);

// = функции для сервера =
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// функция запуска сервера (create -> bind -> listen)
SOCKET createServer(SharedData *dta);

//  точка входа в поток обработки присоединяющихся клиентов
void * clientAcceptorRoutine(void * servSock);

// точка входа в поток создающийся для каждого нового клиента
void * clientRoutine(void * dta);

#endif //ROBOTSGAME_SERVER_H

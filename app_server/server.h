#ifndef ROBOTSGAME_SERVER_H
#define ROBOTSGAME_SERVER_H

#include <stdio.h>
#include <winsock2.h>
#include <pthread.h>

// = структуры =

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

// упаковка нужных аргументов для потока создания клиентов
typedef struct client_handler_data {
    ClientsList *list;
    SOCKET server;
} CHandlerDta;

// = функции =
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// функция добавления
void addClient(ClientsList * list, SOCKET s, SOCKADDR_IN a, pthread_t t);

// функция удаления + возврат
ClientNode popClient(ClientsList * list, pthread_t id);

// функция запуска сервера (create -> bind -> listen)
SOCKET createServer(CHandlerDta *dta);

//  точка входа в поток обработки присоединяющихся клиентов
void * handleNewClients(void * servSock);

// точка входа в поток создающийся для каждого нового клиента
void * clientRoutine(void * clientSock);

// обработка сообщений; только для тестов, потом будет заменена или удалена
void processData(char *data);

#endif //ROBOTSGAME_SERVER_H

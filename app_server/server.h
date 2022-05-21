#ifndef ROBOTSGAME_SERVER_H
#define ROBOTSGAME_SERVER_H

#include <stdio.h>
#include <winsock2.h>
#include <pthread.h>

SOCKET createServer();
void * handleNewClients(void * servSock);
void * clientRoutine(void * clientSock);
void processData(char *data); // только для тестов, потом будет заменена или удалена

#endif //ROBOTSGAME_SERVER_H

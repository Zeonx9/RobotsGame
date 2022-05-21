#ifndef ROBOTSGAME_CLIENT_H
#define ROBOTSGAME_CLIENT_H

#include <stdio.h>
#include <winsock2.h>

typedef struct {
    SOCKET sock;
    int connected;
    char *buf;
} ClientState;

// функция создает сокет и соединяет его с сервером
SOCKET connectToServer();

// функция организует отправку сообщений на сервер и прием ответов от него
int serverSession(SOCKET client, char * buffer);

#endif //ROBOTSGAME_CLIENT_H

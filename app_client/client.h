/// файл с функциями необходимыми клиентскому приложения для связи с сервером

#ifndef ROBOTSGAME_CLIENT_H
#define ROBOTSGAME_CLIENT_H

#include <stdio.h>
#include <winsock2.h>

#define IP "192.168.0.105"

// функция создает сокет и соединяет его с сервером
SOCKET connectToServer();

// отправить что-то на сервер, получить ответ
int serverSession(SOCKET client, char * bufferIn, char * bufferOut);

#endif //ROBOTSGAME_CLIENT_H

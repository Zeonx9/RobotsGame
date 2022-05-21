#include "client.h"

SOCKET connectToServer() {
    SOCKET client;

    // создать сокет для клиента и проверить на удачное создание
    client = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (client == INVALID_SOCKET){
        printf("!! ERROR CANNOT CREATE SOCKET\n");
        return INVALID_SOCKET;
    }

    // описание сервера для подключения
    SOCKADDR_IN server;
    server.sin_family = AF_INET;
    server.sin_port = htons(2205); // такой же порт как на сервере
    server.sin_addr.S_un.S_addr = inet_addr("192.168.244.105"); // Zeon's IP адрес

    // инициализация соединения с сервером
    if (connect(client, (SOCKADDR *) &server, sizeof(server)) == SOCKET_ERROR) {
        printf("!! CANNOT CONNECT TO SERVER: %d\n", WSAGetLastError());
        closesocket(client);
        return INVALID_SOCKET;
    }
    printf(">> Connected to server\n");

    return client;
}

int serverSession(SOCKET client, char *buffer) {
    // отправить сообщение на север
    if (send(client, buffer, (int) strlen(buffer) + 1, 0) == SOCKET_ERROR) {
        printf("!! CANNOT SEND MASSAGE\n");
        closesocket(client);
        return 4;
    }

    if (strcmp(buffer, "EXIT\n") == 0)
        return 1;

    // получить ответ от сервера
    int rc = SOCKET_ERROR;
    while (rc == SOCKET_ERROR) {
        rc = recv(client, buffer, 1025, 0);
        if (!rc || rc == WSAECONNRESET) {
            printf("!! CONNECTION CLOSED\n");
            closesocket(client);
            return 5;
        }
    }

    return 0;
}


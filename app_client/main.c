#include <stdio.h>
#include <winsock2.h>

SOCKET connectToServer() {
    SOCKET client;

    // создать сокет для клиента и проверить на удачное создание
    client = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (client == INVALID_SOCKET){
        printf("ERROR CANNOT CREATE SOCKET");
        exit(1);
    } printf("Client created\n");

    // описание сервера для подключения
    SOCKADDR_IN server;
    server.sin_family = AF_INET;
    server.sin_port = htons(2205); // такой же порт как на сервере
    server.sin_addr.S_un.S_addr = inet_addr("127.0.0.1"); // специальный look-up адрес

    // инициализация соединения с сервером
    if (connect(client, (SOCKADDR *) &server, sizeof(server)) == SOCKET_ERROR) {
        printf("CANNOT CONNECT TO SERVER");
        closesocket(client);
        exit(2);
    } printf("Connected to server\n");

    return client;
}

int main() {
    WSADATA wsd;

    // подключение библиотеки ws2_32.lib
    if (WSAStartup(MAKEWORD(2, 2), &wsd)) {
        printf("CANNOT CONNECT TO THE LIB");
        return 1;
    } printf("Connected to the lib\n");

    // запуск сервера
    SOCKET client = connectToServer();

    closesocket(client);
    printf("Client stopped\n");
    getchar();
    return 0;
}

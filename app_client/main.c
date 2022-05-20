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
        WSACleanup();
        exit(2);
    } printf("Connected to server\n");

    return client;
}

int main() {
    WSADATA wsd;

    // подключение библиотеки ws2_32.lib
    if (WSAStartup(MAKEWORD(2, 2), &wsd)) {
        printf("CANNOT CONNECT TO THE LIB");
        return 3;
    } printf("Connected to the lib\n");

    // запуск сервера
    SOCKET client = connectToServer();

    char msg[1025] = "";
    printf(" >> Enter message to be processed on server: ");
    fgets(msg, 1024, stdin); // получить сообщение от пользователя

    // отправить сообщение на север
    if (send(client, msg, (int) strlen(msg) + 1, 0) == SOCKET_ERROR) {
        printf("CANNOT SEND MASSAGE");
        closesocket(client);
        return 4;
    } printf("message sent\n");

    // получить ответ от сервера
    int rc = SOCKET_ERROR;
    while (rc == SOCKET_ERROR) {
        rc = recv(client, msg, 1025, 0);
        if (!rc || rc == WSAECONNRESET) {
            printf("CONNECTION CLOSED\n");
            closesocket(client);
            return 5;
        }
    } printf("processed message from server:\n\t%s\n", msg);

    // закрыть сокет
    closesocket(client);
    WSACleanup();
    printf("Client stopped\n");
    getchar();
    return 0;
}

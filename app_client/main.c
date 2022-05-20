#include <stdio.h>
#include <winsock2.h>

// функция создает сокет и соединяет его с сервером
SOCKET connectToServer() {
    SOCKET client;

    // создать сокет для клиента и проверить на удачное создание
    client = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (client == INVALID_SOCKET){
        printf("ERROR CANNOT CREATE SOCKET");
        exit(2);
    } printf("Client created\n");

    // описание сервера для подключения
    SOCKADDR_IN server;
    server.sin_family = AF_INET;
    server.sin_port = htons(2205); // такой же порт как на сервере
    server.sin_addr.S_un.S_addr = inet_addr("192.168.244.105"); // Zeon's IP адрес

    // инициализация соединения с сервером
    if (connect(client, (SOCKADDR *) &server, sizeof(server)) == SOCKET_ERROR) {
        printf("CANNOT CONNECT TO SERVER: %d", WSAGetLastError());
        closesocket(client);
        WSACleanup();
        getchar();
        exit(3);
    } printf("Connected to server\n");

    return client;
}

// функция организует отправку сообщений на сервер и прием ответов от него
int runClient(SOCKET client) {
    char msg[1025] = "";

    // цикл запросов от пользователя, пока не введена команда выхода
    int extFlag = 0;
    while (!extFlag) {
        printf(" >> Enter message to be processed on server: ");
        fgets(msg, 1024, stdin); // получить сообщение от пользователя

        // условие закрытия клиента
        extFlag = strcmp(msg, "EXIT\n") == 0;

        // отправить сообщение на север
        if (send(client, msg, (int) strlen(msg) + 1, 0) == SOCKET_ERROR) {
            printf("CANNOT SEND MASSAGE");
            closesocket(client);
            return 4;
        }
        printf("message sent\n");

        // получить ответ от сервера
        int rc = SOCKET_ERROR;
        while (rc == SOCKET_ERROR) {
            rc = recv(client, msg, 1025, 0);
            if (!rc || rc == WSAECONNRESET) {
                printf("CONNECTION CLOSED\n");
                closesocket(client);
                return 5;
            }
        }
        printf("processed message from server:\n\t%s\n", msg);
    }
    // закрыть сокет
    closesocket(client);
    WSACleanup();
    printf("Client stopped\n");
    return 0;
}

int main() {
    // подключение библиотеки ws2_32.lib
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(2, 2), &wsd)) {
        printf("CANNOT CONNECT TO THE LIB");
        return 1;
    }
    printf("Connected to the lib\n");

    // получать запросы от пользователя и отправлять на сервер
    runClient(connectToServer());

    getchar();
    return 0;
}

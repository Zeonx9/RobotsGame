#include "client.h"

int main() {
    // подключение библиотеки ws2_32.lib
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(2, 2), &wsd)) {
        printf("!! CANNOT CONNECT TO THE LIB");
        return 1;
    }

    // создать и подключить к серверу сокет
    SOCKET client = connectToServer();
    if (client == INVALID_SOCKET)
        return -1;

    // начать цикл для обработки запросов
    char buffer[1025] = "";
    for(int res = 0; !res; ) {
        printf("@ Enter message to be processed on server : ");
        fgets(buffer, 1024, stdin); // получить сообщение от пользователя

        // обработать запрос
        res = serverSession(client, buffer);
        if (res == 1)
            break;

        printf("@ processed message from server:\n\t%s\n", buffer);
    }

    // закрыть сокет
    closesocket(client);
    printf(">> Client stopped\n");

    getchar();
    return 0;
}

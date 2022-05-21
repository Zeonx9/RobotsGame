#include "server.h"

int main() {
    // подключение библиотеки ws2_32.lib
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(2, 2), &wsd)) {
        printf("!! CANNOT CONNECT TO THE LIB\n");
        return 1;
    }

    // запуск сервера
    SOCKET server = createServer();
    if (server == INVALID_SOCKET)
        return -1;

    // цикл обработки команд
    for  (char cmd[20] = ""; strcmp(cmd, "halt") != 0; scanf("#%s", cmd));

    // закрытие сервера
    closesocket(server);
    printf(">> Server stopped\n");

    getchar();
    return 0;
}

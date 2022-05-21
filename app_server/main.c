#include "server.h"

int main() {
    // подключение библиотеки ws2_32.lib
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(2, 2), &wsd)) {
        printf("!! CANNOT CONNECT TO THE LIB\n");
        return 1;
    }

    // создать список клиентов;
    ClientsList list = {};
    struct client_handler_data * chd = malloc(sizeof(struct client_handler_data));
    chd->list = &list;

    // запуск сервера
    SOCKET server = createServer(chd);
    if (server == INVALID_SOCKET)
        return -1;

    // цикл обработки команд
    char cmd[20] = "";
    while(strcmp(cmd, "stop") != 0) {
        // выводит количество подключенных клиентов
        if (strcmp(cmd, "count") == 0){
            printf("now %d clients connected\n", list.count);
        }
        // выводит весь список клиентов
        if (strcmp(cmd, "show") == 0) {
            for (ClientNode * this = list.self; this; this = this->next) {
                printf("-> Client <%s> with socket %llu at thread %llu",
                       inet_ntoa(this->data.addr.sin_addr), this->data.sock, this->data.tid);
            }
        }
        scanf("%s", cmd);
    }

    // закрытие сервера
    closesocket(server);
    printf(">> Server stopped\n");

    getchar();
    return 0;
}

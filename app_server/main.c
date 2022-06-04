#include "server.h"

int main() {
    // подключение библиотеки ws2_32.lib
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(2, 2), &wsd)) {
        printf("!! CANNOT CONNECT TO THE LIB\n");
        return 1;
    }

    // создать список клиентов;
    ClientsList list = {}; // список создан статически !
    struct shared_data * shd = malloc(sizeof(struct shared_data));
    shd->list = &list;
    shd->shutdown = shd->gManager.hasActiveGame = shd->gManager.notifyFirst = 0;
    pthread_mutex_init(&(shd->mutex), NULL); // создать мьютекс для контроля общих данных

    // запуск сервера
    SOCKET server = createServer(shd);
    if (server == INVALID_SOCKET)
        return -1;

    // цикл обработки команд
    char cmd[20] = "";
    while(strcmp(cmd, "stop") != 0) {
        pthread_mutex_lock(&(shd->mutex)); // поставить блокировку на время использования общих данных
        // выводит количество подключенных клиентов
        if (strcmp(cmd, "count") == 0){
            printf("now %d clients connected\n", list.count);
        }
        // выводит весь список клиентов
        if (strcmp(cmd, "show") == 0) {
            printf("list of clients (%d):\n", list.count);
            for (ClientNode * this = list.self; this; this = this->next) {
                printf("-> Client <%s> with socket %llu at thread %llu\n",
                       inet_ntoa(this->data.addr.sin_addr), this->data.sock, this->data.tid);
            }
            printf("\n");
        }
        pthread_mutex_unlock(&(shd->mutex)); // снять блокировку
        scanf("%s", cmd);
    }


    // закрытие сервера
    printf(">> Server stopped\n");
    shd->shutdown = 1;
    closesocket(server);
    deleteList(shd->list);
    pthread_mutex_destroy(&(shd->mutex));
    return 0;
}

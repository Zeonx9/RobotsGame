#include "server.h"

SOCKET createServer() {
    SOCKET server;
    SOCKADDR_IN serverAddr;

    // создать сокет для сервера (tcp/ip)
    server = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (server == INVALID_SOCKET) {
        printf("!! СANNOT CREATE SERVER SOCKET\n");
        return INVALID_SOCKET;
    }

    // настройка сокета сервера: указание домена, адреса и порта
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(2205); // номер порта (22 год май месяц)
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // любой адрес

    // привязывание сокету сервера сформированного адреса и проверка на удачную привязку
    if (bind(server, (SOCKADDR *) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("!! CANNOT BIND SERVER ADDRESS\n");
        return INVALID_SOCKET;
    }

    // начинаем прослушивание, 5 - максимальное число клиентов в ожидании на подключение
    if (listen(server, 5) == SOCKET_ERROR) {
        printf("!! CANNOT START TO LISTEN\n");
        return INVALID_SOCKET;
    }

    // создать поток обрабатывающий новых клиентов
    pthread_t comingClientsTid;
    pthread_create(&comingClientsTid, NULL, handleNewClients, (void *) server);
    pthread_detach(comingClientsTid);

    printf(">> Server started\n");
    return server;
}

void * handleNewClients(void * servSock) {
    SOCKET client, server = (SOCKET) servSock;
    SOCKADDR_IN clientAddr;

    while(1) {
        int size = sizeof(clientAddr);

        // захват клиента и установление соединения
        client = accept(server, (SOCKADDR *) &clientAddr, &size);
        if (client == INVALID_SOCKET) {
            printf("!! ERROR ACCEPT CLIENT\n");
            continue; // попробуем снова установить соединение
        }

        // создать отдельный поток для обработки соединения с новым клиентом
        printf(">> Client accepted\n");
        pthread_t newTreadId;
        pthread_create(&newTreadId, NULL, clientRoutine, (void *) client);
        pthread_detach(newTreadId);
    }
}

void * clientRoutine(void * param) {
    SOCKET client = (SOCKET) param;
    char msg[1025];

    // цикл обработки диалога с клиентом
    while (1) {
        //  принять сообщение от клиента
        int res = recv(client, msg, 1025, 0);
        if (!res || res == SOCKET_ERROR) {
            printf("!! ERROR RECEIVE MESSAGE, CLIENT DISCONNECTED\n");
            return (void *) 4;
        }

        // выход из цикла по команде
        if (strcmp(msg, "EXIT\n") == 0) {
            printf(">> client disconnecting");
            break;
        }

        // распечатать и обработать сообщение
        printf("@ massage received form %llu:\n\t%s\n", client, msg);
        processData(msg);

        // послать обратно
        if (send(client, msg, (int) strlen(msg) + 1, 0) == SOCKET_ERROR) {
            printf("!! CANNOT SEND MESSAGE BACK");
            return (void *) 5;
        }
    }
    return (void *) 0;
}

void processData(char *data) {
    for (int i = 0, len = (int) strlen(data); i < len; ++i) {
        if ('A' <= data[i] && data[i] <= 'Z')
            data[i] += 'a' - 'A';
        else if ('a' <= data[i] && data[i] <= 'z')
            data[i] -= 'a' - 'A';
    }
}

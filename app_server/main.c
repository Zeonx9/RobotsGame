#include <stdio.h>
#include <winsock2.h>
#include <pthread.h>

// глобальная переменная для остановки сервера
int closeFlag = 0;

// функция обработки данный от клиента
void processData(char *data) {
    for (int i = 0, len = (int) strlen(data); i < len; ++i) {
        if ('A' <= data[i] && data[i] <= 'Z')
            data[i] += 'a' - 'A';
        else if ('a' <= data[i] && data[i] <= 'z')
            data[i] -= 'a' - 'A';
    }
}

// точка входа в поток обработки соединения с отдельным клиентом
void * clientRoutine(void * param) {
    SOCKET client = (SOCKET) param;
    char msg[1025];

    // цикл обработки диалога с клиентом
    while (1) {
        //  принять сообщение от клиента
        int res = recv(client, msg, 1025, 0);
        if (!res || res == SOCKET_ERROR) {
            printf("ERROR RECEIVE MESSAGE, CLIENT DISCONNECTED\n");
            pthread_exit((void *) 4);
        }
        printf("massage received form %llu:\n\t%s\n", client, msg);

        // поднять флаг остановки сервера если пришла такая команда
        closeFlag += strcmp(msg, "CLOSE_SERVER\n") == 0;

        // обработать сообщение
        processData(msg);

        // послать обратно
        if (send(client, msg, (int) strlen(msg) + 1, 0) == SOCKET_ERROR) {
            printf("CANNOT SEND MESSAGE BACK");
            return (void *) 5;
        }
        printf("message sent back\n");
    }
}

int CreateServer(){
    SOCKET server, client;
    SOCKADDR_IN serverAddr, clientAddr;

    // создать сокет для сервера (tcp/ip)
    server = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (server == INVALID_SOCKET) {
        printf("СANNOT CREATE SERVER SOCKET");
        return 1;
    }
    printf("Server created\n");

    // настройка сокета сервера: указание домена, адреса и порта
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(2205); // номер порта (22 год май месяц)
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); // любой адрес

    // привязывание сокету сервера сформированного адреса и проверка на удачную привязку
    if (bind(server, (SOCKADDR *) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("CANNOT BIND SERVER ADDRESS");
        return 2;
    }
    printf("Server bound\n");

    // начинаем прослушивание, 5 - максимальное число клиентов в ожидании на подключение
    if (listen(server, 5) == SOCKET_ERROR) {
        printf("CANNOT START TO LISTEN");
        return 3;
    }
    printf("Server started to listen\n");

    // бесконечный цикл - сервер работает и готов устанавливать соединения с клиентами
    while (1) {
        int size = sizeof(clientAddr);

        // захват клиента и установление соединения
        client = accept(server, (SOCKADDR *) &clientAddr, &size);
        if (client == INVALID_SOCKET) {
            printf("ERROR ACCEPT CLIENT");
            continue; // попробуем снова установить соединение
        }
        printf("Client accepted\n");

        // не создавать новый поток, если была команда закрытия
        if (closeFlag) break;

        // создать отдельный поток для обработки соединения с новым клиентом
        pthread_t newTreadId;
        pthread_create(&newTreadId, NULL, clientRoutine, (void *) client);
        pthread_detach(newTreadId);
    }

    printf("Server stopped\n");
    closesocket(server);
    WSACleanup();
    getchar();
    return 0;
}

int main() {
    // подключение библиотеки ws2_32.lib
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(2, 2), &wsd)) {
        printf("CANNOT CONNECT TO THE LIB");
        return 1;
    } printf("Connected to the lib\n");

    // запуск сервера
    return CreateServer();
}

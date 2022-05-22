#include "server.h"

void addClient(ClientsList * list, SOCKET s, SOCKADDR_IN a, pthread_t t) {
    ++list->count;
    // создать новый узел для клиента
    ClientNode * new = malloc(sizeof(ClientNode));
    new->data.sock = s, new->data.addr = a, new->data.tid = t;
    new->next = NULL;

    // если список пуст, положить туда новый узел
    if (!list->self) {
        list->self = new;
        return;
    }

    // положить узел в начало
    new->next = list->self;
    list->self = new;
}

ClientNode * popClient(ClientsList * list, SOCKET s) {
    --list->count;

    if (list->self->data.sock == s) {
        ClientNode *node = list->self;
        list->self = list->self->next;
        node->next = NULL;
        return node;
    }

    for (ClientNode * prev = list->self, *this = prev->next; this; prev = this, this = prev->next) {
        if (this->data.sock == s) {
            prev->next = this->next;
            this->next = NULL;
            return this;
        }
    }
    printf("!! NO CLIENT WITH THIS SOCKET\n");
    return NULL;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SOCKET createServer(CHandlerDta *dta) {
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
    dta->sock = server; // записать сокет созданного сервера в объект передающихся данных
    pthread_t clientAcceptor;
    pthread_create(&clientAcceptor, NULL, clientAcceptorRoutine, (void *) dta);
    pthread_detach(clientAcceptor);

    printf(">> Server started\n");
    return server;
}

void * clientAcceptorRoutine(void * dta) {
    // распаковать данные, объявить переменные
    CHandlerDta * chd = (CHandlerDta *) dta;
    SOCKET client, server = chd->sock;
    SOCKADDR_IN clientAddr;

    // бесконечный цикл приема новых клиентов, будет прерван при завершении главного потока сервера
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
        chd->sock = client;
        pthread_t newTreadId;
        pthread_create(&newTreadId, NULL, clientRoutine, (void *) chd);
        pthread_detach(newTreadId);

        // добавить нового клиента в список сервера
        addClient(chd->list, client, clientAddr, newTreadId);
    }
}

void * clientRoutine(void * dta) {
    CHandlerDta * chd = (CHandlerDta *) dta;
    SOCKET client = chd->sock;
    char msg[1025];

    // цикл обработки диалога с клиентом
    while (1) {
        //  принять сообщение от клиента
        int res = recv(client, msg, 1025, 0);
        if (!res || res == SOCKET_ERROR) {
            printf("!! ERROR RECEIVE MESSAGE, CLIENT DISCONNECTED\n");
            break;
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
            break;
        }
    }
    // клиент отключился, удаляем из списка
    ClientNode *node = popClient(chd->list, client);
    free(node);
}

void processData(char *data) {
    for (int i = 0, len = (int) strlen(data); i < len; ++i) {
        if ('A' <= data[i] && data[i] <= 'Z')
            data[i] += 'a' - 'A';
        else if ('a' <= data[i] && data[i] <= 'z')
            data[i] -= 'a' - 'A';
    }
}

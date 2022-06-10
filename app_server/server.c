#include "server.h"
#include "requests.h"
#include "../app_client/game.h"


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

void deleteList(ClientsList * list) {
    while (list->self) {
        ClientNode * this = list->self;
        list->self = list->self->next;
        free(this);
    }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SOCKET createServer(SharedData *dta) {
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
    SharedData * shd = (SharedData *) dta;
    SOCKET client, server = shd->sock;
    SOCKADDR_IN clientAddr;

    // бесконечный цикл приема новых клиентов, будет прерван при завершении главного потока сервера
    while(!shd->shutdown) {
        int size = sizeof(clientAddr);

        // захват клиента и установление соединения
        client = accept(server, (SOCKADDR *) &clientAddr, &size);
        if (client == INVALID_SOCKET) {
            printf("!! ERROR ACCEPT CLIENT\n");
            continue; // попробуем снова установить соединение
        }

        // создать отдельный поток для обработки соединения с новым клиентом
        printf(">> Client accepted\n");

        pthread_mutex_lock(&(shd->mutex));
        shd->sock = client;
        pthread_mutex_unlock(&(shd->mutex));

        pthread_t newTreadId;
        pthread_create(&newTreadId, NULL, clientRoutine, (void *) shd);
        pthread_detach(newTreadId);

        // добавить нового клиента в список сервера
        pthread_mutex_lock(&(shd->mutex));
        addClient(shd->list, client, clientAddr, newTreadId);
        pthread_mutex_unlock(&(shd->mutex));
    }

}

void * clientRoutine(void * dta) {
    SharedData * shd = (SharedData *) dta;
    pthread_mutex_lock(&(shd->mutex));
    SOCKET client = shd->sock;
    pthread_mutex_unlock(&(shd->mutex));
    char msg[1025], respond[1025];

    // цикл обработки диалога с клиентом
    while (!shd->shutdown) {
        //  принять сообщение от клиента
        int res = recv(client, msg, 1025, 0);
        if (!res || res == SOCKET_ERROR) {
            printf("!! ERROR RECEIVE MESSAGE, CLIENT DISCONNECTED\n");
            break;
        }

        // распечатать и обработать сообщение
        printf("@@ %llu: %s\n", client, msg);
        shd->sock = client;
        int r = handleRequest(msg, respond, shd);

        // послать обратно
        if (send(client, respond, (int) strlen(respond) + 1, 0) == SOCKET_ERROR) {
            printf("!! CANNOT SEND MESSAGE BACK\n");
            break;
        }

        // создать новый игровой поток, если нужно и ожидать его конца
        if (r == JOIN_TO_GAME) {
            pthread_t thread;
            Game * curGame = shd->gManager.game;
            curGame->n = shd->gManager.count;
            shd->gManager.count += 2;
            pthread_create(&thread, NULL, gameRoutine, (void *)curGame);
            pthread_detach(thread);
            while(!curGame->hasFinished);
            printf("this should be only printed when the game is over\n");
            free(curGame);
        }
        else if (r == JUST_WAIT) {
            Game * curGame = shd->gManager.game;
            while (!curGame->hasFinished);
            printf("waited thread was resumed\n");
        }
        else if (r)
            printf("error occurred: %d\n", r);
    }

    // клиент отключился, удаляем из списка
    pthread_mutex_lock(&(shd->mutex));
    ClientNode *node = popClient(shd->list, client);
    free(node);
    pthread_mutex_unlock(&(shd->mutex));
}

void * gameRoutine(void * dta) {
    Game * game = (Game *) dta;
    printf("game thread started\n");

    // настроить udp-сервер
    SOCKET s1, s2;
    s1 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    s2 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s1 == SOCKET_ERROR || s2 == SOCKET_ERROR) {
        printf("failed to create udp server(%d)\n", WSAGetLastError());
        return (void *) -1;
    }

    // привязать порты к созданным сокетам
    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(2206 + game->n);
    serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    if (bind(s1, (SOCKADDR *) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("failed to bound udp server, port %d (%d)\n", serverAddr.sin_port, WSAGetLastError());
        closesocket(s1), closesocket(s2);
        return (void *) -2;
    }
    serverAddr.sin_port = htons(2207 + game->n);
    if (bind(s2, (SOCKADDR *) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("failed to bound udp server, port %d (%d)\n", serverAddr.sin_port, WSAGetLastError());
        return (void *) -2;
    }
    printf("sockets bound to ports\n");

    // отправить каждому клиенту порт, по которому он будет обмениваться информацией
    char port[10] = {};
    sprintf(port, "%d", 2206 + game->n);
    if(send(game->client1, port, 10, 0) == SOCKET_ERROR) {
        printf("failed to send port for first client\n");
        return (void *) -3;
    }
    sprintf(port, "%d", 2207 + game->n);
    if (send(game->client2, port, 10, 0) == SOCKET_ERROR){
        printf("failed to send port for second client (%d)\n", WSAGetLastError());
        return (void *) -3;
    }
    printf ("all ports has been sent\n");

    // обмен игроков логинами
    char log1[21] = {}, log2[21] = {};
    int r1 = recv(game->client1, log1, 21, 0);
    int r2 = recv(game->client2, log2, 21, 0);
    if (!r1 || !r2 || r1 == SOCKET_ERROR || r2 == SOCKET_ERROR)
        printf("failed to receive logins of the players\n");
    if (send(game->client1, log2, (int) strlen(log2) + 1, 0) == SOCKET_ERROR ||
        send(game->client2, log1, (int) strlen(log1) + 1, 0) == SOCKET_ERROR) {
        printf("failed to send logins back\n");
    } else
        printf("logins were exchanged\n");

    // буферы для приема информации
    char buffer1[101] = {}, buffer2[101] = {};
    Player p1, p2;
    SOCKADDR_IN addr1, addr2; int size = sizeof(addr1);

    int err1 = 0, err2 = 0;
    printf("game started\n");
    while (1) {
        // получить информацию от обоих клиентов
        int len1, len2;
        len1 = recvfrom(s1, buffer1, 100, 0, (SOCKADDR *) &addr1, &size);
        len2 = recvfrom(s2, buffer2, 100, 0, (SOCKADDR *) &addr2, &size);
        if (strcmp(buffer1, "NO") == 0 || err1 > 30) { // первый отключился
            printf("client1 has sent no\n");
            sendto(s2, buffer1, 3, 0, (SOCKADDR *) &addr2, sizeof(addr2));
            break;
        }
        if (strcmp(buffer2, "NO") == 0 || err2 > 30) { // второй отключился
            printf("client2 has sent no\n");
            sendto(s1, buffer2, 3, 0, (SOCKADDR *) &addr1, sizeof(addr1));
            break;
        }
        err1 += len1 < 0;
        err2 += len2 < 0;
        if (len1 != sizeof(p1) || len2 != sizeof(p2)) // произошла ошибка!
            printf("error occurred while receiving (%d, %d)\n", len1, len2);
        else {
            // ошибок не было можно сохранить полученные данные в объекты игроков
            memcpy(&p1, buffer1, len1);
            memcpy(&p2, buffer2, len2);
            err1 = err2 = 0;
        }

        // отправить ответ каждому
        len1 = sendto(s1, (const char *) &p2, sizeof(p2), 0, (SOCKADDR *) &addr1, sizeof(addr1));
        len2 = sendto(s2, (const char *) &p1, sizeof(p1), 0, (SOCKADDR *) &addr2, sizeof(addr2));
        if (len1 != sizeof(p1) || len2 != sizeof(p2))
            printf("error occurred while sending (%d, %d)\n", len1, len2);
    }
    closesocket(s1);
    closesocket(s2);
    printf("game ended\n");
    game->hasFinished = 1;
}
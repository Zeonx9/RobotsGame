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
            printf("!! ERROR RECEIVE MESSAGE, CLIENT DISCONNECTED (%llu)\n", client);
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
            printf("this should be only printed when the game is over (%llu)\n", client);
            free(curGame);
        }
        else if (r == JUST_WAIT) {
            Game * curGame = shd->gManager.game;
            while (!curGame->hasFinished);
            printf("waited thread was resumed (%llu)\n", client);
        }
        else if (r)
            printf("error occurred: %d (%llu)\n", r, client);
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
        game->hasFinished = 1;
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
        game->hasFinished = 1;
        return (void *) -2;
    }
    serverAddr.sin_port = htons(2207 + game->n);
    if (bind(s2, (SOCKADDR *) &serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("failed to bound udp server, port %d (%d)\n", serverAddr.sin_port, WSAGetLastError());
        game->hasFinished = 1;
        return (void *) -2;
    }
    printf("sockets bound to ports\n");

    // отправить каждому клиенту порт, по которому он будет обмениваться информацией
    char portAndLogin[40] = {};
    sprintf(portAndLogin, "%d %s", 2206 + game->n, game->login2);
    if(send(game->client1, portAndLogin, 40, 0) == SOCKET_ERROR) {
        printf("failed to send port for first client\n");
        game->hasFinished = 1;
        return (void *) -3;
    }
    sprintf(portAndLogin, "%d %s", 2207 + game->n, game->login1);
    if (send(game->client2, portAndLogin, 40, 0) == SOCKET_ERROR){
        printf("failed to send port for second client (%d)\n", WSAGetLastError());
        game->hasFinished = 1;
        return (void *) -3;
    }
    printf ("all ports and logins has been sent\n");

    // буферы для приема информации
    char buffer1[101] = {}, buffer2[101] = {};
    Player p1, p2;
    SOCKADDR_IN addr1, addr2; int size = sizeof(addr1);

    int err1 = 0, err2 = 0;
//    u_long mode = 1;  // сделать сокет не блокирующим
//    ioctlsocket(s1, FIONBIO, &mode);
//    ioctlsocket(s2, FIONBIO, &mode);

    printf("game started\n");
    while (1) {
        // получить информацию от обоих клиентов
        int len1, len2;
        len1 = recvfrom(s1, buffer1, 100, 0, (SOCKADDR *) &addr1, &size);
        len2 = recvfrom(s2, buffer2, 100, 0, (SOCKADDR *) &addr2, &size);
        if (strcmp(buffer1, "NO") == 0) { // первый отключился
            printf("client1 has sent no\n");
            sendto(s2, buffer1, 3, 0, (SOCKADDR *) &addr2, sizeof(addr2));
            break;
        }
        if (strcmp(buffer2, "NO") == 0) { // второй отключился
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
        //  игра завершается победой одного из игроков
        if (p1.health < 1 || p2.health < 1) {
            printf("someone was killed p1.hp = %d; p2.hp = %d\n", p1.health, p2.health);

            PlayerData *pd = findPlayer(game->login1);
            updateData(game->id1, games, pd->gamesPlayed + 1);
            updateData(game->id1, highScore, pd->highScore + 5 - p2.health);
            if (p1.health > 0)
                updateData(game->id1, wins, pd->wins + 1);
            free(pd);

            pd = findPlayer(game->login2);
            updateData(game->id2, games, pd->gamesPlayed + 1);
            updateData(game->id2, highScore, pd->highScore + 5 - p1.health);
            if (p2.health > 0)
                updateData(game->id2, wins, pd->wins + 1);
            free(pd);

            sprintf(buffer1, "OVER");
            sendto(s2, buffer1, 5, 0, (SOCKADDR *) &addr2, sizeof(addr2));
            sendto(s1, buffer1, 5, 0, (SOCKADDR *) &addr1, sizeof(addr1));
            break;
        }

        // обработка попаданий и перемещение пуль
        for (Bullet *b = p1.bullets; b < p1.bullets + MAX_BULLETS; ++b) {
            if (!b->dir) continue;
            b->x += p1.t * b->dir * 0.003f;
            if (b->x > p2.x && b->x < p2.x + WIDTH && b->y > p2.y && b->y < p2.y + HEIGHT) {
                p2.health--;
                b->dir = 0;
                p2.damaged = 1;
            }
        }
        for (Bullet *b = p2.bullets; b < p2.bullets + MAX_BULLETS; ++b) {
            if (!b->dir) continue;
            b->x += p2.t * b->dir * 0.003f;
            if (b->x > p1.x && b->x < p1.x + WIDTH && b->y > p1.y && b->y < p1.y + HEIGHT) {
                p1.health--;
                b->dir = 0;
                p1.damaged = 1;
            }
        }

        // отправить ответ каждому
        len1 = sendto(s1, (const char *) &p2, sizeof(p2), 0, (SOCKADDR *) &addr1, sizeof(addr1));
        len2 = sendto(s2, (const char *) &p1, sizeof(p1), 0, (SOCKADDR *) &addr2, sizeof(addr2));
        if (len1 != sizeof(p1) || len2 != sizeof(p2))
            printf("error occurred while sending (%d, %d)\n", len1, len2);

        if (err1 > 100 || err2 > 100) {
            printf("to many errors\n");
            break;
        }
    }
    closesocket(s1);
    closesocket(s2);
    printf("game ended\n");
    game->hasFinished = 1;
}
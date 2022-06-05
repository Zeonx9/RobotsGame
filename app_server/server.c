#include "server.h"
#include "requests.h"

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
void * GameRoutine(void * dta) {
    Game * game = (Game *) dta;

    printf("game started\n");
    while (game->client2 == INVALID_SOCKET); // ожидание подключения второго клиента
    char in1[1025], in2[1025], no[] = "NO";

    while (1) {
        // получить информацию от обоих игроков
        int res1 = recv(game->client1, in1, 1025, 0);
        if (!res1 || res1 == SOCKET_ERROR) { // организатор отключился
            printf("!! player1 disconnected recv\n");
            send(game->client2, no, 3, 0);
            break;
        }

        int res2 = recv(game->client2, in2, 1025, 0);
        if (!res2 || res2 == SOCKET_ERROR) {
            printf("!! player2 disconnected recv\n");
            send(game->client1, no, 3, 0);
            break;
        }

        // отправить каждому игроку информацию о другом
        if (send(game->client1, in2, sizeof(Player), 0) == SOCKET_ERROR) {
            printf("!! player1 disconnected send\n");
            send(game->client2, no, 3, 0);
            break;
        }
        if (send(game->client2, in1, sizeof(Player), 0) == SOCKET_ERROR) {
            printf("!! player2 disconnected send\n");
            send(game->client1, no, 3, 0);
            break;
        }
    }

    printf("game ended\n");
    free(game);
}

JoinStates handleJoinRequest(SharedData * shd, SOCKET self, int id) {
    //printf("handling hasGame=%d, toNotify=%d, gamePtr=%p\n", shd->gManager.hasActiveGame, shd->gManager.notifyFirst, shd->gManager.game);
    if (shd->gManager.hasActiveGame) {
        if (shd->gManager.game->firstId == id) { // сравнение идет по ID
            if (shd->gManager.notifyFirst) {
                shd->gManager.hasActiveGame = 0;
                shd->gManager.game = NULL;
                return completed; // сказать, что 2ой игрок присоединился
            }
            return waiting; // повторный запрос
        }
        shd->gManager.game->client2 = self;
        shd->gManager.notifyFirst = 1;
        return justJoined; // игра уже была инициализирована, и клиент присоединился к существующей
    }

    shd->gManager.game = (Game *) malloc(sizeof(Game));
    shd->gManager.game->client1 = self;
    shd->gManager.game->firstId = id;
    shd->gManager.game->client2 = INVALID_SOCKET;
    shd->gManager.hasActiveGame = 1;
    shd->gManager.notifyFirst = 0;

    // создать поток в котором будет обрабатываться игра
    pthread_create(&shd->newGameThread, NULL, GameRoutine, (void *) shd->gManager.game);
    return justCreated; // игр не было, создается новая и ожидает второго игрока
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
        int r = handleRequest(msg, respond);
        if (r > JOIN_TO_GAME) { // обработать ситуацию подключения к игровой сессии
            int id  = r - JOIN_TO_GAME;
            JoinStates result = handleJoinRequest(shd, client, id);
            sprintf(respond, result == justCreated || result == waiting ? "W" : "C");
        }
        else if (r)
            printf("error occurred: %d\n", r);

        // послать обратно
        if (send(client, respond, (int) strlen(respond) + 1, 0) == SOCKET_ERROR) {
            printf("!! CANNOT SEND MESSAGE BACK");
            break;
        }

        // если был успешно начат новый игровой поток, то данный поток ожидает его завершения
        if (respond[0] == 'C')
            pthread_join(shd->newGameThread, NULL);
    }

    // клиент отключился, удаляем из списка
    pthread_mutex_lock(&(shd->mutex));
    ClientNode *node = popClient(shd->list, client);
    free(node);
    pthread_mutex_unlock(&(shd->mutex));
}

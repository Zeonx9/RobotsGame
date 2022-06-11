#include "requests.h"
#include <stdio.h>
#include <strings.h>

// обработчик запроса на вход (A)
int reqLogIn(char *in, char *out, SharedData *shd) {
    char log[20] = "", pass[20] = "";
    sscanf(in, "A %s %s", log, pass);

    // запросить пользователя с таким логином в базе данных
    PlayerData *pd = findPlayer(log);

    // проверить, что пользователь с таки логином найден
    if (pd->ID < 0) {
        strcpy(out, "N"); // не найден
        free(pd);
        return -1;
    }

    // проверить правильность пароля
    if (strcmp(pd->password, pass) != 0) {
        strcpy(out, "W"); // неверный пароль
        free(pd);
        return -2;
    }

    // вход успешен
    playerToString(pd, out);
    free(pd);
    return 0;
}

// обработчик запроса на регистрацию (B)
int reqReg(char *in, char *out, SharedData *shd) {
    char log[20] = "", pass[20] = "";
    sscanf(in, "B %s %s", log, pass);

    int r = registerUser(log, pass);
    if (r < 0) {
        strcpy(out, "E"); // уже существует
        return -3;
    }

    in[0] = 'A';
    return reqLogIn(in, out, shd);
}

// обработчик запроса рейтинга (C)
int reqRating(char * in, char * out, SharedData *shd) {
    int count = 5, len = 0;
    PlayerData ** pdArr = findBestPlayers(count);
    for (int i = 0; i < count && pdArr[i]->ID > -1; ++i) {
        PlayerData * pd = pdArr[i];
        // формат: позиция логин счет игры победы
        len += sprintf(out + len, "%d\t%20s\t%d\t%d\t%d\n", i+1, pd->login, pd->highScore, pd->gamesPlayed, pd->wins);
        free(pd);
    }
    free(pdArr);
    return 0;
}

// обработчик запроса на присоединение к игре или созданию (D)
int reqJoinGame(char * in, char * out, SharedData *shd) {
    SOCKET sock = shd->sock;
    int id;
    char login[21];
    sscanf(in, "D %d %s", &id, login);
    if (!shd->gManager.hasActiveGame) {
        shd->gManager.game = (Game *) malloc(sizeof(Game));
        shd->gManager.game->hasFinished = 0;
        shd->gManager.hasActiveGame = 1;
        shd->gManager.game->id1 = id;
        strcpy(shd->gManager.game->login1, login);
        shd->gManager.game->client1 = sock;
        shd->gManager.game->client2 = INVALID_SOCKET;
        sprintf(out, "W");
        return 0;
    }
    if (shd->gManager.game->id1 == id) {
        if (shd->gManager.game->client2 != INVALID_SOCKET) {
            sprintf(out, "C");
            shd->gManager.hasActiveGame = 0;
            return JOIN_TO_GAME;
        }
        sprintf(out, "W");
        return 0;
    }

    sprintf(out, "C");
    shd->gManager.game->id2 = id;
    strcpy(shd->gManager.game->login2, login);
    shd->gManager.game->client2 = sock;
    return JUST_WAIT;
}

// запрос отмены игры (E)
int reqCancelGame(char * in, char * out, SharedData *shd) {
    if (shd->gManager.hasActiveGame) {
        shd->gManager.hasActiveGame = 0;
        free(shd->gManager.game);
    }
    sprintf(out, "O");
    return 0;
}

// диспетчер обработчиков запросов
int handleRequest(char *in, char *out, SharedData *shd) {
    // массив указателей на функции
    int (*requestHandlers[])(char *, char *, SharedData*) = {
            reqLogIn, reqReg, reqRating, reqJoinGame, reqCancelGame
    };

    if (in[0] < 'A' || 'E' < in[0])
        return -356;
    return requestHandlers[in[0] - 'A'](in, out, shd);
}
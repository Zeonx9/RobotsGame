#include "requests.h"
#include <stdio.h>
#include <strings.h>
#include <malloc.h>

void PlayerToString(PlayerData *pd, char *dest) {
    sprintf(dest, "%d %s %s %d %d %d", pd->ID, pd->login, pd->password, pd->gamesPlayed, pd->wins, pd->highScore);
}

void StringToPlayer(PlayerData *pd, char *src) {
    sscanf(src, "%d %s %s %d %d %d", &pd->ID, pd->login, pd->password, &pd->gamesPlayed, &pd->wins, &pd->highScore);
}

// обработчик запроса на вход (A)
int reqLogIn(char *in, char *out) {
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
    PlayerToString(pd, out);
    free(pd);
    return 0;
}

// обработчик запроса на регистрацию (B)
int reqReg(char *in, char *out) {
    char log[20] = "", pass[20] = "";
    sscanf(in, "B %s %s", log, pass);

    int r = registerUser(log, pass);
    if (r < 0) {
        strcpy(out, "E"); // уже существует
        return -3;
    }

    in[0] = 'A';
    return reqLogIn(in, out);
}

int handleRequest(char *in, char *out) {
    return requestHandlers[in[0] - 'A'](in, out);
}

int (*requestHandlers[])(char *, char *) = {
        reqLogIn, reqReg
};
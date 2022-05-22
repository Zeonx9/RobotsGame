#include "requests.h"
#include <stdio.h>

// обработчик запроса на вход (A)
int reqLogIn(char *in, char *out) {
    sprintf(out, "1");
    return 0;
}

// обработчик запроса на регистрацию (B)
int reqReg(char *in, char *out) {
    sprintf(out, "1");
    return 0;
}

int handleRequest(char *in, char *out) {
    return requestHandlers[in[0] - 'A'](in, out);
}

int (*requestHandlers[])(char *, char *) = {
        reqLogIn, reqReg
};
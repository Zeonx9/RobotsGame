#ifndef ROBOTSGAME_REQUESTS_H
#define ROBOTSGAME_REQUESTS_H


// метод вызывает соответствующий обработчик запроса
int handleRequest(char *in, char *out);

// массив указателей на функции-обработчики
extern int (*requestHandlers[])(char *, char *);


#endif //ROBOTSGAME_REQUESTS_H

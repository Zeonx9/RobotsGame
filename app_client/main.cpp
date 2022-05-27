#include "interface.h"

// точка входа в клиентское приложение
int main() {
    // подключение библиотеки ws2_32.lib
    WSADATA wsd;
    if (WSAStartup(MAKEWORD(2, 2), &wsd)) {
        printf("!! CANNOT CONNECT TO THE LIB");
        return 1;
    }

    // выделить память для объекта связи с потоком работающим с сетью
    SharedState * shs = (SharedState *) calloc(1, sizeof(SharedState));
    pthread_mutex_init(&(shs->mutex), NULL);

    // создать поток для работы с сетью
    pthread_t tid;
    pthread_create(&tid, NULL, requestsRoutine, (void *) shs);
    pthread_detach(tid);

    // создать окно
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Client");
    window.setFramerateLimit(40);
    // загрузить иконку
    sf::Image icon;
    icon.loadFromFile("../app_client/src/rgame_icon64.png");
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

    // отрисовка интерфейса стартового окна
    createMenuApp(window, shs);

    // ждать, пока поток связи не закроется
    while(shs->currentActivity != closeApproved);

    // очистить память при выходе из приложения
    pthread_mutex_destroy(&(shs->mutex));
    free(shs);
    return 0;
}
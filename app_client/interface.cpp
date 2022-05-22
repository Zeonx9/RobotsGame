#include "interface.h"
#include "client.h"
#include <pthread.h>

// поток связи с сервером
void * requestsRoutine(void * dta) {
    SharedState * shs = (SharedState *) dta;

    while (1) {
        pthread_mutex_lock(&(shs->mutex));
        shs->sock = connectToServer();
        pthread_mutex_unlock(&(shs->mutex));
        if (shs->sock != INVALID_SOCKET)
            break;
        printf("try again?\n");
        getchar();
    }
    shs->connected++;

    // начать цикл для обработки запросов, пока не закроют приложение
    char bufIn[1025] = "", bufOut[1025] = "";
    while(shs->currentActivity != closeApp){
        // логин на сервер
        if (shs->currentActivity == logIn || shs->currentActivity == registration) {
            char login[20], password[20];
            printf("enter your login:");
            scanf("%s", login);
            printf("enter your password:");
            scanf("%s", password);
            sprintf(bufIn, "%c %s %s", (shs->currentActivity == logIn ? 'A' : 'B'), login, password);

            serverSession(shs->sock, bufIn, bufOut);
            printf("<<<%s\n", bufOut);
            //TODO эта строка должна быть в соответствующей активности
            shs->currentActivity = mainMenu;
        }
    }

    // закрыть сокет
    pthread_mutex_lock(&(shs->mutex));
    closesocket(shs->sock);
    shs->connected = 0;
    shs->currentActivity = closeApproved;
    pthread_mutex_unlock(&(shs->mutex));

    printf(">> Client stopped\n");
    return (void *) 0;
}

// пространство имен sfml
using namespace sf;

void createMenuApp(RenderWindow &window, SharedState * shs) {
    Texture texture;
    texture.loadFromFile("../app_client/src/factory_bg.jpg");

    Vector2u txSize = texture.getSize();
    Vector2u winSize = window.getSize();

    Sprite sprite;
    sprite.setTexture(texture);
    float scaling = std::max((float) winSize.x / (float) txSize.x, (float) winSize.y / (float) txSize.y);
    sprite.scale(scaling, scaling);

    Font font1, font2;
    font1.loadFromFile("../app_client/src/game_font.otf");
    font2.loadFromFile("../app_client/src/circle_font.otf");

    Text header("Robots   Game", font1, 40),
        button1("Log me in", font2, 30),
        button2("Exit", font2, 30);

    header.setFillColor(Color(120, 50, 20));
    header.setPosition(200, 200);

    button1.setFillColor(Color(10, 10, 10));
    button1.setPosition(330, 330);
    button1.setStyle(Text::Bold);

    button2.setFillColor(Color(10, 10, 10));
    button2.setPosition(360, 380);
    button2.setStyle(Text::Bold);


    while (window.isOpen()) {
        Event event{};
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                pthread_mutex_lock(&(shs->mutex));
                shs->currentActivity = closeApp; // сигнал для закрытия потока связи с сервером
                pthread_mutex_unlock(&(shs->mutex));
                window.close();
            }
            if (event.type == Event::MouseMoved) {
                if (330 < event.mouseMove.x && event.mouseMove.x < 460 &&
                    330 < event.mouseMove.y && event.mouseMove.y < 360) {
                    button1.setCharacterSize(35);
                    button1.setFillColor(Color(120, 50, 20));
                }
                else if (360 < event.mouseMove.x && event.mouseMove.x < 410 &&
                    380 < event.mouseMove.y && event.mouseMove.y < 410) {
                    button2.setCharacterSize(35);
                    button2.setFillColor(Color(120, 50, 20));
                }
                else {
                    button1.setFillColor(Color(10, 10, 10));
                    button2.setFillColor(Color(10, 10, 10));
                    button1.setCharacterSize(30);
                    button2.setCharacterSize(30);
                }
                // printf("mouse %d, %d\na", event.mouseMove.x, event.mouseMove.y);
            }
            if (event.type == Event::MouseButtonPressed &&
                330 < event.mouseButton.x && event.mouseButton.x < 460 &&
                330 < event.mouseButton.y && event.mouseButton.y < 360) {
                printf("clicked login\n");
                pthread_mutex_lock(&(shs->mutex));
                shs->currentActivity = registration;
                pthread_mutex_unlock(&(shs->mutex));
            }
        }

        window.clear();
        window.draw(sprite);
        window.draw(header);
        window.draw(button1);
        window.draw(button2);
        window.display();
    }
}

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
    RenderWindow window(VideoMode(800, 500), "Client");
    window.setFramerateLimit(40);
    // загрузить иконку
    Image icon;
    icon.loadFromFile("../app_client/src/rgame_icon64.png");
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

    // начать рисовать интерфейс
    createMenuApp(window, shs);

    // ждать, пока поток связи не закроется
    while(shs->currentActivity != closeApproved);

    // очистить память при выходе из приложения
    pthread_mutex_destroy(&(shs->mutex));
    free(shs);
    return 0;
}
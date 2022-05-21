#include "interface.h"
#include "client.h"
#include <pthread.h>

void * routine(void * param) {
    auto state = (ClientState *) param;

    sf::Clock clk;
    while (state->sock == INVALID_SOCKET) {
        state->sock = connectToServer();
        while(clk.getElapsedTime().asSeconds() < 5);
        clk.restart();
    }
    state->connected++;

    // начать цикл для обработки запросов
    for(int res = 0; !res; ) {
        printf("@ Enter message to be processed on server : ");
        fgets(state->buf, 1024, stdin); // получить сообщение от пользователя

        // обработать запрос
        res = serverSession(state->sock, state->buf);
        if (res == 1)
            break;

        printf("@ processed message from server:\n\t%s\n", state->buf);
    }

    // закрыть сокет
    closesocket(state->sock);
    printf(">> Client stopped\n");

    state->connected = 0;
    return (void *) 0;
}

// пространство имен sfml
using namespace sf;

void createMenuApp(RenderWindow &window) {
    Texture texture;
    texture.loadFromFile("../app_client/src/sky_bg.jpg");

    Vector2u txSize = texture.getSize();
    Vector2u winSize = window.getSize();

    Sprite sprite;
    sprite.setTexture(texture);
    float scaling = std::max((float) winSize.x / txSize.x, (float) winSize.y / txSize.y);
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

    button2.setFillColor(Color(10, 10, 10));
    button2.setPosition(360, 380);


    while (window.isOpen()) {
        Event event{};
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
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
        }

        window.clear();
        window.draw(sprite);
        window.draw(header);
        window.draw(button1);
        window.draw(button2);
        window.display();
    }
}

void createConnectingApp(ClientState *state, RenderWindow &window) {
    // настройка шрифта и текста для стадии соединения
    Font font;
    font.loadFromFile("../app_client/src/arial.ttf");

    Text text1("", font, 20);
    text1.setFillColor(Color(200, 200, 200));
    text1.setPosition(300, 150);
    Text text2("", font, 20);
    text2.setFillColor(Color(150, 20, 180));
    text2.setPosition(300, 300);

    // обработка окна приложения в цикле
    while (window.isOpen()) {
        Event event{};
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            }
        }

        window.clear(Color(40, 40, 40));
        text1.setString(state->connected ? "You are connected!" : "Cannot connect to server :( " );
        window.draw(text1);

        if(state->connected) {
            text2.setString(state->buf);
            window.draw(text2);
        }

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
    auto cs = (ClientState *) malloc(sizeof(ClientState));
    cs->sock = INVALID_SOCKET; cs->connected = 0;
    cs->buf = (char *) calloc(1025, sizeof(char));

    // создать поток для работы с сетью
    pthread_t tid;
    pthread_create(&tid, NULL, routine, cs);
    pthread_detach(tid);

    // создать окно
    RenderWindow window(VideoMode(800, 500), "Client");
    window.setFramerateLimit(40);
    // загрузить иконку
    Image icon;
    icon.loadFromFile("../app_client/src/rgame_icon64.png");
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());

    // начать рисовать интерфейс
    createConnectingApp(cs, window);
//    createMenuApp(window);

    // очистить память при выходе из приложения
    free(cs->buf);
    free(cs);
    return 0;
}
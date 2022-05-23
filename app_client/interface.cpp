#include <SFML/Audio.hpp>
#include "interface.h"

void createMenuApp(sf::RenderWindow &window, SharedState * shs) {
    sf::Texture texture;
    sf::Sprite sprite;
    sf::Music music;
    sf::Font font1, font2;
    int state = -1;

    texture.loadFromFile("../app_client/src/factory_bg.jpg");
    music.openFromFile("../app_client/src/menu_music.wav");
    font1.loadFromFile("../app_client/src/game_font.otf");
    font2.loadFromFile("../app_client/src/circle_font.otf");

    music.setLoop(true);
    music.play();

    sf::Vector2u txSize = texture.getSize();
    sf::Vector2u winSize = window.getSize();
    float scaling = std::max((float) winSize.x / (float) txSize.x, (float) winSize.y / (float) txSize.y);

    sprite.setTexture(texture);
    sprite.scale(scaling, scaling);

    sf::Text
        header("Robots   Game", font1, 40),
        button1("Log me in", font2, 30),
        button2("Exit", font2, 30),
        connected("", font2, 20);

    header.setFillColor(sf::Color(120, 50, 20));
    header.setPosition(200, 200);

    connected.setPosition(650, 450);

    button1.setFillColor(sf::Color(10, 10, 10));
    button1.setPosition(330, 330);
    button1.setStyle(sf::Text::Bold);

    button2.setFillColor(sf::Color(10, 10, 10));
    button2.setPosition(360, 380);
    button2.setStyle(sf::Text::Bold);


    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                shs->currentActivity = closeApp; // сигнал для закрытия потока связи с сервером
                window.close();
            }
            if (event.type == sf::Event::MouseMoved) {
                if (330 < event.mouseMove.x && event.mouseMove.x < 460 &&
                    330 < event.mouseMove.y && event.mouseMove.y < 360) {
                    button1.setCharacterSize(35);
                    button1.setFillColor(sf::Color(120, 50, 20));
                }
                else if (360 < event.mouseMove.x && event.mouseMove.x < 410 &&
                    380 < event.mouseMove.y && event.mouseMove.y < 410) {
                    button2.setCharacterSize(35);
                    button2.setFillColor(sf::Color(120, 50, 20));
                }
                else {
                    button1.setFillColor(sf::Color(10, 10, 10));
                    button2.setFillColor(sf::Color(10, 10, 10));
                    button1.setCharacterSize(30);
                    button2.setCharacterSize(30);
                }
                // printf("mouse %d, %d\na", event.mouseMove.x, event.mouseMove.y);
            }
            if (event.type == sf::Event::MouseButtonPressed &&
                330 < event.mouseButton.x && event.mouseButton.x < 460 &&
                330 < event.mouseButton.y && event.mouseButton.y < 360) {
                printf("clicked login\n");
                pthread_mutex_lock(&(shs->mutex));
                shs->currentActivity = logIn;
                pthread_mutex_unlock(&(shs->mutex));
            }
        }
        if (state != shs->connected) {
            state = shs->connected;
            connected.setString(state ? "connected" : "disconnected");
            connected.setFillColor(state ? sf::Color::Green : sf::Color::Red);
        }

        window.clear();
        window.draw(sprite);
        window.draw(header);
        window.draw(button1);
        window.draw(button2);
        window.draw(connected);
        window.display();
    }
}

// поток связи с сервером
void * requestsRoutine(void * dta) {
    SharedState * shs = (SharedState *) dta;

    // начать цикл для обработки запросов, пока не закроют приложение
    char bufIn[1025] = "", bufOut[1025] = "";
    while(shs->currentActivity != closeApp){

        // цикл пытается установить соединение, пока оно не установлено или приложение не будет закрываться
        while (!shs->connected && shs->currentActivity != closeApp) {
            shs->sock = connectToServer();
            if (shs->sock != INVALID_SOCKET)
                shs->connected++;
        }

        // логин на сервер
        if (shs->currentActivity == logIn) {
            char login[20], password[20];
            printf("enter your login:");
            scanf("%s", login);
            printf("enter your password:");
            scanf("%s", password);
            sprintf(bufIn, "%c %s %s", (shs->currentActivity == logIn ? 'A' : 'B'), login, password);

            int res = serverSession(shs->sock, bufIn, bufOut);
            if (res) shs->connected = 0;

            printf("<<< %s\n", bufOut);

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
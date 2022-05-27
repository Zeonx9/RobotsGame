#include <SFML/Audio.hpp>
#include "interface.h"
#include "intfc_classes.h"

void createRegWindow(sf::RenderWindow &window, SharedState * shs) {
    // TODO сделать, чтобы музыка продолжала играть непрерывно
    sf::Texture texture;
    sf::Sprite sprite;
    sf::Font font;
    int state = -1;

    texture.loadFromFile("../app_client/src/background_log.png");
    font.loadFromFile("../app_client/src/gameFont.otf");

    sf::Vector2u txSize = texture.getSize();
    sf::Vector2u winSize = window.getSize();
    float scaling = std::max((float) winSize.x / (float) txSize.x, (float) winSize.y / (float) txSize.y);

    sprite.setTexture(texture);
    sprite.scale(scaling, scaling);

    sf::Text
        header("login or registration", font, 100),
        connected("", font, 40), // показывает, есть ли соединение с сервером
        version("version 1.0", font, 40),
        text1("enter your username", font, 30),
        text2("enter your password", font, 30);

    header.setFillColor(sf::Color(178, 189, 231));
    header.setPosition(447, 150);

    connected.setPosition(1681, 970);

    version.setFillColor(sf::Color(255, 255, 255));
    version.setPosition(1681, 1013);

    text1.setFillColor(sf::Color(122, 122, 122));
    text1.setPosition(818, 427);

    text2.setFillColor(sf::Color(122, 122, 122));
    text2.setPosition(821, 522);

    Button login ("login", font, 70, 0, 171, 57);
    Button reg ("register", font, 70, 0, 274, 57);
    Button back ("back", font, 70, 0, 137, 57);

    login.setPosition(682, 607);
    reg.setPosition(966, 607);
    back.setPosition(45, 944);

    char* username = (char*) malloc(100);
    char* password = (char*) malloc(100);

    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                shs->currentActivity = closeApp; // сигнал для закрытия потока связи с сервером
                window.close();
            }
            if (event.type == sf::Event::MouseMoved) {
                login.mouseOnButton(event.mouseMove.x, event.mouseMove.y);
                reg.mouseOnButton(event.mouseMove.x, event.mouseMove.y);
                back.mouseOnButton(event.mouseMove.x, event.mouseMove.y);
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (login.isClick(event.mouseButton.x, event.mouseButton.y)){
//                    printf("clicked login\n");
//                    pthread_mutex_lock(&(shs->mutex));
//                    shs->currentActivity = logIn;
//                    pthread_mutex_unlock(&(shs->mutex));
                    printf("Input your username: ");
                    scanf("%s\n", &username);
                    printf("Input your password: ");
                    scanf("%s\n", &password);
                    shs->logged = 1;
                    createMenuApp(window, shs);
                    // TODO войти в аккаунт
                }
                if (reg.isClick(event.mouseButton.x, event.mouseButton.y)){
                    printf("Input your username: ");
                    scanf("%s\n", &username);
                    printf("Input your password: ");
                    scanf("%s\n", &password);
                    shs->logged = 1;
                    createMenuApp(window, shs);
                    // TODO зарегистрировать пользователя
                }
                if (back.isClick(event.mouseButton.x, event.mouseButton.y)){
                    createMenuApp(window, shs);
                }

            }
        }
        if (state != shs->connected) {
            state = shs->connected;
            connected.setString(state ? "   connected" : "disconnected");
            connected.setFillColor(state ? sf::Color(143, 200, 99) : sf::Color(176, 52, 37));
        }

        window.clear();
        window.draw(sprite);
        window.draw(header);
        window.draw(connected);
        window.draw(version);
        window.draw(login.draw());
        window.draw(reg.draw());
        window.draw(back.draw());
        window.draw(text1);
        window.draw(text2);
        window.display();

    }

}


void createMenuApp(sf::RenderWindow &window, SharedState * shs) {
    sf::Texture texture;
    sf::Sprite sprite;
    sf::Music music;
    sf::Font font;
    int state = -1;

    texture.loadFromFile("../app_client/src/background_sm.png");
    music.openFromFile("../app_client/src/menu_music.wav");
    font.loadFromFile("../app_client/src/gameFont.otf");

    music.setLoop(true);
    music.play();

    sf::Vector2u txSize = texture.getSize();
    sf::Vector2u winSize = window.getSize();
    float scaling = std::max((float) winSize.x / (float) txSize.x, (float) winSize.y / (float) txSize.y);

    sprite.setTexture(texture);
    sprite.scale(scaling, scaling);

    sf::Text
        header("Robots Game", font, 150),
        connected("", font, 40), // показывает, есть ли соединение с сервером
        version("version 1.0", font, 40);

    header.setFillColor(sf::Color(178, 189, 231));
    header.setPosition(150, 150);

    connected.setPosition(1681, 970);

    version.setFillColor(sf::Color(255, 255, 255));
    version.setPosition(1681, 1013);

    Button startGame ("Start game", font, 70, -1, 342, 57);
    Button login ("Log in", font, 70, 0, 206, 57);
    Button exit ("Exit", font, 70, 0, 137, 57);

    if (shs->logged != 0) startGame.changeCondition(0);

    startGame.setPosition(155, 442);
    login.setPosition(155, 512);
    exit.setPosition(155, 582);

    while (window.isOpen()) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                shs->currentActivity = closeApp; // сигнал для закрытия потока связи с сервером
                window.close();
            }
            if (event.type == sf::Event::MouseMoved) {
                startGame.mouseOnButton(event.mouseMove.x, event.mouseMove.y);
                login.mouseOnButton(event.mouseMove.x, event.mouseMove.y);
                exit.mouseOnButton(event.mouseMove.x, event.mouseMove.y);
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (startGame.isClick(event.mouseButton.x, event.mouseButton.y)) {
                    // TODO окно ожидания игры
                }
                if (login.isClick(event.mouseButton.x, event.mouseButton.y)){
                    printf("clicked login\n");
                    pthread_mutex_lock(&(shs->mutex));
                    shs->currentActivity = logIn;
                    pthread_mutex_unlock(&(shs->mutex));
                    createRegWindow(window, shs);
                }
                if (exit.isClick(event.mouseButton.x, event.mouseButton.y)){
                    // TODO сделать выход из программы
                }

            }
        }
        if (state != shs->connected) {
            state = shs->connected;
            connected.setString(state ? "   connected" : "disconnected");
            connected.setFillColor(state ? sf::Color(143, 200, 99) : sf::Color(176, 52, 37));
        }

        window.clear();
        window.draw(sprite);
        window.draw(header);
        window.draw(connected);
        window.draw(version);
        window.draw(startGame.draw());
        window.draw(login.draw());
        window.draw(exit.draw());
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
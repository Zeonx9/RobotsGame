#include <SFML/Audio.hpp>
#include "interface.h"
#include "intfc_classes.h"

// объявления функций для отрисовки окон
void createMenuApp(sf::RenderWindow &window, SharedState * shs);
void createRegWindow(sf::RenderWindow &window, SharedState * shs);

// диспетчер окон приложения
void windowDispatcher(SharedState * shs) {
    // массив указателей на функции для отрисовки соответствующих окон
    void (* windowFuncs[]) (sf::RenderWindow&, SharedState*) = {
            createMenuApp, createRegWindow
    };

    // создать окно
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Client");
    // загрузить иконку
    sf::Image icon;
    icon.loadFromFile("../app_client/src/rgame_icon64.png");
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    window.setFramerateLimit(40);

    sf::Music music;
    music.openFromFile("../app_client/src/menu_music.wav");
    music.setLoop(true);
    music.play();
    int isPlaying = 1;

    // если приложение в валидном для отрисовки состоянии, то создаем его
    while(shs->currentActivity > closeApp) {
        if (shs->currentActivity == play) {
            music.stop();
            isPlaying = 0;
        }
        windowFuncs[shs->currentActivity](window, shs);
        if (!isPlaying)
            music.play();
    }
    window.close();
}

// вход в поток связи с сервером
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
        if (shs->currentActivity == logIn || shs->currentActivity == registering) {
            char login[21], password[21];
            printf("enter your login:");
            scanf("%s", login);
            printf("enter your password:");
            scanf("%s", password);
            sprintf(bufIn, "%c %s %s", (shs->currentActivity == logIn ? 'A' : 'B'), login, password);

            int res = serverSession(shs->sock, bufIn, bufOut);
            if (res)
                shs->connected = 0;

            printf("<<< %s\n", bufOut);

            //pthread_mutex_lock(&(shs->mutex));
            if (bufOut[0] == 'N')
                shs->logged = notLogged;
            else if (bufOut[0] == 'W')
                shs->logged = wrongPassword;
            else if (bufOut[0] == 'E')
                shs->logged = alreadyExists;
            else
                shs->logged = success;
            shs->currentActivity = mainMenu;
            //pthread_mutex_unlock(&(shs->mutex));
        }
    }

    // закрыть сокет
    //pthread_mutex_lock(&(shs->mutex));
    closesocket(shs->sock);
    shs->connected = 0;
    shs->currentActivity = closeApproved;
    //pthread_mutex_unlock(&(shs->mutex));

    printf(">> Client stopped\n");
    return (void *) 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// окно для входа и регистрации
void createRegWindow(sf::RenderWindow &window, SharedState * shs) {
    sf::Texture texture;
    sf::Sprite sprite;
    sf::Font font;
    int drawConnectionState = -1;

    texture.loadFromFile("../app_client/src/background_log.png");
    font.loadFromFile("../app_client/src/gameFont.otf");

    sprite.setTexture(texture);

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

    while (window.isOpen() && (shs->currentActivity == logHub || shs->currentActivity > play)) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                //pthread_mutex_lock(&(shs->mutex));
                shs->currentActivity = closeApp; // сигнал для закрытия потока связи с сервером
                //pthread_mutex_unlock(&(shs->mutex));
            }
            if (event.type == sf::Event::MouseMoved) {
                login.mouseOnButton(event.mouseMove.x, event.mouseMove.y);
                reg.mouseOnButton(event.mouseMove.x, event.mouseMove.y);
                back.mouseOnButton(event.mouseMove.x, event.mouseMove.y);
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (login.isClick(event.mouseButton.x, event.mouseButton.y)){
                    if (shs->connected) {
                        //pthread_mutex_lock(&(shs->mutex));
                        shs->currentActivity = logIn; // будет сформирован запрос на вход
                        //pthread_mutex_unlock(&(shs->mutex));
                    } else
                        printf("Not connected! Cannot send request!\n");
                }
                if (reg.isClick(event.mouseButton.x, event.mouseButton.y)){
                    if (shs->connected) {
                        //pthread_mutex_lock(&(shs->mutex));
                        shs->currentActivity = registering; // будет сформирован запрос на регистрацию
                        //pthread_mutex_unlock(&(shs->mutex));
                    } else
                        printf("Not connected! Cannot send request!\n");
                }
                if (back.isClick(event.mouseButton.x, event.mouseButton.y)){
                    //pthread_mutex_lock(&(shs->mutex));
                    shs->currentActivity = mainMenu;
                    //pthread_mutex_unlock(&(shs->mutex));
                }
            }
        }

        if (drawConnectionState != shs->connected) {
            drawConnectionState = shs->connected;
            connected.setString(drawConnectionState ? "   connected" : "disconnected");
            connected.setFillColor(drawConnectionState ? sf::Color(143, 200, 99) : sf::Color(176, 52, 37));
        }

        if (shs->logged != notLogged && shs->logged != success) {
            if (shs->logged == noSuchUser) // нет такого логина
                printf("No such user!\n");
            else if (shs->logged == wrongPassword) // не правильный пароль
                printf("Wrong password!\n");
            else if (shs->logged == alreadyExists)  // уже занят логин
                printf("This username already taken!\n");

            shs->logged = notLogged; // сигнал о том, что вход не выполнен
        }
        else if (shs->logged == success) {
            printf("Logged in successfully\n");
            //pthread_mutex_lock(&(shs->mutex));
            shs->currentActivity = mainMenu;
            //pthread_mutex_unlock(&(shs->mutex));
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

// окно для главного меню
void createMenuApp(sf::RenderWindow &window, SharedState * shs) {
    sf::Texture texture;
    sf::Sprite sprite;
    sf::Font font;
    int DrawnConnectionState = -1;

    texture.loadFromFile("../app_client/src/background_sm.png");
    font.loadFromFile("../app_client/src/gameFont.otf");

    sprite.setTexture(texture);

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

    if (shs->logged == success) startGame.changeCondition(0);

    startGame.setPosition(155, 442);
    login.setPosition(155, 512);
    exit.setPosition(155, 582);

    while (window.isOpen() && shs->currentActivity == mainMenu) {
        sf::Event event{};
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                //pthread_mutex_lock(&(shs->mutex));
                shs->currentActivity = closeApp; // сигнал для закрытия потока связи с сервером
                //pthread_mutex_unlock(&(shs->mutex));
            }
            if (event.type == sf::Event::MouseMoved) {
                startGame.mouseOnButton(event.mouseMove.x, event.mouseMove.y);
                login.mouseOnButton(event.mouseMove.x, event.mouseMove.y);
                exit.mouseOnButton(event.mouseMove.x, event.mouseMove.y);
            }
            if (event.type == sf::Event::MouseButtonPressed) {
                if (exit.isClick(event.mouseButton.x, event.mouseButton.y)){
                    //pthread_mutex_lock(&(shs->mutex));
                    shs->currentActivity = closeApp; // сигнал для закрытия потока связи с сервером
                    //pthread_mutex_unlock(&(shs->mutex));
                }
                if (startGame.isClick(event.mouseButton.x, event.mouseButton.y))
                    printf("Not Implemented 'START GAME'\n");

                if (login.isClick(event.mouseButton.x, event.mouseButton.y)) {
                    //pthread_mutex_lock(&(shs->mutex));
                    shs->currentActivity = logHub;
                    //pthread_mutex_unlock(&(shs->mutex));
                }
            }
        }

        // обновить информацию о подключении, если она не совпадает с отображаемой
        if (DrawnConnectionState != shs->connected) {
            DrawnConnectionState = shs->connected;
            connected.setString(DrawnConnectionState ? "   connected" : "disconnected");
            connected.setFillColor(DrawnConnectionState ? sf::Color(143, 200, 99) : sf::Color(176, 52, 37));
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
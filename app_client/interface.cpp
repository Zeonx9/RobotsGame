#include <SFML/Audio.hpp>
#include "interface.h"
#include "intfc_classes.h"

// объявления функций для отрисовки окон
void createMenuApp(sf::RenderWindow &window, SharedState * shs);
void createRegWindow(sf::RenderWindow &window, SharedState * shs);
void createLobby(sf::RenderWindow &window, SharedState * shs);
void beginGame(sf::RenderWindow &window, SharedState * shs);

// диспетчер окон приложения
void windowDispatcher(SharedState * shs) {
    // массив указателей на функции для отрисовки соответствующих окон
    void (* windowFuncs[]) (sf::RenderWindow&, SharedState*) = {
            createMenuApp, createRegWindow, createLobby, beginGame
    };

    // создать окно
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Client", sf::Style::Fullscreen);
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
    while(shs->act > closeApp) {
        if (shs->act == play) {
            music.stop();
            isPlaying = 0;
        }
        windowFuncs[shs->act](window, shs);
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
    while(shs->act != closeApp){

        // цикл пытается установить соединение, пока оно не установлено или приложение не будет закрываться
        while (!shs->connected && shs->act != closeApp) {
            shs->sock = connectToServer();
            if (shs->sock != INVALID_SOCKET)
                shs->connected++;
        }

        // блок обработки команд, заключен в мьютекс
        pthread_mutex_lock(&(shs->mutex));
        // логин на сервер
        if (shs->act == logIn || shs->act == registering) {
            sprintf(bufIn, "%c %s", (shs->act == logIn ? 'A' : 'B'), shs->logInfo);
            free(shs->logInfo); shs->logInfo = NULL;

            int res = serverSession(shs->sock, bufIn, bufOut);
            if (res) { // соединение с сервером разорвано
                shs->connected = 0;
                shs->logged = notLogged;
                continue;
            }

            printf("<<< %s\n", bufOut);

            if (bufOut[0] == 'N')
                shs->logged = noSuchUser;
            else if (bufOut[0] == 'W')
                shs->logged = wrongPassword;
            else if (bufOut[0] == 'E')
                shs->logged = alreadyExists;
            else {
                shs->logged = success;
                shs->player = (PlayerData *) malloc(sizeof(PlayerData));
                playerFromStr(shs->player, bufOut); // получить ID пользователя от сервера
            }

            shs->act = logHub;
        }
        // получить рейтинг
        else if (shs->act == getRating) {
            sprintf(bufIn, "C");

            int res = serverSession(shs->sock, bufIn, bufOut);
            if (res) {
                shs->connected = 0;
                shs->act = mainMenu;
                continue;
            }
            printf("<<< %s\n", bufOut);

            // память выделяется, не забыть очистить
            shs->rating = (char *) malloc((strlen(bufOut) + 1) * sizeof(char));
            strcpy(shs->rating, bufOut);
            shs->act = gameLobby;
        }
        // подключиться к игре или создать новую
        else if (shs->act == joinGame) {
            sprintf(bufIn, "D %d", shs->player->ID);
            int res = serverSession(shs->sock, bufIn, bufOut);
            if (res) {
                shs->connected = 0;
                shs->act = mainMenu;
                continue;
            }
            printf("<<< %s\n", bufOut);
            if (bufOut[0] == 'C')
                shs->gameStarted = 1;
            shs->act = gameLobby;
        }
        // отменить подключение к игре
        else if (shs->act == cancelGame) {
            sprintf(bufIn, "E %d", shs->player->ID);
            int res = serverSession(shs->sock, bufIn, bufOut);
            if (res) {
                shs->connected = 0;
                shs->act = mainMenu;
                continue;
            }
            if (bufOut[0] == 'O')
                shs->act = mainMenu;
        }
        pthread_mutex_unlock(&(shs->mutex));
    }

    // закрыть сокет
    pthread_mutex_lock(&(shs->mutex));
    closesocket(shs->sock);
    shs->connected = 0;
    shs->act = closeApproved;
    pthread_mutex_unlock(&(shs->mutex));

    printf(">> Client stopped\n");
    return (void *) 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// передвижение и анимация персонажа
void animatePlayer(Player * p, float t, sf::Sprite &s, float *frame) {
    t /= 500;
    updatePlayer(p, t);

    *frame += .0075f * t; // анимация
    if (*frame >= 12)
        *frame = 0;
    if (p->jumped){
        *frame = 0;
        p->jumped = 0;
    }

    if (p->onGround) {
        if (p->dx > 0)  s.setTextureRect(sf::IntRect(WIDTH * (int)(*frame), 0, WIDTH, HEIGHT));
        if (p->dx < 0)  s.setTextureRect(sf::IntRect(WIDTH * (int)(*frame + 1), 0, -WIDTH, HEIGHT));
        if (p->dx == 0) {
            if (p->dir > 0) s.setTextureRect(sf::IntRect(WIDTH * 13, 0, WIDTH, HEIGHT));
            else s.setTextureRect(sf::IntRect(WIDTH * 14, 0, -WIDTH, HEIGHT));
        }
    } else {
        if (p->dir > 0) s.setTextureRect(sf::IntRect(WIDTH * (int)(*frame), HEIGHT, WIDTH, HEIGHT));
        else s.setTextureRect(sf::IntRect(WIDTH * (int)(*frame+ 1), HEIGHT, -WIDTH, HEIGHT));
    }

    s.setPosition(p->x, p->y);
    p->dx = 0;
}

// начало самой игры
void beginGame(sf::RenderWindow &window, SharedState * shs){

    // TODO создание udp сокета и подключение его к серверу
    SOCKET client;

    // создать сокет для клиента и проверить на удачное создание
    client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client == INVALID_SOCKET){
        printf("!! ERROR CANNOT CREATE SOCKET\n");
    }

    // описание сервера для подключения
    SOCKADDR_IN server; int size = sizeof(server);
    server.sin_family = AF_INET;
    server.sin_port = htons(2206); // такой же порт как на сервере
    server.sin_addr.S_un.S_addr = inet_addr(IP); // Zeon's IP адрес

    sf::Texture bgTexture, playerTexture;
    sf::Sprite bgSprite, s1, s2;
    sf::Event ev{};
    sf::Clock clock1, clock2;

    bgTexture.loadFromFile("../app_client/src/background.png");
    playerTexture.loadFromFile("../app_client/src/robotgamesprites.png");
    bgSprite.setTexture(bgTexture);
    s1.setTexture(playerTexture);
    s2.setTexture(playerTexture);
    s2.setColor(sf::Color(255, 180, 180));

    Player player1, player2;
    float animation1, animation2;
    initPlayer(&player1);
    initPlayer(&player2);

    // отправить первый сигнал
    char no[3] = "NO";
    sendto(client, (const char *) &player1, sizeof(Player), 0, (SOCKADDR *) &server, sizeof(server));
    recvfrom(client, (char *) &player2, sizeof(Player), 0, (SOCKADDR *) &server, &size);
    printf("sockets tested\n");

    while(window.isOpen() && shs->act == play) {
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) {
                sendto(client, no, 3, 0, (SOCKADDR *) &server, sizeof(server));
                printf("no sent\n");
                pthread_mutex_lock(&(shs->mutex));
                shs->act = closeApp;
                pthread_mutex_unlock(&(shs->mutex));
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
            sendto(client, no, 3, 0, (SOCKADDR *) &server, sizeof(server));
            printf("no sent\n");
            pthread_mutex_lock(&(shs->mutex));
            shs->act = closeApp;
            pthread_mutex_unlock(&(shs->mutex));
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            walk(&player1, Right);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            walk(&player1, Left);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            leap(&player1);

        // отправить информацию о себе
        sendto(client, (const char *) &player1, sizeof(Player), 0, (SOCKADDR *) &server, sizeof(server));
        animatePlayer(&player1, (float) clock1.restart().asMicroseconds(), s1, &animation1);
        printf("sent\n");

        // получить информацию о сопернике
        recvfrom(client, (char *)&player2, sizeof(Player), 0, (SOCKADDR *) &server, &size);
        if (strcmp((char *)&player2, "NO") == 0) {
            printf("disconnected\n");
            pthread_mutex_lock(&(shs->mutex));
            if (shs->act > 0) shs->act = mainMenu;
            pthread_mutex_unlock(&(shs->mutex));
        }
        animatePlayer(&player2, (float) clock2.restart().asMicroseconds(), s2, &animation2);
        printf("received\n");

        window.clear();
        window.draw(bgSprite);
        window.draw(s2);
        window.draw(s1);
        window.display();
    }
    closesocket(client);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// окно ожидания второго игрока
void createLobby(sf::RenderWindow &window, SharedState * shs) {
    int drawConnectionState = -1;
    char myInfo[100];
    sf::Texture bgTexture;
    sf::Sprite bgSprite;
    sf::Font font;
    sf::Event ev{};
    sf::Clock clock;
    sf::Text
            points("", font, 100),
            connected("", font, 40),
            rating("", font, 50),
            me("", font, 60);
    Button
            back ("back", font, 70, 0, 137, 57);

    bgTexture.loadFromFile("../app_client/src/background_lobby.png");
    font.loadFromFile("../app_client/src/gameFont.otf");
    sprintf(myInfo, "ME\t%10s\t%d\t%d\t%d",
            shs->player->login, shs->player->highScore, shs->player->gamesPlayed, shs->player->wins);
    bgSprite.setTexture(bgTexture);

    connected.setPosition(1681, 970);
    points.setPosition(1128, 121);
    points.setFillColor(sf::Color(178, 189, 231));
    rating.setPosition(155, 512);
    back.setPosition(45, 944);
    me.setString(myInfo);
    me.setPosition(155, 402);
    me.setFillColor(sf::Color(143, 200, 99));

    // запросить рейтинг
    pthread_mutex_lock(&(shs->mutex));
    shs->act = getRating;
    pthread_mutex_unlock(&(shs->mutex));
    while (shs->act == getRating); // ожидание ответа от сервера
    if (!shs->connected)
        rating.setString("Cannot get current rating!\n");
    else {
        rating.setString(shs->rating);
        free(shs->rating); shs->rating = NULL;
    } shs->gameStarted = 0; // игра ещё не началась

    while (window.isOpen() && (shs->act == gameLobby || shs->act == joinGame || shs->act == cancelGame)) {
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) {
                pthread_mutex_lock(&(shs->mutex));
                shs->act = closeApp;
                pthread_mutex_unlock(&(shs->mutex));
            }
            else if (ev.type == sf::Event::MouseMoved) {
                back.mouseOnButton(ev.mouseMove.x, ev.mouseMove.y);
            }
            else if (ev.type == sf::Event::MouseButtonPressed) {
                // кнопка назад
                if (back.isClick(ev.mouseButton.x, ev.mouseButton.y)) {
                    pthread_mutex_lock(&(shs->mutex));
                    shs->act = cancelGame;
                    pthread_mutex_unlock(&(shs->mutex));
                }
            }
        }

        if (drawConnectionState != shs->connected) {
            drawConnectionState = shs->connected;
            connected.setString(drawConnectionState ? "   connected" : "disconnected");
            connected.setFillColor(drawConnectionState ? sf::Color(143, 200, 99) : sf::Color(176, 52, 37));
        }

        if (clock.getElapsedTime().asMilliseconds() > 700) {
            points.setString(points.getString().getSize() == 3 ? "" : points.getString() + ".");
            clock.restart();
        }

        // если игровая сессия уже готова, то начинаем, иначе отправим запрос
        pthread_mutex_lock(&(shs->mutex));
        if (shs->act == gameLobby) {
            shs->act = shs->gameStarted ? play : joinGame;
            printf("game state: %d, act: %d\n", shs->gameStarted, shs->act);
        }
        pthread_mutex_unlock(&(shs->mutex));

        window.clear();
        window.draw(bgSprite);
        window.draw(me);
        window.draw(rating);
        window.draw(connected);
        window.draw(points);
        window.draw(back.draw());
        window.display();
    }
}

// окно для входа и регистрации
void createRegWindow(sf::RenderWindow &window, SharedState * shs) {
    int drawConnectionState = -1;
    sf::Texture bgTexture;
    sf::Sprite bgSprite;
    sf::Font font;
    sf::Event ev{};
    sf::Text
            connected("", font, 40), // показывает, есть ли соединение с сервером
            errorText("", font, 50);  // текст ошибки
    Button
            login ("login", font, 70, 0, 171, 57),
            reg ("register", font, 70, 0, 274, 57),
            back ("back", font, 70, 0, 137, 57);
    TextBox
            log(font, 50, 557, 41),
            pass(font, 50, 557, 41);

    bgTexture.loadFromFile("../app_client/src/background_log.png");
    font.loadFromFile("../app_client/src/gameFont.otf");
    bgSprite.setTexture(bgTexture);

    connected.setPosition(1681, 970);
    errorText.setFillColor(sf::Color(176, 52, 37));
    errorText.setPosition(630, 724);
    login.setPosition(682, 607);
    reg.setPosition(966, 607);
    back.setPosition(45, 944);
    log.setPosition(682, 361);
    pass.setPosition(682, 456);

    while (window.isOpen() && (shs->act == logHub || shs->act > play)) {
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) {
                pthread_mutex_lock(&(shs->mutex));
                shs->act = closeApp;
                pthread_mutex_unlock(&(shs->mutex));
            }
            else if (ev.type == sf::Event::MouseMoved) {
                login.mouseOnButton(ev.mouseMove.x, ev.mouseMove.y);
                reg.mouseOnButton(ev.mouseMove.x, ev.mouseMove.y);
                back.mouseOnButton(ev.mouseMove.x, ev.mouseMove.y);
            }
            else if (ev.type == sf::Event::MouseButtonPressed) {
                // обработать текстовые клики по текстовым полям
                log.changeCondition(0);
                pass.changeCondition(0);
                log.isClick(ev.mouseButton.x, ev.mouseButton.y);
                pass.isClick(ev.mouseButton.x, ev.mouseButton.y);

                // кнопка назад
                if (back.isClick(ev.mouseButton.x, ev.mouseButton.y)) {
                    pthread_mutex_lock(&(shs->mutex));
                    shs->act = mainMenu;
                    pthread_mutex_unlock(&(shs->mutex));
                }

                // если клик не по кнопкам дальше не обрабатываем
                if (!login.isClick(ev.mouseButton.x, ev.mouseButton.y) &&
                    !reg.isClick(ev.mouseButton.x, ev.mouseButton.y))
                    continue; // guard clause

                if (!shs->connected || log.isEmpty() || pass.isEmpty()) {
                    errorText.setString(!shs->connected ? "Not connected. Cannot send request!" : "Fill the form!");
                } else {
                    pthread_mutex_lock(&(shs->mutex));
                    login.isClick(ev.mouseButton.x, ev.mouseButton.y) ? shs->act = logIn : shs->act = registering;
                    shs->logInfo = (char *) malloc(50 * sizeof(char));
                    sprintf(shs->logInfo, "%s %s", log.getStr().c_str(), pass.getStr().c_str());
                    pthread_mutex_unlock(&(shs->mutex));
                    printf("%s\n", shs->logInfo);
                }
            }
            else if (ev.type == sf::Event::TextEntered) {
                // получать текст в текстовые поля
                if (log.isActive() && ev.text.unicode < 128)
                    log.updateText(ev.text.unicode);

                if (pass.isActive() && ev.text.unicode < 128)
                    pass.updateText(ev.text.unicode);
            }
        }

        if (drawConnectionState != shs->connected) {
            drawConnectionState = shs->connected;
            connected.setString(drawConnectionState ? "   connected" : "disconnected");
            connected.setFillColor(drawConnectionState ? sf::Color(143, 200, 99) : sf::Color(176, 52, 37));
        }

        // блок обработки результатов входа
        pthread_mutex_lock(&(shs->mutex));
        if (shs->logged == success) {
            errorText.setString("Logged in successfully");
            shs->act = mainMenu;
        }
        else if (shs->logged != notLogged) {
            if (shs->logged == noSuchUser) // нет такого логина
                errorText.setString("No such user!");
            else if (shs->logged == wrongPassword) // не правильный пароль
                errorText.setString("Wrong password!\n");
            else if (shs->logged == alreadyExists)  // уже занят логин
                errorText.setString("This username already taken!");

            shs->logged = notLogged; // сигнал о том, что вход не выполнен
        }
        pthread_mutex_unlock(&(shs->mutex));

        errorText.setPosition(960 - (errorText.getGlobalBounds().width)/2 ,724);

        window.clear();
        window.draw(bgSprite);
        window.draw(connected);
        window.draw(login.draw());
        window.draw(reg.draw());
        window.draw(back.draw());
        window.draw(log.draw());
        window.draw(pass.draw());
        window.draw(errorText);
        window.display();
    }
}

// окно для главного меню
void createMenuApp(sf::RenderWindow &window, SharedState * shs) {
    int DrawnConnectionState = -1;
    sf::Texture bgTexture;
    sf::Sprite bgSprite;
    sf::Font font;
    sf::Event ev{};
    sf::Text connected("", font, 40);
    Button
            startGame("Start game", font, 70, -1, 342, 57),
            login("Log in", font, 70, 0, 206, 57),
            exit("Exit", font, 70, 0, 137, 57);

    bgTexture.loadFromFile("../app_client/src/background_sm.png");
    font.loadFromFile("../app_client/src/gameFont.otf");
    bgSprite.setTexture(bgTexture);

    connected.setPosition(1681, 970);
    startGame.setPosition(155, 442);
    login.setPosition(155, 512);
    exit.setPosition(155, 582);

    if (shs->logged == success)
        startGame.changeCondition(0);

    while (window.isOpen() && shs->act == mainMenu) {
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) {
                shs->act = closeApp;
                continue;
            }
            if (ev.type == sf::Event::MouseMoved) {
                startGame.mouseOnButton(ev.mouseMove.x, ev.mouseMove.y);
                login.mouseOnButton(ev.mouseMove.x, ev.mouseMove.y);
                exit.mouseOnButton(ev.mouseMove.x, ev.mouseMove.y);
                continue;
            }
            if (ev.type == sf::Event::MouseButtonPressed) {
                pthread_mutex_lock(&(shs->mutex));
                if (exit.isClick(ev.mouseButton.x, ev.mouseButton.y)) {
                    shs->act = closeApp;
                }
                else if (login.isClick(ev.mouseButton.x, ev.mouseButton.y)) {
                    shs->act = logHub;
                }
                else if (startGame.isClick(ev.mouseButton.x, ev.mouseButton.y)) {
                    shs->act = gameLobby;
                }
                pthread_mutex_unlock(&(shs->mutex));
            }
        }

        // обновить информацию о подключении, если она не совпадает с отображаемой
        if (DrawnConnectionState != shs->connected) {
            DrawnConnectionState = shs->connected;
            connected.setString(DrawnConnectionState ? "   connected" : "disconnected");
            connected.setFillColor(DrawnConnectionState ? sf::Color(143, 200, 99) : sf::Color(176, 52, 37));
        }

        window.clear();
        window.draw(bgSprite);
        window.draw(connected);
        window.draw(startGame.draw());
        window.draw(login.draw());
        window.draw(exit.draw());
        window.display();
    }
}
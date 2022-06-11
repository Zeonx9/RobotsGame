#include <SFML/Audio.hpp>
#include "interface.h"
#include "intfc_classes.h"

#define W 64
#define H 36

// объявления функций для отрисовки окон
void createMenuApp(sf::RenderWindow &window, SharedState * shs);
void createRegWindow(sf::RenderWindow &window, SharedState * shs);
void createLobby(sf::RenderWindow &window, SharedState * shs);
void beginGame(sf::RenderWindow &window, SharedState * shs);
void endGame(sf::RenderWindow &window, SharedState * shs);

// диспетчер окон приложения
void windowDispatcher(SharedState * shs) {
    // массив указателей на функции для отрисовки соответствующих окон
    void (* windowFuncs[]) (sf::RenderWindow&, SharedState*) = {
            createMenuApp, createRegWindow, createLobby, beginGame, endGame
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
            sprintf(bufIn, "D %d %s", shs->player->ID, shs->player->login);
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
void animatePlayer(Player * p, float t, sf::Sprite &s, float *frame, char **field, float *offsX, float *offsY, int self) {
    t /= 500;
    updatePlayer(p, t, field);

    if (self) {
        if (p->x > 770 && p->x < 2700) *offsX = p->x - 770.f;
        if (p->y > 480 && p->y < 1560) *offsY = p->y - 480.f;
    }

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

    s.setPosition(p->x - *offsX, p->y - *offsY);
    p->dx = 0;
}

// начало самой игры
void beginGame(sf::RenderWindow &window, SharedState * shs){
    // создать сокет для клиента и проверить на удачное создание при ошибках закрывать приложение
    SOCKET client;
    client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client == INVALID_SOCKET){
        printf("!! ERROR CANNOT CREATE SOCKET\n");
        pthread_mutex_lock(&(shs->mutex));
        shs->act = closeApp;
        pthread_mutex_unlock(&(shs->mutex));
        return;
    }
    char portBuf[40] = {};
    // получить порт для подключения
    int r = recv(shs->sock, portBuf, 40, 0);
    if (!r || r == SOCKET_ERROR) {
        printf("error in receiving port\n");
        pthread_mutex_lock(&(shs->mutex));
        shs->act = closeApp;
        pthread_mutex_unlock(&(shs->mutex));
        return;
    }
    u_short port;
    sscanf(portBuf, "%lu %s", &port, shs->gameResult->opponentLogin); // извлечь число из строки
    // описание сервера для подключения
    SOCKADDR_IN saddr; int size = sizeof(saddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(port); // такой же порт как на сервер
    saddr.sin_addr.S_un.S_addr = inet_addr(IP); // Zeon's IP адрес
    printf("udp socket has been configured (port = %d)\nopponetLogin: %s\n",
           port, shs->gameResult->opponentLogin);

    char buffer[101], no[] = "NO";
    int err = 0;
    u_long mode = 1;  // сделать сокет не блокирующим
    ioctlsocket(client, FIONBIO, &mode);

    sf::Texture bgTexture, playerTexture, bulletTexture, blockTexture;
    sf::Sprite bgSprite, s1, s2, bulletS, block, hp;
    sf::Font font;
    sf::Event ev{};
    sf::Text name("", font, 40);

    font.loadFromFile("../app_client/src/gameFont.otf");
    bgTexture.loadFromFile("../app_client/src/background.png");
    playerTexture.loadFromFile("../app_client/src/robotgamesprites.png");
    bulletTexture.loadFromFile("../app_client/src/bullet.png");
    blockTexture.loadFromFile("../app_client/src/groupBlocks.png");

    bgSprite.setTexture(bgTexture);
    bulletS.setTexture(bulletTexture);
    block.setTexture(blockTexture);
    hp.setTexture(bulletTexture);
    s1.setTexture(playerTexture);
    s2.setTexture(playerTexture);
    s2.setColor(sf::Color(255, 180, 180));
    name.setFillColor(sf::Color::White);

    // загрузка карты из файла
    char **field = (char **) malloc(H * sizeof(char *));
    FILE * file_map = fopen("../app_client/src/test_field.txt", "r");
    for (int i = 0; i < H; ++i) {
        field[i] = (char *) malloc(W + 2 * sizeof(char));
        fgets(field[i], W + 2, file_map);
    }
    fclose(file_map);

    Player player1, player2;
    Bullet bullets[MAX_BULLETS] = {};
    float animation1, animation2, offsX, offsY;
    initPlayer(&player1); initPlayer(&player2);
    sf::Clock clock1, clock2, bulletTimer, clockBullets, overall;

    while(window.isOpen() && shs->act == play) {
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) {
                sendto(client, no, 3, 0, (SOCKADDR *) &saddr, sizeof(saddr));
                printf("no sent\n");
                pthread_mutex_lock(&(shs->mutex));
                shs->act = closeApp;
                pthread_mutex_unlock(&(shs->mutex));
                printf("exit\n");
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
            sendto(client, no, 3, 0, (SOCKADDR *) &saddr, sizeof(saddr));
            printf("Q no sent\n");
            pthread_mutex_lock(&(shs->mutex));
            shs->act = closeApp;
            pthread_mutex_unlock(&(shs->mutex));
            printf("exit\n");
            break;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            walk(&player1, Right);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            walk(&player1, Left);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            leap(&player1);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) &&
            bulletTimer.getElapsedTime().asMilliseconds() > 600) {
            player1.shoot = 1;
            bulletTimer.restart();
        }

        // отправить информацию о себе, затем анимировать персонажа
        sendto(client, (const char *) &player1, sizeof(Player), 0, (SOCKADDR *) &saddr, sizeof(saddr));
        animatePlayer(&player1, (float) clock1.restart().asMicroseconds(),
                      s1, &animation1, field, &offsX, &offsY, 1);
        initBullet(&player1, bullets);

        // получить информацию о сопернике (в буфер)
        int len = recvfrom(client, buffer, 100, 0, (SOCKADDR *) &saddr, &size);
        if (strcmp(buffer, "NO") == 0) {
            printf("received signal of disconnection\n");
            pthread_mutex_lock(&(shs->mutex));
            if (shs->act > 0) {
                shs->act = mainMenu;
            }
            pthread_mutex_unlock(&(shs->mutex));
            printf("to main Menu\n");
        }
        // если нет ошибок передачи сохраняем данные в объекте клиента
        if (len == sizeof(player2)) {
            memcpy(&player2, buffer, len);
            err = 0;
        } else {
            printf("error occurred in receiving data (%d)\n", len);
            err++;
        }
        animatePlayer(&player2, (float) clock2.restart().asMicroseconds(),
                      s2, &animation2, field, &offsX, &offsY, 0);
        initBullet(&player2, bullets);

        window.clear();
        bgSprite.setPosition(- 0.5 * offsX, - 0.5 * offsY);
        window.draw(bgSprite);

        // отрисовать карту
        for (int i = 0; i < H; ++i)
            for (int j = 0; j < W; ++j) {
                if (field[i][j] == '0')
                    continue;
                block.setTextureRect(sf::IntRect((field[i][j] - '1') * TILE, 0, TILE, TILE));
                block.setPosition(j * TILE - offsX, i * TILE - offsY);
                window.draw(block);
            }

        // отрисовать игроков
        window.draw(s2);
        window.draw(s1);

        // отрисовать пули
        float time = clockBullets.restart().asMicroseconds();
        for (Bullet *b = bullets; b < bullets + MAX_BULLETS; ++b) {
            if (!b->dir)
                continue;

            b->x += b->dir * time * 0.002f;
            if (field[(int)b->y / TILE][(int)b->x / TILE] > '0') {
                b->dir = 0;
                continue;
            }
            if (b->x > player1.x && b->x < player1.x + WIDTH && b->y > player1.y && b->y < player1.y + HEIGHT){
                player1.health--;
                printf("first player got damaged\n");
                b->dir = 0;
            }
            if (b->x > player2.x && b->x < player2.x + WIDTH && b->y > player2.y && b->y < player2.y + HEIGHT){
                printf("second player got damaged\n");
                b->dir = 0;
            }

            bulletS.setPosition(b->x - offsX, b->y - offsY);
            window.draw(bulletS);
        }

        // отрисовать полоски жизней
        for (int i = 0, x1 = 5, x2 = 1715, y = 5; i < 5; ++i, x1 += 40, x2 += 40) {
            if (i + 1 > player1.health)
                bulletS.setColor(sf::Color(50, 50, 50));
            bulletS.setPosition(x1, y);
            window.draw(bulletS);
            bulletS.setColor(sf::Color::White);
            if (i + 1 > player2.health)
                bulletS.setColor(sf::Color(50, 50, 50));
            bulletS.setPosition(x2, y);
            window.draw(bulletS);
            bulletS.setColor(sf::Color::White);
        }
        // имена игроков
        name.setString(shs->player->login);
        name.setPosition(220, 5);
        window.draw(name);
        name.setString(shs->gameResult->opponentLogin);
        name.setPosition(1615, 5);
        window.draw(name);

        window.display();

        if (err > 300) { // проверка, что второй клиент отключился аварийно
            pthread_mutex_lock(&(shs->mutex));
            if (shs->act > 0)
                shs->act = mainMenu;
            pthread_mutex_unlock(&(shs->mutex));
        }

        // проверка на окончание игры
        if (player1.health < 1 || player2.health < 1) {
            shs->gameResult->winner = player1.health > 0;
            shs->gameResult->hits = 5 - player2.health;
            shs->gameResult->time = overall.getElapsedTime().asSeconds();

            sendto(client, no, 3, 0, (SOCKADDR *) &saddr, sizeof(saddr));
            printf("game is over, NO is sent");

            pthread_mutex_lock(&(shs->mutex));
            shs->act = gameOver;
            pthread_mutex_unlock(&(shs->mutex));
        }
    }

    // очистить память занятую полем
    for (int i = 0; i < H; ++i)
        free(field[i]);
    free(field);

    printf("exiting playing room\n");
    closesocket(client);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void endGame(sf::RenderWindow &window, SharedState * shs){
    int drawConnectionState = -1;
    char timeStr[30], newRating[60];
    sprintf(timeStr, "%2d min  %2d sec", (int)shs->gameResult->time / 60, (int)shs->gameResult->time % 60);
    sprintf(newRating, "%d\t%d\t%d",
            shs->player->highScore + shs->gameResult->hits,
            shs->player->wins + shs->gameResult->winner, shs->player->gamesPlayed + 1);
    sf::Texture bgTexture;
    sf::Sprite bgSprite;
    sf::Font font;
    sf::Event ev{};
    sf::Text
            winner(shs->gameResult->winner ? shs->player->login : shs->gameResult->opponentLogin, font, 40),
            gameTime(timeStr, font, 40),
            rating(newRating, font, 40);
    Button
            back ("menu", font, 70, 0, 137, 57);

    bgTexture.loadFromFile("../app_client/src/background_eg.png");
    font.loadFromFile("../app_client/src/gameFont.otf");
    bgSprite.setTexture(bgTexture);

    winner.setPosition(855, 550);
    winner.setFillColor(sf::Color(143, 200, 99));
    gameTime.setPosition(777, 610);
    gameTime.setFillColor(sf::Color(143, 200, 99));
    rating.setPosition(973, 670);
    rating.setFillColor(sf::Color(143, 200, 99));
    back.setPosition(45, 944);

    while (window.isOpen() && shs->act == gameOver) {
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
                    shs->act = mainMenu;
                    pthread_mutex_unlock(&(shs->mutex));
                }
            }
        }

        window.clear();
        window.draw(bgSprite);
        window.draw(winner);
        window.draw(gameTime);
        window.draw(rating);
        window.draw(back.draw());
        window.display();
    }
}

// окно ожидания второго игрока
void createLobby(sf::RenderWindow &window, SharedState * shs) {
    int drawConnectionState = -1;
    char myInfo[100];
    // запросить рейтинг
    pthread_mutex_lock(&(shs->mutex));
    shs->act = getRating;
    pthread_mutex_unlock(&(shs->mutex));
    printf("getting rating %d\n", shs->act);

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

    while (shs->act == getRating); // ожидание ответа от сервера
    if (!shs->connected)
        rating.setString("Cannot get current rating!\n");
    else {
        rating.setString(shs->rating);
        printf("rating set\n");
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
                    printf("canceling the game\n");
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
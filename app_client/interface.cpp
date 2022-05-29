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

        // логин на сервер
        if (shs->act == logIn || shs->act == registering) {
            pthread_mutex_lock(&(shs->mutex));
            sprintf(bufIn, "%c %s", (shs->act == logIn ? 'A' : 'B'), shs->logInfo);
            pthread_mutex_unlock(&(shs->mutex));

            int res = serverSession(shs->sock, bufIn, bufOut);
            if (res) { // соединение с сервером разорвано
                shs->connected = 0;
                shs->logged = notLogged;
                continue;
            }

            pthread_mutex_lock(&(shs->mutex));
            printf("<<< %s\n", bufOut);

            if (bufOut[0] == 'N')
                shs->logged = noSuchUser;
            else if (bufOut[0] == 'W')
                shs->logged = wrongPassword;
            else if (bufOut[0] == 'E')
                shs->logged = alreadyExists;
            else
                shs->logged = success;

            shs->act = logHub;
            pthread_mutex_unlock(&(shs->mutex));
        }
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

// окно для входа и регистрации
void createRegWindow(sf::RenderWindow &window, SharedState * shs) {
    int drawConnectionState = -1;
    sf::Texture bgTexture;
    sf::Sprite bgSprite;
    sf::Font font;
    sf::Event ev{};
    sf::RectangleShape
            line1(sf::Vector2f(557, 2)),
            line2(sf::Vector2f(557, 2));
    sf::Text
            header("login or registration", font, 100),
            connected("", font, 40), // показывает, есть ли соединение с сервером
            version("version 1.0", font, 40),
            text1("enter your username", font, 30),
            text2("enter your password", font, 30),
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

    header.setFillColor(sf::Color(178, 189, 231));
    header.setPosition(447, 150);
    connected.setPosition(1681, 970);
    version.setFillColor(sf::Color(255, 255, 255));
    version.setPosition(1681, 1013);
    text1.setFillColor(sf::Color(122, 122, 122));
    text1.setPosition(818, 427);
    text2.setFillColor(sf::Color(122, 122, 122));
    text2.setPosition(821, 522);
    errorText.setFillColor(sf::Color(176, 52, 37));
    errorText.setPosition(630, 724);
    login.setPosition(682, 607);
    reg.setPosition(966, 607);
    back.setPosition(45, 944);
    line1.setPosition(682, 417);
    line2.setPosition(682, 512);
    log.setPosition(682, 361);
    pass.setPosition(682, 456);

    while (window.isOpen() && (shs->act == logHub || shs->act > play)) {
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) {
                shs->act = closeApp;
                continue;
            }
            if (ev.type == sf::Event::MouseMoved) {
                login.mouseOnButton(ev.mouseMove.x, ev.mouseMove.y);
                reg.mouseOnButton(ev.mouseMove.x, ev.mouseMove.y);
                back.mouseOnButton(ev.mouseMove.x, ev.mouseMove.y);
                continue;
            }
            if (ev.type == sf::Event::MouseButtonPressed) {
                // обработать текстовые клики по текстовым полям
                log.changeCondition(0);
                pass.changeCondition(0);
                log.isClick(ev.mouseButton.x, ev.mouseButton.y);
                pass.isClick(ev.mouseButton.x, ev.mouseButton.y);

                // кнопка назад
                if (back.isClick(ev.mouseButton.x, ev.mouseButton.y)) {
                    shs->act = mainMenu;
                    continue;
                }

                // если клик не по кнопкам дальше не обрабатываем
                if (!login.isClick(ev.mouseButton.x, ev.mouseButton.y) &&
                    !reg.isClick(ev.mouseButton.x, ev.mouseButton.y))
                    continue;

                if (!shs->connected || !log.draw().getString().getSize() || !pass.draw().getString().getSize()) {
                    errorText.setString(!shs->connected ? "Not connected. Cannot send request!" : "Fill the form!");
                    continue;
                }

                pthread_mutex_lock(&(shs->mutex));
                login.isClick(ev.mouseButton.x, ev.mouseButton.y) ? shs->act = logIn : shs->act = registering;
                sprintf(shs->logInfo, "%s %s",
                        log.draw().getString().toAnsiString().c_str(),
                        pass.draw().getString().toAnsiString().c_str());
                pthread_mutex_unlock(&(shs->mutex));
            }
            if (ev.type == sf::Event::TextEntered){
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

        if (shs->logged != notLogged && shs->logged != success) {
            if (shs->logged == noSuchUser) // нет такого логина
                errorText.setString("No such user!");
            else if (shs->logged == wrongPassword) // не правильный пароль
                errorText.setString("Wrong password!\n");
            else if (shs->logged == alreadyExists)  // уже занят логин
                errorText.setString("This username already taken!");

            shs->logged = notLogged; // сигнал о том, что вход не выполнен
        }
        else if (shs->logged == success) {
            errorText.setString("Logged in successfully");
            shs->act = mainMenu;
        }

        window.clear();
        window.draw(bgSprite);
        window.draw(header);
        window.draw(connected);
        window.draw(version);
        window.draw(login.draw());
        window.draw(reg.draw());
        window.draw(back.draw());
        window.draw(text1);
        window.draw(text2);
        window.draw(line1);
        window.draw(line2);
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
    sf::Text 
            header("Robots Game", font, 150),
            connected("", font, 40), 
            version("version 1.0", font, 40);
    Button 
            startGame ("Start game", font, 70, -1, 342, 57),
            login ("Log in", font, 70, 0, 206, 57),
            exit ("Exit", font, 70, 0, 137, 57);

    bgTexture.loadFromFile("../app_client/src/background_sm.png");
    font.loadFromFile("../app_client/src/gameFont.otf");
    bgSprite.setTexture(bgTexture);

    header.setFillColor(sf::Color(178, 189, 231));
    header.setPosition(150, 150);
    connected.setPosition(1681, 970);
    version.setFillColor(sf::Color(255, 255, 255));
    version.setPosition(1681, 1013);
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
                if (exit.isClick(ev.mouseButton.x, ev.mouseButton.y)){
                    shs->act = closeApp;
                    continue;
                }
                if (login.isClick(ev.mouseButton.x, ev.mouseButton.y)) {
                    shs->act = logHub;
                    continue;
                }
                if (startGame.isClick(ev.mouseButton.x, ev.mouseButton.y))
                    printf("Not Implemented 'START GAME'\n");
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
        window.draw(header);
        window.draw(connected);
        window.draw(version);
        window.draw(startGame.draw());
        window.draw(login.draw());
        window.draw(exit.draw());
        window.display();
    }
}
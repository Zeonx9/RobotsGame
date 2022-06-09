                #include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
extern "C" {
    #include "../app_client/game.h"
}

using namespace sf;

void animatePlayer(Player * p, float t, sf::Sprite &s) {
    t /= 500;
    updatePlayer(p, t);

    p->curFrame += .0075f * t; // анимация
    if (p->curFrame >= 12)
        p->curFrame = 0;
    if (p->dx > 0)  s.setTextureRect(sf::IntRect(WIDTH * (int)p->curFrame, 0, WIDTH, HEIGHT));
    if (p->dx < 0)  s.setTextureRect(sf::IntRect(WIDTH * (int)(p->curFrame + 1), 0, -WIDTH, HEIGHT));
    if (p->dx == 0) {
        if (p->dir > 0) s.setTextureRect(sf::IntRect(WIDTH * 13, 0, WIDTH, HEIGHT));
        else s.setTextureRect(sf::IntRect(WIDTH * 14, 0, -WIDTH, HEIGHT));
    }

    s.setPosition(p->x, p->y);
    p->dx = 0;
}

int startWindow() {
    RenderWindow window(VideoMode(1920, 1080), "Tupa Bounce Ball");
    window.setFramerateLimit(60);

    Clock clock;
    Event event{};
    Texture texture;
    Sprite sprite1, sprite2;
    texture.loadFromFile("../app_client/src/robotgamesprites.png");
    sprite1.setTexture(texture);
    sprite2.setTexture(texture);
    CircleShape *bullets = (CircleShape*) calloc(10, sizeof(CircleShape));
    Player player1, player2;
    initPlayer(&player1);
    initPlayer(&player2);


    while (window.isOpen()) {
        while (window.pollEvent(event))
            if (event.type == Event::Closed)
                window.close();

        if (Keyboard::isKeyPressed(Keyboard::Q))
            window.close(); // закрытие окна

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            walk(&player1, Right);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            walk(&player1, Left);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            leap(&player1);

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            walk(&player2, Right);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            walk(&player2, Left);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            leap(&player2);

        float time = (float) clock.restart().asMicroseconds();
        animatePlayer(&player1, time, sprite1);
        animatePlayer(&player2, time, sprite2);



        window.clear(Color(30, 30, 60));
        window.draw(sprite1);
        window.draw(sprite2);
        window.display();
    }
    return 0;
}

int main() {
    startWindow();
}
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
    Sprite sprite1;
    texture.loadFromFile("../app_client/src/robotgamesprites.png");
    sprite1.setTexture(texture);

    Player player;
    initPlayer(&player);


    while (window.isOpen()) {
        while (window.pollEvent(event))
            if (event.type == Event::Closed)
                window.close();

        if (Keyboard::isKeyPressed(Keyboard::Q))
            window.close(); // закрытие окна

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
            walk(&player, right);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
            walk(&player, left);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
            leap(&player);

        animatePlayer(&player, (float) clock.restart().asMicroseconds(), sprite1);

        window.clear(Color(30, 30, 60));
        window.draw(sprite1);
        window.display();
    }
    return 0;
}

int main() {
    startWindow();
}
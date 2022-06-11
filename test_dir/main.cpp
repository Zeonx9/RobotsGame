                #include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
extern "C" {
    #include "../app_client/game.h"
}

#define W 64
#define H 36

using namespace sf;

void animatePlayer(Player * p, float t, sf::Sprite &s, float *frame, char **field, float *offsX, float *offsY) {
    t /= 500;
    updatePlayer(p, t, field);

    if (p->x > 770 && p->x < 2700) *offsX = p->x - 770.f;
    if (p->y > 480 && p->y < 1560) *offsY = p->y - 480.f;

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

int startWindow() {
    RenderWindow window(VideoMode(1920, 1080), "tut est' karta");
    window.setFramerateLimit(60);

    Clock clock, clock3;
    Event event{};
    Texture t, bg, blockT, bulletTexture;
    Sprite sbg, s1, block, bulletS;
    bg.loadFromFile("../app_client/src/background_2.png");
    t.loadFromFile("../app_client/src/robotgamesprites.png");
    blockT.loadFromFile("../app_client/src/groupBlocks.png");
    bulletTexture.loadFromFile("../app_client/src/bullet.png");

    sbg.setTexture(bg);
    s1.setTexture(t);
    block.setTexture(blockT);
    bulletS.setTexture(bulletTexture);
    s1.setColor(Color(255, 180, 180));

    Player player1;
    float animation1, offsX = 0, offsY = 0;
    Bullet bullets[MAX_BULLETS] = {};
    initPlayer(&player1);

    char **field = (char **) malloc(H * sizeof(char *));
    FILE * file_map = fopen("../app_client/src/test_field.txt", "r");
    for (int i = 0; i < H; ++i) {
        field[i] = (char *) malloc(W + 2 * sizeof(char));
        fgets(field[i], W + 2, file_map);
    }
    fclose(file_map);


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
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            leap(&player1);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && clock3.getElapsedTime().asMilliseconds() > 600) {
            player1.shoot = 1;
            clock3.restart();
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            player1.health--;

        float time = (float) clock.restart().asMicroseconds();
        animatePlayer(&player1, time, s1, &animation1, field, &offsX, &offsY);
        initBullet(&player1, bullets);

        window.clear();
        sbg.setPosition(- 0.5f * offsX, - 0.5f * offsY);
        window.draw(sbg);

        for (int i = 0; i < H; ++i)
            for (int j = 0; j < W; ++j) {
                if (field[i][j] == '0')
                    continue;
                block.setTextureRect(IntRect((field[i][j] - '1') * TILE, 0, TILE, TILE));
                block.setPosition(j * TILE - offsX, i * TILE - offsY);
                window.draw(block);
            }

        window.draw(s1);

        for (Bullet *bullet = bullets; bullet < bullets + MAX_BULLETS; ++bullet) {
            if (!bullet->dir)
                continue;

            bullet->x += bullet->dir * time * 0.002f;
            if (field[(int)bullet->y / TILE][(int)bullet->x / TILE] > '0') {
                bullet->dir = 0;
                continue;
            }

            bulletS.setPosition(bullet->x - offsX, bullet->y - offsY);
            window.draw(bulletS);
        }


        for (int i = 0, x = 5, y = 5; i < 5; ++i, x += 40) {
            if (i + 1 > player1.health)
                bulletS.setColor(sf::Color(50, 50, 50));
            bulletS.setPosition(x, y);
            window.draw(bulletS);
        } bulletS.setColor(sf::Color::White);

        for (int i = 0, x = 1715, y = 5; i < 5; ++i, x += 40) {
            if (i + 1 > player1.health)
                bulletS.setColor(sf::Color(50, 50, 50));
            bulletS.setPosition(x, y);
            window.draw(bulletS);
        } bulletS.setColor(sf::Color::White);
        window.display();
    }

    for (int i = 0; i < H; ++i)
        free(field[i]);
    free(field);
    return 0;
}

int main() {
    startWindow();
}
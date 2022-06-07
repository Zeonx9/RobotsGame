#include "field.h"
void ifKeyPressed(int *x, int *y, unsigned int symbol){
    if (symbol == 72) {if (*x <= 1860) *x += 60;}
    else if (symbol == 74) {if (*y <= 1020) *y += 60;}
    else if (symbol == 71) { if (*x != 0) *x -= 60;}
    else if (symbol == 73) { if (*y != 0) *y -= 60;}
}

void createField(sf::RenderWindow &window) {
    sf::Texture bgTexture, blockTexture;
    sf::Sprite bgSprite, blockSprite;
    sf::Event ev{};
    GAMEFIELD game;

    game.borderX = game.borderY = 0;
    bgTexture.loadFromFile("../app_client/src/background_2.png");
    bgSprite.setTexture(bgTexture);
    blockTexture.loadFromFile("../app_client/src/groupBlocks.png");
    blockSprite.setTexture(blockTexture);

    FILE * f = fopen("../app_client/src/test_field", "r");
    char line[64] = {};
    int count = 0;
    while (fscanf(f, "%s", &line) != EOF) {
        for (int i = 0; i < 64; i += 1)
            game.field[count][i] = line[i];
        count += 1;
    }

    while(window.isOpen()) {
        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed) {
                return;
            }
            if (ev.type == sf::Event::KeyPressed){
                ifKeyPressed(&game.borderX, &game.borderY, ev.text.unicode);
            }
        }
        window.clear();
        bgSprite.setTextureRect(sf::IntRect(game.borderX, game.borderY, 1920, 1080));
        window.draw(bgSprite);
        for (int i = 0; i < 36; i += 1)
            for (int j = 0; j < 64; j += 1) {
                if (game.field[i][j] != 48) {
                    blockSprite.setTextureRect(sf::IntRect((game.field[i][j] - 49) * 60, 0, 60, 60));
                    blockSprite.setPosition((float) j * 60 - (float)game.borderX, (float) i * 60 - (float)game.borderY);
                    window.draw(blockSprite);
                }
            }
        window.display();
    }
}


void startGame() {
    // создать окно
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Client", sf::Style::Fullscreen);
    // загрузить иконку
    sf::Image icon;
    icon.loadFromFile("../app_client/src/rgame_icon64.png");
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    window.setFramerateLimit(40);

    createField(window);
}
#ifndef ROBOTSGAME_INTFC_CLASSES_H
#define ROBOTSGAME_INTFC_CLASSES_H
#include <SFML/Graphics.hpp>

class Button {
    public:
        Button(const char *text, const sf::Font &font, int textSize, int condition, int buttonWidth, int buttonHeight);
        void changeCondition(int);
        void mouseOnButton(int, int);
        int isClick(int, int);
        void setPosition(int, int);
        sf::Text draw();
    private:
        int width, height, status, size;
        sf::Text button;
};

#endif //ROBOTSGAME_INTFC_CLASSES_H

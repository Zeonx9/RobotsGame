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

class TextBox {
    public:
        TextBox(const sf::Font &font, int textSize, int boxWidth, int boxHeight);
        void isClick(int, int);
        void updateText(unsigned int);
        void setPosition(int, int);
        int isActive() const;
        void changeCondition(int);
        sf::Text draw();
        const char* getChar();
        bool isEmpty();
    private:
        sf::String input;
        sf::Text text;
        int condition, size, width, height;

};

#endif //ROBOTSGAME_INTFC_CLASSES_H

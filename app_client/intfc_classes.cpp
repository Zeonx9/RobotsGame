#include "intfc_classes.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


Button::Button (const char *text, const sf::Font &font, int textSize, int condition, int buttonWidth, int buttonHeight) {
    button.setString(text);
    button.setFont(font);
    button.setCharacterSize(textSize);
    status = condition;
    size = textSize;
    width = buttonWidth;
    height = buttonHeight;
    changeCondition(condition);
}
void Button::changeCondition(int condition) {
    status = condition;
    if (status == 0) {
        button.setFillColor(sf::Color(255, 255, 255));
        button.setCharacterSize(size);
    }
    else if (status == -1) {
        button.setFillColor(sf::Color(122, 122, 122));
        button.setCharacterSize(size);
    }
    else if (status == 1) {
        button.setFillColor(sf::Color(255, 255, 255));
        button.setCharacterSize(size + 10);
    }
    else if (status == 2) {
        button.setFillColor(sf::Color(178, 189, 231));
        button.setCharacterSize(size + 10);
    }
}
void Button::mouseOnButton(int mouseX, int mouseY) {
    // TODO убрать баг с координатами кнопки
    int x = (int)button.getPosition().x, y = (int)button.getPosition().y;
    if (status != -1) {
        if (x <= mouseX && mouseX <= x + width && y <= mouseY && mouseY <= y + height) changeCondition(1);
        else
            changeCondition(0);
    }
}
int Button::isClick(int mouseX, int mouseY) {
    int x = (int)button.getPosition().x, y = (int)button.getPosition().y;
    if (status != -1) {
        if (x <= mouseX && mouseX <= x + width && y <= mouseY && mouseY <= y + height) {
            changeCondition(2);
            return true;
        }
        else {
            changeCondition(0);
            return false;
        }
    }
    else
        return false;
}
void Button::setPosition(int x, int y) {
    button.setPosition((float)x, (float)y);
}
sf::Text Button::draw() {
    return button;
}
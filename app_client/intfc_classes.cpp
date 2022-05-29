#include "intfc_classes.h"

//конструктор класса кнопки
Button::Button (const char *text, const sf::Font &font, int textSize, int condition, int buttonWidth, int buttonHeight) {
    button.setString(text);
    button.setFont(font);
    button.setCharacterSize(textSize);
    status = condition;
    size = textSize;
    changeCondition(condition);
    width = buttonWidth;
    height = buttonHeight;
}

// изменение состояние кнопки (ее размера и цвета) в зависимости от действий пользователя
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

// проверка, наведена ли мышка на кнопку
void Button::mouseOnButton(int mouseX, int mouseY) {
    int x = (int)button.getPosition().x, y = (int)button.getPosition().y;
    if (status != -1) {
        if (x <= mouseX && mouseX <= x + width && y + 30 <= mouseY && mouseY <= y + 30 + height) changeCondition(1);
        else
            changeCondition(0);
    }
}

// проверка, нажал ли пользователь на кнопку
int Button::isClick(int mouseX, int mouseY) {
    int x = (int)button.getPosition().x, y = (int)button.getPosition().y;
    if (status != -1) {
        if (x <= mouseX && mouseX <= x + width && y + 30 <= mouseY && mouseY <= y + 30 + height) {
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

// установка позиции кнопки на экране
void Button::setPosition(int x, int y) {
    button.setPosition((float)x, (float)y);
}

// возвращает текст кнопки для отрисовки
sf::Text Button::draw() {
    return button;
}

// конструктор класса текстового поля
TextBox::TextBox(const sf::Font &font, int textSize, int boxWidth, int boxHeight) {
    condition = 0;
    size = textSize;
    text.setCharacterSize(size);
    text.setFont(font);
    text.setFillColor(sf::Color(255, 255, 255));
    width = boxWidth;
    height = boxHeight;
}

// проверка, активно ли поле в определенный момент
int TextBox::isActive() const {
    if (condition == 1) return true;
    return false;
}

// проверка, нажал ли пользователь на текстовое поле
void TextBox::isClick(int mouseX, int mouseY) {
    int x = (int)text.getPosition().x, y = (int)text.getPosition().y;
    if (x <= mouseX && mouseX <= x + width && y + 15 <= mouseY && mouseY <= y + 15 + height) {
        condition = 1;
    }
}

// установка позиции текстового поля на экран
void TextBox::setPosition(int x, int y) {
    text.setPosition((float)x, (float)y);
}

// при вводе текста обновляется строка
void TextBox::updateText(unsigned int symbol) {
    if (symbol == 8){
        if (input.getSize() == 0)
            return;
        input.erase(input.getSize() - 1, 1);
        text.setString(input);
        return;
    }
    if (input.getSize() == 20)
        return;
    input += symbol;
    text.setString(input);
}

// возвращает введенный текст для отрисовки
sf::Text TextBox::draw() {
    return text;
}

// изменения состояния текстового поля
void TextBox::changeCondition(int status) {
    condition = status;
}

const char* TextBox::getChar() {
    return static_cast<std::string>(input).c_str();
}

bool TextBox::isEmpty() {
    return !input.getSize();
}

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

using namespace sf;

void music() {
    Music music;
    music.openFromFile("../app_client/src/menu_music.ogg");
    music.setLoop(true);
    music.play();
}

int startWindow() {
    RenderWindow window(VideoMode(800, 600), "Tupa Bounce Ball");

    CircleShape circle;
    circle.setRadius(50); circle.setPosition(0, 500);
    circle.setFillColor(Color::Yellow);
    window.setFramerateLimit(60);

    Clock clk;
    float speed = 300;
    bool onGround = true;
    float dy;
    float a = 10;

    while (window.isOpen()) {
        Event event{};
        while (window.pollEvent(event))
            if (event.type == Event::Closed)
                window.close();

        window.clear(Color::Black);

        float t = (float) clk.restart().asMilliseconds();
        if (Keyboard::isKeyPressed(Keyboard::A) && circle.getPosition().x > 0)
            circle.move(-speed * t / 1000, 0);
        if (Keyboard::isKeyPressed(Keyboard::D) && circle.getPosition().x < 700)
            circle.move(speed * t / 1000, 0);
        if (Keyboard::isKeyPressed(Keyboard::W) && onGround ) {
            dy = -50;
            onGround = false;
        }

        if (!onGround) {
            circle.move(0, dy);
            dy += 2 * a * t / 100;
            if (circle.getPosition().y > 500) {
                circle.setPosition(circle.getPosition().x, 500);
                onGround = true;
            }
        }

        window.draw(circle);
        window.display();
    }
    return 0;
}

int main() {
    startWindow();
}
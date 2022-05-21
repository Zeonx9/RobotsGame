#ifndef ROBOTSGAME_INTERFACE_H
#define ROBOTSGAME_INTERFACE_H

#include "client.h"
#include <SFML/Graphics.hpp>

void createConnectingApp(ClientState *state, sf::RenderWindow &window);

void createMenuApp(sf::RenderWindow &window);

#endif //ROBOTSGAME_INTERFACE_H

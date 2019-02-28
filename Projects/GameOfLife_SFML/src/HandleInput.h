#ifndef HANDLEINPUT_H
#define HANDLEINPUT_H
#include "Window.h"
#include "GUI.h"

#include <SFML/Graphics.hpp>

void handleKeyPressed(sf::Event::KeyEvent &keyEvent, Window &window);
void handleMouseWheelScroll(sf::Event::MouseWheelScrollEvent &mwScroll, Window &window);
void handleMouseButtonPressed(sf::Event::MouseButtonEvent mbE, Window &window, GUIs &guis, Cells &cells);

#endif
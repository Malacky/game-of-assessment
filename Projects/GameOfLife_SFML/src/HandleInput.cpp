#include "HandleInput.h"
#include "Window.h"
#include "GUI.h"
#include "Map.h"

#include <SFML/Graphics.hpp>

void handleKeyPressed(sf::Event::KeyEvent &keyEvent, Window &window) {
	constexpr double originOffsetChange = 100;

	switch (keyEvent.code) {
	case sf::Keyboard::Up:
		window.moveView(sf::Vector2f(0, -originOffsetChange));
		break;

	case sf::Keyboard::Right:
		window.moveView(sf::Vector2f(originOffsetChange, 0));
		break;

	case sf::Keyboard::Down:
		window.moveView(sf::Vector2f(0, originOffsetChange));
		break;

	case sf::Keyboard::Left:
		window.moveView(sf::Vector2f(-originOffsetChange, 0));
		break;
	}
}

void handleMouseWheelScroll(sf::Event::MouseWheelScrollEvent &mwScroll, Window &window) {
	window.zoom(sf::Vector2f(mwScroll.x, mwScroll.y), mwScroll.delta > 0);
}

void handleMouseButtonPressed(sf::Event::MouseButtonEvent mbE, Window &window, GUIs &guis, Cells &cells) {
	bool guiPressed = false;

	for (GUI &gui : guis) {
		if (withinBounds(sf::Vector2f(mbE.x, mbE.y), sf::FloatRect(static_cast<sf::Vector2f>(window.getSFMLWindow().mapCoordsToPixel(gui.getPosition())), static_cast<sf::Vector2f>(window.getSFMLWindow().mapCoordsToPixel(gui.getSize()))))) {
			gui.onClick();
			guiPressed = true;
		}
	}
}
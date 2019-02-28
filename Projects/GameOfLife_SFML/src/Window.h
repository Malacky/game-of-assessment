#ifndef WINDOW_H
#define WINDOW_H

#include <SFML/Graphics.hpp>

class Window {
public:
	Window(sf::VideoMode vm, std::string str) : window{ vm, str }, view{ window.getDefaultView() } {
		window.setView(view);
	}

	sf::RenderWindow &getSFMLWindow() {
		return window;
	}

	sf::View &getView() {
		return view;
	}

	void setView(const sf::FloatRect rect) {
		view.setViewport(rect);
		window.setView(view);
	}

	void moveView(sf::Vector2f offset) {
		view.move(offset);
		window.setView(view);
	}

	void zoom(sf::Vector2f mousePos, bool zoomIn);

private:
	sf::RenderWindow window;
	sf::View view;
	float zoomInSum{1};
};

#endif
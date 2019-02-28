#include <SFML/Graphics.hpp>

#include "Window.h"

void Window::zoom(sf::Vector2f mousePos, bool zoomIn) { //Second arguments is positive if zooming in, negative if zooming out.
	constexpr float changeCenterToMouseCoords = 2; //Affects affinity of the center of the window towards the mouse.
	constexpr float zoomInMovementChange = 0.1f;

	if (mousePos.x < 0 || mousePos.y < 0)
		return;

	auto adjustedCenterCoord = sf::Vector2f(window.getSize().x / 2.f, window.getSize().y / 2.f); //Retrieve the coordinate of the middle of the window.

	if (zoomIn) { //For intuitive zooming.
		view.move((-(adjustedCenterCoord.x - mousePos.x) / changeCenterToMouseCoords / zoomInSum), -(adjustedCenterCoord.y - mousePos.y) / changeCenterToMouseCoords / zoomInSum);
		view.zoom(0.8f);

		zoomInSum += zoomInMovementChange; //The more you zoom in, the less the movement will change. Values are arbitrary.
	}
	else {
		view.move(((adjustedCenterCoord.x - mousePos.x) / changeCenterToMouseCoords / zoomInSum), (adjustedCenterCoord.y - mousePos.y) / changeCenterToMouseCoords / zoomInSum);
		view.zoom(1.2f);

		if (zoomInSum > zoomInMovementChange * 2)
			zoomInSum -= zoomInMovementChange;
	}

	window.setView(view);
}
#include "Cell.h"
#include "GUI.h"
#include "Window.h"

#include <SFML/Graphics.hpp>

#include <vector>
#include <chrono>

const sf::Vector2f textureSize(32, 32);

void Button::render(Window &window, sf::Texture texture) {
	std::vector<sf::Vertex> vertices;

	float textureBeginX = buttonTextureFactor * textureSize.x;

	vertices.emplace_back(sf::Vector2f(getPosition().x, getPosition().y), sf::Vector2f(textureBeginX, 0));
	vertices.emplace_back(sf::Vector2f(getPosition().x + getSize().x, getPosition().y), sf::Vector2f(textureBeginX + textureSize.x, 0));
	vertices.emplace_back(sf::Vector2f(getPosition().x + getSize().x, getPosition().y + getSize().y), sf::Vector2f(textureBeginX + textureSize.x, textureSize.y));
	vertices.emplace_back(sf::Vector2f(getPosition().x, getPosition().y + getSize().y), sf::Vector2f(textureBeginX, textureSize.y));

	window.getSFMLWindow().setView(window.getSFMLWindow().getDefaultView());

	sf::RenderStates rs(&texture);
	window.getSFMLWindow().draw(vertices.data(), vertices.size(), sf::Quads, rs);
}

void FastForwardButton::onClick() {
	if (!cells->getPause()) {
		auto tickAimTime = cells->getTickAimTime();
		if (!cells->getRewinding()) { //If not rewinding.
			if (tickAimTime > toZero)
				cells->setTickAimTime((tickAimTime - std::chrono::milliseconds(1)) / 2); //Increase speed.
			else
				cells->setTickAimTime(std::chrono::milliseconds(0));
		}
		else { //If rewinding.
			auto tickRewindAimTime = cells->getTickRewindAimTime();
			if (tickRewindAimTime > maxSlowDownTime) {
				cells->setRewind(false);
				cells->removeFuture(); //If history past the current point is not removed, rewind is false, and pause is false, the updateCells function will keep on executing. This is not what we want. So remove.
				cells->setPause(false);
			}
			else {
				cells->setTickRewindAimTime((tickRewindAimTime + std::chrono::milliseconds(1)) * 2);
			}
		}
	}
}

void StepForwardButton::onClick() {
	cells->setRewind(false);
	cells->removeFuture();
	cells->setPause(true);
	cells->updateCells();
}

void PauseButton::onClick() {
	cells->setPause(!cells->getPause());
}

void StepBackwardButton::onClick() {
	cells->stepBackInHistory();
	cells->setRewind(true);
	cells->setPause(true);
}

void RewindButton::onClick() {
	if (!cells->getPause()) {
		if (!cells->getRewinding()) { //If not rewinding.
			auto tickAimTime = cells->getTickAimTime();
			if (tickAimTime < maxSlowDownTime) //Slow down.
				cells->setTickAimTime((tickAimTime + std::chrono::milliseconds(1)) * 2);
			else { //Start rewinding.
				cells->setRewind(true);
				cells->setPause(false);
			}
		}
		else { //If currently rewinding.
			auto tickRewindAimTime = cells->getTickRewindAimTime();
			if (tickRewindAimTime > toZero) { //Increase the speed of rewinding.
				cells->setTickRewindAimTime((tickRewindAimTime - std::chrono::milliseconds(1)) / 2);
			}
			else {
				cells->setTickRewindAimTime(std::chrono::milliseconds(0));
			}
		}
	}
}
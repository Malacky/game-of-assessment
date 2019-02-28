#include <iostream>
#include <chrono>
#include <string>

#include <SFML/Graphics.hpp>

#include "Cell.h" 
#include "FileProcessing.h"
#include "Window.h"
#include "HandleInput.h"
#include "GUI.h"

constexpr auto assetsFilePath = "../assets/bitmap.jpg";

int main(int argc, char **argv) {
	//Process the map/rule file, and then construct the map.
	Cells cells;
	try {
		processMapRuleFiles(argv, argc, &cells);
	}
	catch (std::exception &le) {
		std::cerr << le.what() << "\n Press enter to continue.";
		std::string str;
		std::getline(std::cin, str);

		return -1;
	}

	auto &rules = cells.getRules();

	Window window(sf::VideoMode::getDesktopMode(), ".___.");
	window.getSFMLWindow().setVerticalSyncEnabled(false);

	sf::Texture texture;
	texture.loadFromFile(assetsFilePath);
	GUIs guis(texture); //Create guis.
	{
		guis.emplace<RewindButton>(sf::FloatRect(window.getSFMLWindow().getSize().x / 2 - 120, window.getSFMLWindow().getSize().y - 60, 60, 60), cells);
		guis.emplace<StepBackwardButton>(sf::FloatRect(window.getSFMLWindow().getSize().x / 2 - 60, window.getSFMLWindow().getSize().y - 60, 60, 60), cells);
		guis.emplace<PauseButton>(sf::FloatRect(window.getSFMLWindow().getSize().x / 2, window.getSFMLWindow().getSize().y - 60, 60, 60), cells);
		guis.emplace<StepForwardButton>(sf::FloatRect(window.getSFMLWindow().getSize().x / 2 + 60, window.getSFMLWindow().getSize().y - 60, 60, 60), cells);
		guis.emplace<FastForwardButton>(sf::FloatRect(window.getSFMLWindow().getSize().x / 2 + 120, window.getSFMLWindow().getSize().y - 60, 60, 60), cells);
	}

	auto start = std::chrono::steady_clock::now(), end = std::chrono::steady_clock::now();
	auto lastFrameTime = std::chrono::nanoseconds(0), timePassed = std::chrono::nanoseconds(0);
	for (bool done = false; !done;) { //Game loop.
		start = std::chrono::steady_clock::now();

		if (timePassed > minTimeBetweenEachFrame) {
			window.getSFMLWindow().clear(sf::Color::Black);
			cells.render(window); //Render cells.
			guis.render(window);
			window.getSFMLWindow().display();

			timePassed = std::chrono::nanoseconds(0);
		}

		cells.update(); //Update all cells. (start of a tick)

		end = std::chrono::steady_clock::now();
		lastFrameTime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
		timePassed += lastFrameTime;

		for (sf::Event event; window.getSFMLWindow().pollEvent(event);)
		{
			switch (event.type) {
			case sf::Event::Closed:
				done = true;
				break;

			case sf::Event::KeyPressed:
				handleKeyPressed(event.key, window);
				break;

			case sf::Event::MouseWheelScrolled:
				handleMouseWheelScroll(event.mouseWheelScroll, window);
				break;

			case::sf::Event::MouseButtonPressed:
				handleMouseButtonPressed(event.mouseButton, window, guis, cells);
				break;
			}
		}
	}
}

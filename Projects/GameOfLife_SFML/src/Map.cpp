#include "Map.h"

#include <SFML/Graphics.hpp>

bool withinBounds(sf::Vector2f point, sf::FloatRect bounds) {
	return bounds.contains(point);
}
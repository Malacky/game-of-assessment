#include <vector>
#include <chrono>
#include <utility>

#include <SFML/Graphics.hpp>

#include "Cell.h"
#include "Window.h"

sf::Vector2f Cell::size(10, 10);

void Cells::updateCells() {
	history.next();

	for (Cell &cell : *this) {
		expandIfNecessary(cell); //Expand cells when necessary.
	}

	for (Cell &cell : *this) {
		rules(cell); //Evaluate rules and set the 'future'(futureAlive) of the cell.
	}

	for (Cell &cell : *this) { //Apply the 'futures' to the cells.
		bool future = cell.getFutureAlive();
		if (cell.getAlive() != future) {
			cell.setAlive(future);
			historyAppendChange(cell);
		}
	}
}

void Cells::update() {
	timePassedSinceLastTick += std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now() - tickStartTime);

	if (!getPause()) {
		if (!getRewinding()) {
			if (timePassedSinceLastTick > getTickAimTime()) {
				updateCells();
				timePassedSinceLastTick = std::chrono::nanoseconds{ 0 };
			}
		}
		else {
			if (timePassedSinceLastTick > getTickRewindAimTime()) {
				history.last();
				timePassedSinceLastTick = std::chrono::nanoseconds{ 0 };
			}
		}
	}

	tickStartTime = std::chrono::steady_clock::now();
}

void Cells::render(Window &window) const {
	std::vector<sf::Vertex> vertices;

	auto last = cend();
	for (auto beg = cbegin(); beg != last; ++beg) {
		const Cell &cell = *beg;
		Position pos = cell.getPosition();
		const sf::Color &finalColor = cell.getAlive() ? sf::Color::Green : sf::Color::Red;

		vertices.emplace_back(sf::Vector2f(pos.x * cell.size.x, pos.y * cell.size.y), finalColor);
		vertices.emplace_back(sf::Vector2f(pos.x * cell.size.x + cell.size.x, pos.y * cell.size.y), finalColor);
		vertices.emplace_back(sf::Vector2f(pos.x * cell.size.x + cell.size.x, pos.y * cell.size.y + cell.size.y), finalColor);
		vertices.emplace_back(sf::Vector2f(pos.x * cell.size.x, pos.y * cell.size.y + cell.size.y), finalColor);
	}

	window.getSFMLWindow().setView(window.getView());

	window.getSFMLWindow().draw(vertices.data(), vertices.size(), sf::Quads);
}

void addNeighborsToAllCells(Cells &cells) {
	auto cellsAmount = cells.size();

	for (auto beg = cells.begin(), end = cells.end(); beg != end; ++beg) {
		Cell &currCell = *beg;

		currCell.removeAllNeighbors(); //Remove all neighbors first.
		addNeighbors(currCell, cells);
	}
}

void addNeighbors(Cell &cell, Cells& cells) {
	Position cellPosition = cell.getPosition();

	//neighbors are added in order, starting from above the current cell.
	addNeighborToCellIfPossible(cell, cells, cellPosition - Position{ 0, 1 });

	addNeighborToCellIfPossible(cell, cells, cellPosition - Position{ -1, 1 });

	addNeighborToCellIfPossible(cell, cells, cellPosition + Position{ 1, 0 });

	addNeighborToCellIfPossible(cell, cells, cellPosition + Position{ 1, 1 });

	addNeighborToCellIfPossible(cell, cells, cellPosition + Position{ 0, 1 });

	addNeighborToCellIfPossible(cell, cells, cellPosition + Position{ -1, 1 });

	addNeighborToCellIfPossible(cell, cells, cellPosition + Position{ -1, 0 });

	addNeighborToCellIfPossible(cell, cells, cellPosition - Position{ 1, 1 });
}

void Cells::expandIfNecessary(Cell &cell) { //Add cells if current cell touches an edge and it is alive.
	if (cell.getAlive() && cell.neighborCount() < 8) {
		Position cellPosition = cell.getPosition();
		//Construct cells in empty locations.
		addEmptyNeighborToCellsIfPossible(cellPosition - Position{ 0, 1 });

		addEmptyNeighborToCellsIfPossible(cellPosition - Position{ -1, 1 });

		addEmptyNeighborToCellsIfPossible(cellPosition + Position{ 1, 0 });

		addEmptyNeighborToCellsIfPossible(cellPosition + Position{ 1, 1 });

		addEmptyNeighborToCellsIfPossible(cellPosition + Position{ 0, 1 });

		addEmptyNeighborToCellsIfPossible(cellPosition + Position{ -1, 1 });

		addEmptyNeighborToCellsIfPossible(cellPosition + Position{ -1, 0 });

		addEmptyNeighborToCellsIfPossible(cellPosition - Position{ 1, 1 });
	}
}

void Cell::removeAllNeighbors() {
	neighbors.clear();
}


void addNeighborToCellIfPossible(Cell &cell, Cells &cells, Position pos) { //Add a neighbor if it exists.
	Cell *foundCell = cells.find(pos);
	if (foundCell) {
		cell.addNeighbor(foundCell);
	}
}

void Cells::addEmptyNeighborToCellsIfPossible(Position pos) { //Add a new cell to cells if it does not exist, and update neighbors of associated cells.
	if (!find(pos)) {
		Cell &cell = emplace(pos, false); //Initially dead.

		//Add all neighbors to the newly created cell.
		addNeighbors(cell, *this);

		//Add newlyCreatedCell as a neighbor to each neighbor of newlyCreatedCell.
		auto newlyCreatedCellNeighbors = cell.getNeighbors();
		for (Cell *neighbor : newlyCreatedCellNeighbors) {
			neighbor->addNeighbor(&cell);
		}
	}
}

void Cells::CellsHistory::appendChange(Cell &cell) {
	if (currentIndex == cellsChangeContainer.size() - 1)
		cellsChangeContainer.back().push_back(std::make_pair(cell.getPosition(), cell.getAlive()));
}

void Cells::CellsHistory::next() {
	if (currentIndex == cellsChangeContainer.size() - 1) {
		setLookingThroughHistory(false);
		cellsChangeContainer.emplace_back();
		++currentIndex;
	}
}

void Cells::CellsHistory::last() {
	setLookingThroughHistory(true);

	if (currentIndex > 0) {
		std::vector<std::pair<Position, bool>> &currContainer = cellsChangeContainer[currentIndex];
		for (auto &changes : currContainer) { //Iterate through the changes and apply the opposite of what is stored.
			Position pos = changes.first;
			bool alive = changes.second;

			Cell *result = associatedCells->find(pos);
			result->setAlive(!alive);
		}
		--currentIndex;
	}
}

void Cells::CellsHistory::removeFuture() {
	if (currentIndex != cellsChangeContainer.size() - 1)
		cellsChangeContainer.erase(cellsChangeContainer.begin() + currentIndex + 1, cellsChangeContainer.end());
}

void Cells::CellsHistory::setLookingThroughHistory(bool lth) {
	if (lookingThroughHistory != lth) {
		lookingThroughHistory = lth;
		if (!lth) {
			addNeighborsToAllCells(*associatedCells); //Add neighbors if we stopped looking through history.
		}
	}
}
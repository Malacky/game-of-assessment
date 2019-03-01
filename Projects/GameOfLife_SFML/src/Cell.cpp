#include <vector>
#include <chrono>
#include <utility>

#include <SFML/Graphics.hpp>

#include "Cell.h"
#include "Window.h"
#include "Map.h"

sf::Vector2f Cell::size(60, 60);

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
	auto lastTickTime = std::chrono::steady_clock::now() - tickStartTime;
	timePassedSinceLastTick += std::chrono::duration_cast<std::chrono::nanoseconds>(lastTickTime);
	timePassedSinceLastMaintenance += std::chrono::duration_cast<std::chrono::nanoseconds>(lastTickTime);
	tickStartTime = std::chrono::steady_clock::now();

	if (timePassedSinceLastMaintenance > maintenanceTime) { //Remove dead cells to reduce the amount of cells we have to look through in the updateCells function.
		for (auto beg = begin(); beg != end(); ) {
			if (!beg->getAlive()) {
				auto copy = beg;
				++beg;
				removeCell(*copy);
			}
			else
				++beg;
		}
		timePassedSinceLastMaintenance = std::chrono::nanoseconds{ 0 };
	}

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
}

void Cells::render(Window &window) const {
	std::vector<sf::Vertex> vertices;

	auto last = cend();
	for (auto beg = cbegin(); beg != last; ++beg) {
		const Cell &cell = *beg;
		const Position pos = cell.getPosition();
		const sf::Vector2f finalBegPos(pos.x * cell.size.x, pos.y * cell.size.y);

		if (cell.getAlive()) {
			vertices.emplace_back(sf::Vector2f(finalBegPos.x, finalBegPos.y), sf::Color::Green);
			vertices.emplace_back(sf::Vector2f(finalBegPos.x + cell.size.x, finalBegPos.y), sf::Color::Green);
			vertices.emplace_back(sf::Vector2f(finalBegPos.x + cell.size.x, finalBegPos.y + cell.size.y), sf::Color::Green);
			vertices.emplace_back(sf::Vector2f(finalBegPos.x, finalBegPos.y + cell.size.y), sf::Color::Green);
		}
	}

	window.getSFMLWindow().setView(window.getView());

	window.getSFMLWindow().draw(vertices.data(), vertices.size(), sf::Quads);
}

void Cells::addNeighborsToAllCells() {
	auto cellsAmount = size();

	for (auto beg = begin(), last = end(); beg != last; ++beg) {
		Cell &currCell = *beg;

		currCell.removeAllNeighbors(); //Remove all neighbors first.
		addNeighbors(currCell);
	}
}

void Cells::addNeighbors(Cell &cell) {
	Position cellPosition = cell.getPosition();

	//neighbors are added in order, starting from above the current cell.
	addNeighborToCellIfPossible(cell, cellPosition - Position{ 0, 1 });

	addNeighborToCellIfPossible(cell, cellPosition - Position{ -1, 1 });

	addNeighborToCellIfPossible(cell, cellPosition + Position{ 1, 0 });

	addNeighborToCellIfPossible(cell, cellPosition + Position{ 1, 1 });

	addNeighborToCellIfPossible(cell, cellPosition + Position{ 0, 1 });

	addNeighborToCellIfPossible(cell, cellPosition + Position{ -1, 1 });

	addNeighborToCellIfPossible(cell, cellPosition + Position{ -1, 0 });

	addNeighborToCellIfPossible(cell, cellPosition - Position{ 1, 1 });
}

void Cells::expandIfNecessary(Cell &cell) { //Add cells if current cell touches an edge and it is alive.
	if (cell.neighborCount() < 8 && cell.getAlive()) {
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


void Cells::addNeighborToCellIfPossible(Cell &cell, Position pos) { //Add a neighbor if it exists.
	Cell *foundCell = find(pos);
	if (foundCell) {
		cell.addNeighbor(foundCell);
	}
}

void Cells::addEmptyNeighborToCellsIfPossible(Position pos) { //Add a new cell to cells if it does not exist, and update neighbors of associated cells.
	if (!find(pos)) {
		Cell &cell = emplace(pos, false); //Initially dead.

		//Add all neighbors to the newly created cell.
		addNeighbors(cell);

		//Add newlyCreatedCell as a neighbor to each neighbor of newlyCreatedCell.
		addAsNeighborToEachNeighbor(cell);
	}
}

void Cells::addAsNeighborToEachNeighbor(Cell &cell) {
	auto cellNeighbors = cell.getNeighbors();
	for (Cell *neighbor : cellNeighbors) {
		neighbor->addNeighbor(&cell);
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
			if (result)
				result->setAlive(!alive);
			else {
				Cell &cell = associatedCells->emplace(pos, !alive);
				associatedCells->addNeighbors(cell);
				associatedCells->addAsNeighborToEachNeighbor(cell);
			}
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
			associatedCells->addNeighborsToAllCells(); //Add neighbors if we stopped looking through history.
		}
	}
}

void Cells::removeCell(Cell &cell) { //Remove cell and remove cell from neighbors' neighbors container.
	for (Cell *neighbor : cell.getNeighbors()) {
		for (Cell::neighborType::iterator beg = neighbor->getNeighbors().begin(), last = neighbor->getNeighbors().end(); beg != last; ++beg) {
			if (cell == **beg) {
				neighbor->getNeighbors().erase(beg);
				break;
			}
		}
	}

	erase(cell.getPosition());
}
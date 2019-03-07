#ifndef CELL_H
#define CELL_H

#include "Parser.h"
#include "Window.h"
#include "HashTable.h"

#include <SFML/Graphics.hpp>

#include <unordered_map>
#include <string>
#include <iostream>
#include <functional>
#include <cstddef>
#include <utility>
#include <chrono>
#include <type_traits>
#include <limits>
#include <algorithm>

class Position {
public:
	typedef int coordType; //Signed integer.

	Position(coordType x, coordType y) : x{ x }, y{ y } {}
	Position(sf::Vector2f v2f) : x{ coordType(v2f.x) }, y{ coordType(v2f.y) } {}
	Position() = default;

	friend bool operator==(const Position &pos1, const Position &pos2) {
		return pos1.x == pos2.x && pos1.y == pos2.y;
	}
	friend bool operator!=(const Position &pos1, const Position &pos2) {
		return pos1.x != pos2.x && pos1.y != pos2.y;
	}
	friend Position operator+(const Position &pos1, const Position &pos2) {
		return Position{ pos1.x + pos2.x, pos1.y + pos2.y };
	}
	friend Position operator-(const Position &pos1, const Position &pos2) {
		return Position{ pos1.x - pos2.x, pos1.y - pos2.y };
	}

	coordType x = { 0 }, y = { 0 };
};

class PositionHasher { //I know next to nothing about hash functions.
public:
	std::size_t operator()(const Position &pos) const { //https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
		auto hash1 = std::hash<Position::coordType>{}(pos.x), hash2 = std::hash<Position::coordType>{}(pos.y);
		hash1 ^= hash2 + 0x9e3779b97f4a7c16 + (hash1 << 6) + (hash2 >> 2);
		return hash1;
	}
};

class Cells;

class Cell {
public:
	typedef std::vector<Position> neighborPositionType;

	Cell(Position pos, bool a) : position{ pos }, alive{ a }, futureAlive{ a } {}
	Cell() = default;

	friend bool operator==(Cell &cell1, Cell &cell2) {
		return cell1.getPosition() == cell2.getPosition();
	}

	void removeAllNeighbors();

	Position getPosition() const noexcept {
		return position;
	}

	void setFutureAlive(bool fA) noexcept {
		futureAlive = fA;
	}

	bool getFutureAlive() const noexcept {
		return futureAlive;
	}

	bool getAlive() const noexcept {
		return alive;
	}

	void setAlive(bool a) noexcept {
		alive = a;
		futureAlive = a;
	}

	void addNeighborPosition(Position pos) {
		neighborsPositions.push_back(pos);
	}

	bool hasAliveNeighbor(Cells &cells);

	const neighborPositionType &getNeighborsPositions() const noexcept {
		return neighborsPositions;
	}

	neighborPositionType::size_type neighborCount() const noexcept {
		return neighborsPositions.size();
	}

	void removeNeighborPosition(neighborPositionType::const_iterator cIt) {
		neighborsPositions.erase(cIt);
	}

	static sf::Vector2f size;

private:
	neighborPositionType neighborsPositions;
	Position position;
	bool futureAlive; //Determines if the cell is alive after evaluating the rules. (The first part of a tick)
	bool alive;
};

class Cells { //Represents all cells. Cells are stored in an associative container so as to facilitate the adding of neighbors.
public:
	typedef std::size_t size_type;
	typedef Cell value_type;
	typedef Cell &reference;
	typedef const Cell &const_reference;
private:
	class CellsHistory { //Used in the storing of history. Stores the changes made to all cells in each update. We can then loop through those changes to look through history.
	public:
		CellsHistory &operator=(CellsHistory&) = delete;
		CellsHistory &operator=(CellsHistory&&) = delete; //Pointer is passed to constructor, so delete.

		CellsHistory(Cells *ptrCells) : associatedCells{ ptrCells } {
			cellsChangeContainer.emplace_back();
		}

		void appendChange(Cell &cell);
		void next(); //Continues and prepares for the next tick. Called at the beginning of each tick to signify the start of a new part of the history of the cells.
		void last(); //Allows for the retrieval of history.
		void removeFuture(); //Remove all history past the current part of history associated with currentIndex.

		void setLookingThroughHistory(bool lth);

		bool getLookingThroughHistory() {
			return lookingThroughHistory;
		}

	private:
		typedef std::vector<std::vector<std::pair<Position, bool>>> cellsChangeContainerType; //Stores the changes in life of cells to minimalise memory usage. (Instead of storing all cells of each iteration.)

		cellsChangeContainerType cellsChangeContainer;
		Cells *associatedCells;
		cellsChangeContainerType::size_type currentIndex{ 0 };
		bool lookingThroughHistory{ true };
	};

	typedef std::unordered_map<Position, Cell, PositionHasher> cellsContainerType;
	typedef CellsHistory historyType;

	class iterator {
	public:
		iterator(cellsContainerType::iterator cctIt) : baseIterator{ cctIt } {}

		friend bool operator==(const iterator &iterator1, const iterator &iterator2) {
			return iterator1.baseIterator == iterator2.baseIterator;
		}
		friend bool operator!=(const iterator &iterator1, const iterator &iterator2) {
			return iterator1.baseIterator != iterator2.baseIterator;
		}

		reference operator*() {
			return baseIterator->second;
		}
		value_type *operator->() {
			return &baseIterator->second;
		}
		iterator &operator++() {
			++baseIterator;
			return *this;
		}
		iterator operator++(int) {
			return iterator(baseIterator++);
		}
		iterator &operator--() {
			--baseIterator;
			return *this;
		}
		iterator operator--(int) {
			return iterator(baseIterator--);
		}

	private:
		cellsContainerType::iterator baseIterator;
	};

	class const_iterator {
	public:
		const_iterator(cellsContainerType::const_iterator cctIt) : baseIterator{ cctIt } {}

		friend bool operator==(const_iterator &iterator1, const_iterator &iterator2) {
			return iterator1.baseIterator == iterator2.baseIterator;
		}
		friend bool operator!=(const_iterator &iterator1, const_iterator &iterator2) {
			return iterator1.baseIterator != iterator2.baseIterator;
		}

		const_reference operator*() {
			return baseIterator->second;
		}
		const value_type *operator->() {
			return &baseIterator->second;
		}
		const_iterator &operator++() {
			++baseIterator;
			return *this;
		}
		const_iterator operator++(int) {
			return const_iterator(baseIterator++);
		}
		const_iterator &operator--() {
			--baseIterator;
			return *this;
		}
		const_iterator operator--(int) {
			return const_iterator(baseIterator--);
		}

	private:
		cellsContainerType::const_iterator baseIterator;
	};

	friend CellsHistory;

public:
	Cells() : history{ this } {}

	void update();
	void updateCells();
	void render(Window &window) const;
	void expandIfNecessary(Cell &cell);
	void addEmptyNeighborToCellsIfPossible(Position pos);

	void stepBackInHistory() {
		history.last();
	}

	Cell *find(Position pos) {
		auto it = cellsContainer.find(pos);
		return (it != cellsContainer.end()) ? &it->second : nullptr;
	}

	void erase(Position pos) {
		cellsContainer.erase(pos);
	}

	Parser &getRules() noexcept {
		return rules;
	}
	const Parser &getRules() const noexcept {
		return rules;
	}

	cellsContainerType::size_type size() {
		return cellsContainer.size();
	}

	void setRules(std::string str) {
		Parser parser;
		parser(str);
		rules = parser;
	}

	void setRewind(bool r) {
		rewinding = r;
	}

	bool getRewinding() const noexcept {
		return rewinding;
	}

	void setTickAimTime(std::chrono::milliseconds newTickAimTime) {
		tickAimTime = newTickAimTime;
	}

	auto getTickAimTime() const noexcept {
		return tickAimTime;
	}

	void setPause(bool p) {
		pause = p;
	}

	void removeFuture() {
		history.removeFuture();
	}

	auto getTickRewindAimTime() const noexcept {
		return tickRewindAimTime;
	}

	void setTickRewindAimTime(std::chrono::milliseconds trat) {
		tickRewindAimTime = trat;
	}

	bool getPause() const noexcept {
		return pause;
	}

	void historyAppendChange(Cell &cell) {
		history.appendChange(cell);
	}

	Cell &insert(std::pair<Position, Cell> pair) {
		return cellsContainer.emplace(pair).first->second;
	}

	void performMaintenance();

	void removeCell(Cell &cell); //Remove cell and remove cell from neighbors' neighbors container.

	void addNeighborsToAllCells();

	void addNeighbors(Cell &cell);

	void addNeighborToCellIfPossible(Cell &cell, Position pos);

	void addAsNeighborToEachNeighbor(Cell &cell);

	auto begin() {
		return iterator(cellsContainer.begin());
	}
	auto cbegin() const {
		return const_iterator(cellsContainer.cbegin());
	}
	auto end() {
		return iterator(cellsContainer.end());
	}
	auto cend() const {
		return const_iterator(cellsContainer.cend());
	}

private:
	static constexpr std::chrono::seconds maintenanceTime{ 2 };

	cellsContainerType cellsContainer;
	historyType history;
	Parser rules;
	mutable std::vector<sf::Vertex> vertices; //Used in rendering.
	decltype(std::chrono::steady_clock::now()) tickStartTime{ std::chrono::steady_clock::now() };
	std::chrono::nanoseconds timePassedSinceLastTick{ 0 };
	std::chrono::nanoseconds timePassedSinceLastMaintenance{ 0 };
	std::chrono::milliseconds tickAimTime{ 1000 }, tickRewindAimTime{ 1000 };
	bool pause = false;
	bool rewinding = false;
};

Cell::neighborPositionType::size_type aliveNeighborCount(const Cell &cell, Cells &cells);

#endif
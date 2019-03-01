#include <stdexcept>
#include <fstream>
#include <stdexcept>
#include <utility>
#include <cctype>
#include <string>

#include "FileProcessing.h"
#include "Cell.h"

void processMapRuleFiles(char **argv, int argc, Cells *cells) {
	auto test = argv[1];
	if (argc != 3)
		throw(std::invalid_argument("2 arguments must be provided to the program."));

	//Process the rules file first.
	std::ifstream rulesFileStream(argv[1], std::ios::binary | std::ios::in);

	if (!rulesFileStream.is_open())
		throw(std::logic_error("Error opening rules file."));

	std::string rules;
	for (char c; rulesFileStream.get(c);)
		rules += c;

	cells->setRules(rules);


	//Process the map file.
	std::ifstream mapFileStream(argv[2], std::ios::binary | std::ios::in);

	if (!mapFileStream.is_open())
		throw(std::logic_error("Error opening map file."));

	std::string mapStr;
	for (char c; mapFileStream.get(c);)
		mapStr += c;

	addCellsFromStr(*cells, mapStr);
	cells->addNeighborsToAllCells();
}

void addCellsFromStr(Cells &cells, std::string str) {
	if (!str.size())
		throw(std::logic_error("Map Syntax Error: The map must have at least one cell."));

	Position currPos{ 0,0 };
	for (char c : str) {
		switch (c) {
		case '\n':
			++currPos.y;
			currPos.x = 0;
			break;

		case '*':
			cells.emplace(currPos, true);
			++currPos.x;
			break;

		case '#':
			cells.emplace(currPos, false);
			++currPos.x;
			break;

		case ' ':
			++currPos.x;

		case '\r':
			break;

		default:
			throw(std::logic_error(std::string("Map Syntax Error: character '") + c + "' is not allowed"));
		}
	}
}
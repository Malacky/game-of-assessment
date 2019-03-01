#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include <array>
#include <utility>
#include <memory>

class Cell;

//const std::array<char const*, 8> keywordDelimiters{ "->", "ALIVE", "DEAD", "IF N IS", "LESS THAN", "GREATER THAN", "OR EQUAL TO", "\n" };
//const std::array<char const*, 1> identifierDelimiters{ "CELL" };

class Token {
public:
	enum Category {
		keyword,
		identifier,
		literal
	};
	enum Name {
		arrowKeyword,
		aliveKeyword,
		deadKeyword,
		ifnisKeyword,
		lessthanKeyword,
		greaterthanKeyword,
		orequaltoKeyword,
		endofexpressionKeyword,
		cellIdentifier
	};

	Token(Category c, std::string n) : category{ c } {
		if (n == "->")
			name = arrowKeyword;
		if (n == "ALIVE")
			name = aliveKeyword;
		if (n == "DEAD")
			name = deadKeyword;
		if (n == "IF N IS")
			name = ifnisKeyword;
		if (n == "LESS THAN")
			name = lessthanKeyword;
		if (n == "GREATER THAN")
			name = greaterthanKeyword;
		if (n == "OR EQUAL TO")
			name = orequaltoKeyword;
		if (n == "\n")
			name = endofexpressionKeyword;

		if (n == "CELL")
			name = cellIdentifier;

		if (c == literal) {
			optionalValue = std::stoi(n);
		}
	}
	Token() = default;

	int optionalValue{ 0 };
	Category category;
	Name name;
private:
};

class BinaryParseTree {
public:
	BinaryParseTree(Token t) : token{ t } {}
	BinaryParseTree() = default;

	std::shared_ptr<BinaryParseTree> getLeftChild() {
		return leftChild;
	}

	void setLeftChild(std::shared_ptr<BinaryParseTree> newLeftChild) {
		leftChild = newLeftChild;
	}

	std::shared_ptr<BinaryParseTree> getRightChild() {
		return rightChild;
	}

	std::shared_ptr<BinaryParseTree> getParent() {
		return parent;
	}

	void setRightChild(std::shared_ptr<BinaryParseTree> newRightChild) {
		rightChild = newRightChild;
	}

	void setParent(std::shared_ptr<BinaryParseTree> node) {
		parent = node;
	}

	Token token;
private:

	std::shared_ptr<BinaryParseTree> leftChild = nullptr, rightChild = nullptr;
	std::shared_ptr<BinaryParseTree> parent = nullptr;
};

class Tokenizer {
public:
	std::vector<Token> operator()(std::string str);
private:
	std::vector<std::pair<Token, std::string::size_type>> findAllOccurancesOf(std::string str, std::string str2, Token::Category name); //Searches for each occurance of str1 in str2. Returns vector of pairs holding the Token with given name that was found and the index of the start substring in str2.
	std::vector<std::pair<Token, std::string::size_type>> getLiterals(std::string str);

	const std::array<char const*, 8> keywordDelimiters{ "->", "ALIVE", "DEAD", "IF N IS", "LESS THAN", "GREATER THAN", "OR EQUAL TO", "\n" };
	const std::array<char const*, 1> identifierDelimiters{ "CELL" };
};

class Parser {
public:
	void operator()(std::string str);
	void createParseTree(std::vector<Token> tokens);
	Cell &operator()(Cell &cell);
	Cell &evaluateRulesAndSetFuture(Cell &cell, BinaryParseTree &bpt);
	bool conditional(Cell &cell, BinaryParseTree &bpt);

private:
	std::vector<std::shared_ptr<BinaryParseTree>> parseTrees; //The roots of the trees representing expressions.
};

#endif
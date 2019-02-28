#include "Parser.h"
#include "Cell.h"

#include <string>
#include <vector>
#include <stdexcept>
#include <cctype>
#include <algorithm>
#include <utility>
#include <memory>

Cell &Parser::operator()(Cell &cell) { //Apply rules to the cell. (set futures)
	for (auto &tree : parseTrees) {
		evaluateRulesAndSetFuture(cell, *tree);
	}
	return cell;
}

Cell &Parser::evaluateRulesAndSetFuture(Cell &cell, BinaryParseTree &bpt) {
	if (bpt.token.value == "->") {
		if (conditional(cell, bpt)) {
			evaluateRulesAndSetFuture(cell, *bpt.getRightChild());
		}
	}
	else if (bpt.token.value == "IF N IS") {
		if (conditional(cell, bpt)) {
			evaluateRulesAndSetFuture(cell, *bpt.getRightChild());
		}
	}
	else if (bpt.token.value == "ALIVE") {
		cell.setFutureAlive(true);
	}
	else if (bpt.token.value == "DEAD") {
		cell.setFutureAlive(false);
	}

	return cell;
}

bool Parser::conditional(Cell &cell, BinaryParseTree &bpt) { //Evaluate a condition.
	std::shared_ptr<BinaryParseTree> keywordOrLiteralNode = bpt.getLeftChild();
	std::shared_ptr<BinaryParseTree> keywordOrOperandOfKeyworOrLiteralNode = keywordOrLiteralNode->getLeftChild();

	if (bpt.token.value == "IF N IS") {
		if (keywordOrOperandOfKeyworOrLiteralNode) {
			if (keywordOrLiteralNode->token.value == "LESS THAN") {
				if (keywordOrOperandOfKeyworOrLiteralNode->token.value == "OR EQUAL TO") {
					std::shared_ptr<BinaryParseTree> operand = keywordOrOperandOfKeyworOrLiteralNode->getLeftChild();
					return cell.aliveNeighborCount() <= std::stoul(operand->token.value);
				}
				else
					return cell.aliveNeighborCount() < std::stoul(keywordOrOperandOfKeyworOrLiteralNode->token.value);
			}

			else if (keywordOrLiteralNode->token.value == "GREATER THAN") {
				if (keywordOrOperandOfKeyworOrLiteralNode->token.value == "OR EQUAL TO") {
					std::shared_ptr<BinaryParseTree> operand = keywordOrOperandOfKeyworOrLiteralNode->getLeftChild();
					return cell.aliveNeighborCount() >= std::stoul(operand->token.value);
				}
				else
					return cell.aliveNeighborCount() > std::stoul(keywordOrOperandOfKeyworOrLiteralNode->token.value);
			}
		}
		else {
			return cell.aliveNeighborCount() == std::stoul(keywordOrLiteralNode->token.value);
		}
	}
	else { //->
		if (keywordOrLiteralNode->token.value == "ALIVE") {
			if (keywordOrOperandOfKeyworOrLiteralNode->token.value == "CELL")
				return cell.getAlive();
		}
		else if (keywordOrLiteralNode->token.value == "DEAD") {
			if (keywordOrOperandOfKeyworOrLiteralNode->token.value == "CELL")
				return !cell.getAlive();
		}
	}

	throw(std::runtime_error("Node not processed!")); //Should never be evaluated.
}

void Parser::operator()(std::string str) {
	Tokenizer tokenizer;
	std::vector<Token> tokens = tokenizer(str);
	createParseTree(tokens);
}

void Parser::createParseTree(std::vector<Token> tokens) { //Creates parse tree and assigns its roots to data member parseTrees.
	std::shared_ptr<BinaryParseTree> currRoot = nullptr; //Root of the current tree.
	std::shared_ptr<BinaryParseTree> lastNode = nullptr;
	auto end = tokens.end();
	for (auto beg = tokens.begin(); beg != end; ++beg) {
		Token &token = *beg;

		std::shared_ptr<BinaryParseTree> newNode = nullptr;
		if (token.value != "\n") {
			newNode = std::make_shared<BinaryParseTree>(token);

			if (token.value != "CELL" && !lastNode) { //If the first keyword is not equal to "CELL"...
				throw(std::logic_error("Basic Syntax Error."));
			}
		}

		//The token value determines how nodes are ordered amongst each other. In turn, this determines the precedence of 'keywords'.
		//The left children of conditional 'keywords' such as '->' and 'IF N IS' are the conditions itself, the right children are its statements.
		//Non-conditional statements only use their left child as operands.
		//Newline indicates the end of an expression.
		//And perform syntax checking...
		if (token.value == "\n") {
			if (lastNode->token.value == "IF N IS" || lastNode->token.value == "->" || lastNode->token.name == Token::identifier)
				throw(std::logic_error("Syntax Error: A rule cannot end with either the keyword IF N IS, ->, or identifiers."));

			if (currRoot)
				parseTrees.push_back(currRoot); //Add root to the container of expressions.
			currRoot = nullptr;
			lastNode = nullptr;
		}
		else if (token.value == "ALIVE") {
			if (lastNode->token.name == Token::identifier) {
				lastNode->setParent(newNode);
				newNode->setLeftChild(lastNode);
			}
			else {
				lastNode->setRightChild(newNode);
				newNode->setParent(lastNode);
			}
		}
		else if (token.value == "DEAD") {
			if (lastNode->token.name == Token::identifier) {
				lastNode->setParent(newNode);
				newNode->setLeftChild(lastNode);
			}
			else {
				lastNode->setRightChild(newNode);
				newNode->setParent(lastNode);
			}
		}
		else if (token.value == "->") {
			if (lastNode->token.value != "ALIVE" && lastNode->token.value != "DEAD")
				throw(std::logic_error("Syntax Error: The -> keyword must be prepended by the keywords DEAD or ALIVE."));

			lastNode->setParent(newNode);
			newNode->setLeftChild(lastNode);
			currRoot = newNode;
		}
		else if (token.value == "IF N IS") {
			if (!lastNode->getParent())
				throw(std::logic_error("Syntax Error: IF N IS is not valid in this context."));

			lastNode->getParent()->setRightChild(newNode);
			lastNode->setParent(newNode);
			newNode->setRightChild(lastNode);
		}
		else if (token.value == "LESS THAN") {
			if (lastNode->token.value != "IF N IS" && lastNode->token.value != "->")
				throw(std::logic_error("Syntax Error: LESS THAN is not valid in this context."));

			lastNode->setLeftChild(newNode);
			newNode->setParent(lastNode);
		}
		else if (token.value == "GREATER THAN") {
			if (lastNode->token.value != "IF N IS" && lastNode->token.value != "->")
				throw(std::logic_error("Syntax Error: GREATER THAN is not valid in this context."));

			lastNode->setLeftChild(newNode);
			newNode->setParent(lastNode);
		}
		else if (token.value == "OR EQUAL TO") {
			if (lastNode->token.value != "GREATER THAN" && lastNode->token.value != "LESS THAN")
				throw(std::logic_error("Syntax Error: OR EQUAL TO is not valid in this context."));

			lastNode->setLeftChild(newNode);
			newNode->setParent(lastNode);
		}
		else if (token.name == Token::literal) {
			if (lastNode->token.value != "LESS THAN" && lastNode->token.value != "GREATER THAN" && lastNode->token.value != "OR EQUAL TO" && lastNode->token.value != "IF N IS")
				throw(std::logic_error(std::string("Syntax Error: literal '") + newNode->token.value + "' must be prepended by a conditional keyword."));

			lastNode->setLeftChild(newNode);
			newNode->setParent(lastNode);
		}

		lastNode = newNode;
	}
}

std::vector<Token>Tokenizer::operator()(std::string tokensStr) { //Tokenize string provided to the constructor.
	//Detect and remove lines that start with the ';' character (comments).
	std::string commentsRemovedStr;

	bool lineStartsWithCommaCharacter = false;
	bool ignoreUntilNewLine = false;

	char lastChar = '\n';

	for (char c : tokensStr) {
		if (ignoreUntilNewLine) {
			if (c == '\n') {
				ignoreUntilNewLine = false;
				continue;
			}
			else
				continue;
		}

		switch (c) {
		case ';':
			ignoreUntilNewLine = true;
			break;

		case '\f':
		case '\n':
		case '\r':
			if (!lineStartsWithCommaCharacter)
				commentsRemovedStr += c;
			else
				lineStartsWithCommaCharacter = false;
			break;

		default:
			if (!lineStartsWithCommaCharacter)
				commentsRemovedStr += c;
			break;
		}

		lastChar = c;
	}

	//Create tokens by splitting the string into keywords & literals.
	std::vector<std::pair<Token, std::string::size_type>> tokensAndIndex;

	//Delimit string into keywords.
	for (char const* delimiter : keywordDelimiters) {
		auto result = findAllOccurancesOf(std::string(delimiter), commentsRemovedStr, Token::Name::keyword);
		for (auto element : result) {
			tokensAndIndex.push_back(element);
		}
	}

	//Delimit string into identifiers.
	for (char const* delimiter : identifierDelimiters) {
		auto result = findAllOccurancesOf(std::string(delimiter), commentsRemovedStr, Token::Name::identifier);
		for (auto element : result) {
			tokensAndIndex.push_back(element);
		}
	}

	//Delimit string into literals.
	auto result = getLiterals(commentsRemovedStr);
	for (auto element : result) {
		tokensAndIndex.push_back(element);
	}

	//Sort tokensAndIndex to get the original order of the keywords.
	std::sort(tokensAndIndex.begin(), tokensAndIndex.end(), [](std::pair<Token, std::string::size_type> first, std::pair<Token, std::string::size_type> second) -> bool {
		return first.second < second.second;
	});

	//Add all tokens to the final vector of tokens.
	std::vector<Token> tokens;
	for (std::vector<std::pair<Token, std::string::size_type>>::size_type i = 0; i < tokensAndIndex.size(); ++i) {
		tokens.push_back(tokensAndIndex[i].first);
	}

	tokens.emplace_back(Token::keyword, "\n"); //Newline indicates the end of an expression.

	return tokens;
}

std::vector<std::pair<Token, std::string::size_type>> Tokenizer::findAllOccurancesOf(std::string str1, std::string str2, Token::Name name) {
	std::vector<std::pair<Token, std::string::size_type>> vec;

	const auto str1Size = str1.size();
	const auto str2Size = str2.size();
	for (std::string::size_type str1Index = 0, str2Index = 0; str2Index < str2Size; ++str2Index) {
		char c1 = str1[str1Index];
		char c2 = str2[str2Index];

		if (c1 == c2) {
			if (str1Index == str1Size - 1) {
				Token newToken(name, str1);
				vec.push_back(std::make_pair(newToken, str2Index - str1Index));
				str1Index = 0;
			}
			else
				++str1Index;
		}
		else
			str1Index = 0;
	}

	return vec;
}

std::vector<std::pair<Token, std::string::size_type>> Tokenizer::getLiterals(std::string str) {
	std::vector<std::pair<Token, std::string::size_type>> vec;

	std::string currLiteralValue;
	bool wasLastCharDigit = false;
	for (std::string::size_type i = 0; i < str.size(); ++i) {
		unsigned currChar = str[i];

		if (std::isdigit(currChar)) {
			currLiteralValue += currChar;
			wasLastCharDigit = true;
		}
		else if (wasLastCharDigit) {
			Token currToken(Token::Name::literal, currLiteralValue);
			vec.push_back(std::make_pair(currToken, i - currLiteralValue.size()));
			currLiteralValue.clear();
			wasLastCharDigit = false;
		}
	}
	if (wasLastCharDigit) {
		Token currToken(Token::Name::literal, currLiteralValue);
		vec.push_back(std::make_pair(currToken, str.size() - currLiteralValue.size()));
	}

	return vec;
}
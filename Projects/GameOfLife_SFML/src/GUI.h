#ifndef GUI_H
#define GUI_H

#include <vector>
#include <memory>
#include <utility>
#include <chrono>

#include <SFML/Graphics.hpp>

#include "Cell.h"
#include "Window.h"

extern const sf::Vector2f textureSize;

constexpr std::chrono::milliseconds minTimeBetweenEachFrame(15);
constexpr std::chrono::milliseconds maxSlowDownTime(1000);
constexpr std::chrono::milliseconds toZero(1); //When rewinding/fast-forwarding reaches a point below this, it will round down to zero. (Speed limits will be removed.)

enum GUITextureCoordFactors {
	rewindButton,
	stepBackwardButton,
	pauseButton,
	stepForwardButton,
	fastForwardButton
};

class GUI {
public:
	GUI(sf::FloatRect rect) : position{ rect.left, rect.top }, size{ rect.width, rect.height } {}

	virtual void render(Window &window, sf::Texture texture) = 0;

	sf::Vector2f getPosition() {
		return position;
	}

	sf::Vector2f getSize() {
		return size;
	}

	virtual void onClick() = 0;

private:
	sf::Vector2f position;
	sf::Vector2f size; //Width and length.
};

class Button : public GUI {
public:
	Button(GUITextureCoordFactors bTF, sf::FloatRect rect) : GUI(rect), buttonTextureFactor{ bTF }{}

	void render(Window &window, sf::Texture texture) override;
	virtual void onClick() = 0;

private:
	GUITextureCoordFactors buttonTextureFactor;
};

class FastForwardButton : public Button {
public:
	FastForwardButton(sf::FloatRect rect, Cells &c) : Button(GUITextureCoordFactors::fastForwardButton, rect), cells{ &c }{}

	void onClick() override;

private:
	Cells *cells;
};

class StepForwardButton : public Button {
public:
	StepForwardButton(sf::FloatRect rect, Cells &c) : Button(GUITextureCoordFactors::stepForwardButton, rect), cells{ &c }{}

	void onClick() override;

private:
	Cells *cells;
};

class PauseButton : public Button {
public:
	PauseButton(sf::FloatRect rect, Cells &c) : Button(GUITextureCoordFactors::pauseButton, rect), cells{ &c }{}

	void onClick() override;

private:
	Cells *cells;
};

class StepBackwardButton : public Button {
public:
	StepBackwardButton(sf::FloatRect rect, Cells &c) : Button(GUITextureCoordFactors::stepBackwardButton, rect), cells{ &c }{}

	void onClick() override;

private:
	Cells *cells;
};

class RewindButton : public Button {
public:
	RewindButton(sf::FloatRect rect, Cells &c) : Button(GUITextureCoordFactors::rewindButton, rect), cells{ &c }{}

	void onClick() override;

private:
	Cells *cells;
};

class GUIs {
	typedef std::vector<std::shared_ptr<GUI>> GUIsContainerType;

public:
	template<typename T> class iterator {
	public:
		iterator(T it) : baseIterator{ it } {}

		friend bool operator==(iterator &iterator1, iterator &iterator2) {
			return iterator1.baseIterator == iterator2.baseIterator;
		}
		friend bool operator!=(iterator &iterator1, iterator &iterator2) {
			return iterator1.baseIterator != iterator2.baseIterator;
		}

		GUI &operator*() {
			return **baseIterator;
		}
		GUI *operator->() {
			return &**baseIterator;
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
		T baseIterator;
	};

	typedef std::size_t size_type;
	typedef iterator<GUIsContainerType::const_iterator> const_iterator;

	GUIs(sf::Texture t) : texture{ t } {}

	void render(Window &window) {
		for (std::shared_ptr<GUI> gui : guis) {
			gui->render(window, texture);
		}
	}

	sf::Texture &getTexture() {
		return texture;
	}

	template<typename T, typename... Types> GUI &emplace(Types&&... args) {
		return *guis.emplace_back(std::make_shared<T>(std::forward<Types>(args)...));
	}

	auto begin() {
		return iterator<GUIsContainerType::iterator>(guis.begin());
	}
	auto cbegin() {
		return iterator<GUIsContainerType::const_iterator>(guis.cbegin());
	}
	auto end() {
		return iterator<GUIsContainerType::iterator>(guis.end());
	}
	auto cend() {
		return iterator<GUIsContainerType::const_iterator>(guis.cend());
	}

private:
	GUIsContainerType guis;
	sf::Texture texture;
};

#endif
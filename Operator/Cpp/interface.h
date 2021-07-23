#pragma once

#include <SFML/Graphics.hpp>
#include "Msqtt.h"
#include "recognition.h"

class Label : public sf::Drawable
{
protected:
	int x;
	int y;
	std::string text;
	std::string fileFont;
	sf::Font font;
	sf::Text label;
public:
	Label(int x_, int y_, std::string text_);
	void RotateTxt(int angle);
	void Move(int x_, int y_);
	sf::Text DisplayText();
	sf::Text UpdateText(std::string str);
private:
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

class Button : public Label
{
public:
	Button(int x_, int y_, int width_, int height_, std::string text_, Commands command_);
	Button& operator=(const Button& button);
	sf::RectangleShape DisplayButton();
	bool IsClicked(const sf::Vector2i& mouse);
	void Move(int x_, int y_);
	Commands GetCommand();
private:
	int width;
	int height;
	Commands command;
	sf::RectangleShape box;
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

class Joystick : public sf::Drawable
{
public:
	Joystick(int x, int y);
	void Move(int x, int y);
	Commands IsClicked(const sf::Vector2i& mouse);
private:
	Button moveForward;
	Button moveBackward;
	Button rotateLeft;
	Button rotateRight;
	Button stop;
	sf::CircleShape frame;
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
};

class MainWindow
{
public:
	MainWindow();
	void DrawAll();
	void UpdateSprite(const sf::Image& image);
	void MainCycle(Operator& client);
private:
	sf::RenderWindow window; 

	sf::Image icon;

	sf::Texture mainTexture;
	sf::Sprite mainSprite;

	sf::Texture helperTexture;
	sf::Sprite helperSprite;

	Joystick joystick;
	Button autoMode;
	Button manualMode;
};
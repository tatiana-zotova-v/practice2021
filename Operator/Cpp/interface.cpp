#include "interface.h"

Label::Label(int x_ = 0, int y_ = 0, std::string text_ = "")
	: x(x_)
	, y(y_)
	, text(text_)
{
	fileFont = "C:/Users/MI/QtProject/bot_HMI/Gardens CM.ttf";
	if (font.loadFromFile(fileFont))
	{
		label.setFont(font);
		label.setCharacterSize(30);
		label.setOutlineColor(sf::Color::White);
		label.setPosition(x, y);
		label.setString(text);
	}
}

sf::Text Label::DisplayText()
{
	return label;
}

sf::Text Label::UpdateText(std::string str)
{
	label.setString(str);
	return label;
}

void Label::RotateTxt(int angle)
{
	label.rotate(angle);
	label.move(47, -7);
}

void Label::Move(int x_, int y_)
{
	label.move(x_, y_);
	x += x_;
	y += y_;
}

void Label::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(label, states);
}

Button::Button(int x_ = 0, int y_ = 0, int width_ = 0, int height_ = 0, std::string text_ = "", Commands command_ = Commands::IN_SEQUENCE)
	: Label(x_, y_, text_)
	, width(width_)
	, height(height_)
	, command(command_)
{
	box.setSize(sf::Vector2f(width, height));
	box.setFillColor(sf::Color(118, 159, 205, 255));//(119, 144, 210, 255));//67, 102, 176
	box.setPosition(x, y);
	box.setOutlineThickness(1);
	box.setOutlineColor(sf::Color(109, 138, 199, 255));
}

sf::RectangleShape Button::DisplayButton()
{
	return box;
}

bool Button::IsClicked(const sf::Vector2i& mouse)
{
	sf::Vector2f position = box.getPosition();
	return ((mouse.x > position.x) && (mouse.x < position.x + width) && (mouse.y > position.y) && (mouse.y < position.y + height))
		? true
		: false;
}

void Button::Move(int x_, int y_)
{
	Label::Move(x_, y_);
	box.move(x_, y_);
}

void Button::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(box, states);
	target.draw(label, states);
}

Button& Button::operator=(const Button& button)
{
	x = button.x;
	y = button.y;
	text = button.text;
	fileFont = button.fileFont;
	if (font.loadFromFile(fileFont))
	{
		label.setFont(font);
		label.setCharacterSize(30);
		label.setOutlineColor(sf::Color::White);
		label.setPosition(x, y);
		label.setString(text);
	}

	width = button.width;
	height = button.height;
	command = button.command;
	box.setSize(sf::Vector2f(width, height));
	box.setFillColor(sf::Color(118, 159, 205, 255));
	box.setPosition(x, y);
	box.setOutlineThickness(0);
	box.setOutlineColor(sf::Color(109, 138, 199, 255));

	return *this;
}

Commands Button::GetCommand()
{
	return command;
}

Joystick::Joystick(int x = 0, int y = 0)
{
	stop = Button(x + 65, y + 65, 50, 50, "·", Commands::PAUSE);
	stop.Label::Move(21, 6);
	rotateLeft = Button(x + 20, y + 65, 45, 50, "<", Commands::ROT_L);
	rotateLeft.Label::Move(10, 3);
	rotateRight = Button(x + 115, y + 65, 45, 50, ">", Commands::ROT_R);
	rotateRight.Label::Move(24, 3);
	moveForward = Button(x + 65, y + 20, 50, 45, "<", Commands::STR_M_F);
	moveForward.RotateTxt(90);
	moveForward.Label::Move(0, 18);
	moveBackward = Button(x + 65, y + 115, 50, 45, ">", Commands::STR_M_B);
	moveBackward.RotateTxt(90);
	moveBackward.Label::Move(0, 30);

	frame.setRadius(90.f);
	frame.setFillColor(sf::Color(214, 230, 242, 255));
	frame.setPosition(x, y);
}

void Joystick::Move(int x, int y)
{
	frame.move(x, y);
	stop.Move(x, y);
	rotateLeft.Move(x, y);
	rotateRight.Move(x, y);
	moveForward.Move(x, y);
	moveBackward.Move(x, y);
}

Commands Joystick::IsClicked(const sf::Vector2i& mouse)
{
	if (moveForward.IsClicked(mouse))
	{
		return moveForward.GetCommand();
	}
	else if (moveBackward.IsClicked(mouse))
	{
		return moveBackward.GetCommand();
	}
	else if (rotateLeft.IsClicked(mouse))
	{
		return rotateLeft.GetCommand();
	}
	else if (rotateRight.IsClicked(mouse))
	{
		return rotateRight.GetCommand();
	}
	else if (stop.IsClicked(mouse))
	{
		return stop.GetCommand();
	}
	return Commands::DEFAULT;
}

void Joystick::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	target.draw(frame, states);
	target.draw(moveForward, states);
	target.draw(moveBackward, states);
	target.draw(rotateLeft, states);
	target.draw(rotateRight, states);
	target.draw(stop, states);
}

MainWindow::MainWindow()
{
	window.create(sf::VideoMode(650, 867), "RobotControl");

	icon.loadFromFile("C:/Users/MI/QtProject/bot_HMI/bot.png");
	window.setIcon(48, 48, icon.getPixelsPtr());

	mainTexture.loadFromFile("C:/Users/MI/QtProject/bot_HMI/hok.jpg");

	mainSprite.setTexture(mainTexture);
	mainSprite.setPosition(5, 5);

	helperTexture.loadFromFile("C:/Users/MI/QtProject/bot_HMI/Helper2.png");
	helperSprite.setTexture(helperTexture);
	helperSprite.setPosition(232, 474);

	joystick.Move(65, 627);
	autoMode = Button(50, 555, 100, 50, "Auto", Commands::AUTO_MODE);
	autoMode.Label::Move(18, 2);
	manualMode = Button(160, 555, 100, 50, "Manual", Commands::MANUAL_MODE);
	manualMode.Label::Move(4, 2);
}

void MainWindow::DrawAll()
{
	window.clear(sf::Color(247, 251, 252, 255));

	window.draw(mainSprite);
	window.draw(joystick);
	window.draw(manualMode);
	window.draw(autoMode);
	window.draw(helperSprite);

	window.display();
}

void MainWindow::UpdateSprite(const sf::Image& image)
{
	if (image.getPixelsPtr() != nullptr)
	{
		mainTexture.update(image);
		mainSprite.setTexture(mainTexture);
	}
}

void MainWindow::MainCycle(Operator& client)
{
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::MouseButtonPressed)
			{
				if (event.mouseButton.button == sf::Mouse::Left)
				{
					sf::Vector2i mouse = sf::Mouse::getPosition(window);

					ControlType modeClient = client.GetMode();

					if (manualMode.IsClicked(mouse) && modeClient != ControlType::MANUAL)
					{
						uint8_t command = static_cast<int>(manualMode.GetCommand()) + '0';
						client.SendCommand(topics[3], &command, 1);
						client.SetMode(ControlType::MANUAL);
					}
					else if (autoMode.IsClicked(mouse) && modeClient != ControlType::AUTO)
					{
						uint8_t command = static_cast<int>(autoMode.GetCommand()) + '0';
						client.SendCommand(topics[3], &command, 1);
						client.SetMode(ControlType::AUTO);
					}
					else if (modeClient == ControlType::MANUAL)
					{
						Commands Clicked = joystick.IsClicked(mouse);
						if (Clicked != Commands::DEFAULT)
						{
							uint8_t command = static_cast<int>(Clicked) + '0';
							client.SendCommand(topics[2], &command, 1);
						}
					}
				}
			}
		}

		message_lock.lock();
		if (recieved.payload != nullptr)
		{
			char* topic = recieved.topic;
			if (topic != nullptr)
			{
				if (topic == topics[0])
				{
					sf::Image* result = new sf::Image;
					if (client.GetMode() == ControlType::AUTO && client.GetState() != FindingStates::FOUND_ALL) *result = client.ParsePic();
					else result->loadFromMemory(recieved.payload, recieved.payloadlen);
					UpdateSprite(*result);
					delete result;
				}
				else if (topic == topics[4])
				{
					client.ParseCommand();
				}
			}
			recieved.payload = nullptr;
		}
		message_lock.unlock();

		DrawAll();
	}
}

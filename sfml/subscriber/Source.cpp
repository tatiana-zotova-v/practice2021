#include <SFML/Graphics.hpp>
#include "Msqtt.h"
#include <opencv2/core.hpp>
#include "recognition.h"

using namespace sf;

int main()
{
	myMosq mm("client1", "#", "127.0.0.1", 1883);
	int mid = 0;
	mm.subscribe(&mid, "#", 2);

	RenderWindow window(sf::VideoMode(640, 480), "RobotControl"); 
	
	Image image; 
	image.loadFromFile("C:/Users/MI/QtProject/bot_HMI/2.jpg");

	cv::Mat img_mat = sfml2opencv(image);
	cv::Mat thresholded = GetThresholdedMat(img_mat);

	std::vector<std::vector<cv::Point>> contours;

	sf::Image result = GetCountorsPic(thresholded, img_mat, contours);
	std::cout << "size = " << contours.size() << std::endl;
	
	if (contours.size() == 1)
	{
		float point = GetCentralPoint(contours, image);
		std::cout << "point " << point << std::endl;
	}

	Texture texture;
	texture.loadFromImage(result);

	Sprite sprite;//создаем объект Sprite(спрайт)
	sprite.setTexture(texture);//передаём в него объект Texture (текстуры)
	sprite.setPosition(50, 25);//задаем начальные координаты появления спрайта
//*/
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		if (!cnt)//(recieved != nullptr)
		{
			cnt++;
		   /* 
			* Преобразование массива байт в картинку, распознавание, установка на экран
			* Пока не проверяли исправность работы этого блока
			heroimage.loadFromMemory(recieved->payload, recieved->payloadlen);
			cv::Mat img_mat = sfml2opencv(heroimage);
			cv::Mat thresholded = GetThresholdedMat(img_mat);

			std::vector<std::vector<cv::Point>> contours;

			result = GetCountorsPic(thresholded, img_mat, contours);
//			std::cout << "size = " << contours.size() << std::endl;

			mm.send_message(contours, image);

			texture.loadFromImage(result);

			sprite.setTexture(herotexture);
			sprite.setPosition(50, 25);
		   */
			char* str = new char[recieved->payloadlen];
			str = static_cast<char*>(recieved->payload);
			for (int i = 0; i < recieved->payloadlen; i++)
			{
				std::cout << str[i];
			}
			std::cout << std::endl;

			//mosquitto_message_free(&recieved);
			cnt = 1;
		}

		window.clear();
		window.draw(sprite);
		window.display();
	}

	return 0;
}
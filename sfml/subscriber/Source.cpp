#include <SFML/Graphics.hpp>
#include "Msqtt.h"
#include <opencv2/core.hpp>
#include "recognition.h"

int main()
{
	mosquitto* mosq = nullptr;

	mosquitto_lib_init();

	mosq = mosquitto_new("client1", 0, nullptr);
	bool Connect;
	if (mosq)
	{
		mosquitto_connect_callback_set(mosq, OnConnect);
		mosquitto_publish_callback_set(mosq, OnPublish);
		mosquitto_subscribe_callback_set(mosq, OnSubscribe);
		mosquitto_message_callback_set(mosq, OnMessage);
		mosquitto_disconnect_callback_set(mosq, OnDisconnect);

		Connect = (mosquitto_connect(mosq, "127.0.0.1", 1883, 60) == MOSQ_ERR_SUCCESS) ? true : false;//5.145.246.237

		mosquitto_loop_start(mosq);

		int mid0 = 0;
		mosquitto_subscribe(mosq, &mid0, "#", 2);
	}

	sf::RenderWindow window(sf::VideoMode(640, 480), "RobotControl"); 
	
	sf::Image* image = nullptr; 
	sf::Image* result = nullptr;
	std::vector<std::vector<cv::Point>> contours;

	sf::Texture* texture = nullptr;

	sf::Sprite* sprite = nullptr;

	cv::Mat* img_mat = nullptr;
	cv::Mat* thresholded = nullptr;
	
	image = new sf::Image;
	image->loadFromFile("C:/Users/MI/QtProject/bot_HMI/5.jpg");
	
	texture = new sf::Texture;
	texture->loadFromImage(*image);

	sprite = new sf::Sprite;
	sprite->setTexture(*texture);
	sprite->setPosition(5, 5);
	
	recieved.payload = nullptr;

	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}

		message_lock.lock();
		if (recieved.payload != nullptr)
		{
			char* topic = recieved.topic;
			if (topic != nullptr)
			{
				if (topic == topics[0])
				{
					image = new sf::Image;
					image->loadFromMemory(recieved.payload, recieved.payloadlen);
					img_mat = new cv::Mat;
					*img_mat = sfml2opencv(image);
					thresholded = new cv::Mat;
					*thresholded = GetThresholdedMat(img_mat/*, static_cast<int>(PUCK)*/);

					std::vector<std::vector<cv::Point>> contours;

					result = new sf::Image;
					*result = GetCountorsPic(thresholded, img_mat, contours);

					SendMessage(contours, image, mosq);

					texture = new sf::Texture;
					texture->loadFromImage(*result);

					sprite = new sf::Sprite;
					sprite->setTexture(*texture);
					sprite->setPosition(5, 5);

					recieved.payload = nullptr;
					contours.clear();
					delete image;
					delete result;
					delete img_mat;
					delete thresholded;
				}
				else if (topic == topics[1])
				{

				}
				else if (topic == topics[2])
				{

				}
				else if (topic == topics[3])
				{

				}
			}
		}
		
		message_lock.unlock();

		window.clear();
		window.draw(*sprite);
		window.display();

		if (sprite != nullptr && texture != nullptr)
		{
			//delete texture;//????
			//delete sprite;//????
		}
	}

	if (mosq)
	{
		if (Connect)
		{
			mosquitto_disconnect(mosq);
		}
		mosquitto_loop_stop(mosq, false);
		mosquitto_destroy(mosq);
		mosquitto_lib_cleanup();
	}

	return 0;
}
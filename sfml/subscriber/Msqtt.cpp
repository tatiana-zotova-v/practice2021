#include "Msqtt.h"

std::mutex message_lock;
struct mosquitto_message recieved;
int cnt = 1;

std::vector<std::string> topics
{ 
	"/image",
	"/command/auto",// первое число - 4(вращение);34 
	"/command/manual",
	"/command/mode",
	"/state"
};

void OnConnect(struct mosquitto* mosc, void* obj, int rc)
{
	std::cout << ">> MQTT connected " << rc << std::endl;
}

void OnPublish(struct mosquitto* mosc, void* obj, int mid)
{
	std::cout << "The message with " << mid << " has been published" << std::endl;
}

void OnSubscribe(struct mosquitto* mosc, void* obj, int mid, int qos_count, const int* granted_qos)
{
	std::cout << "aoaoaoa subscribe!!!! aoaoao" << std::endl;
}
		
void OnMessage(struct mosquitto* mosc, void* obj, const struct mosquitto_message* message)
{
	std::cout << "Yaaaahooo we have got a new message" << std::endl;
	mosquitto_message_copy(&recieved, message);
}

void OnDisconnect(struct mosquitto* mosc, void* obj, int rc)
{
	std::cout << ">> MQTT disconnected " << rc << std::endl;
}

void SendMessage(const std::vector<std::vector<cv::Point>>& contours, sf::Image* img, struct mosquitto* mosq)
{
	float angle;
	int mid = 2; // NULL mojno
	int toManual = 0;

	if (contours.size() == 1)
	{
		if (toManual) toManual = 0;
		float point = GetCentralPoint(contours, img);
		angle = Radian2Degrees(GetAngle(point));
		mosquitto_publish(mosq, &mid, "topic", sizeof(float), &angle, 2, 1);//ugol
		mosquitto_publish(mosq, &mid, "topic", sizeof(float), &angle, 2, 1);//spat'
		mosquitto_publish(mosq, &mid, "topic", sizeof(float), &angle, 2, 1);//idti
//		std::cout << "point " << point << std::endl;
	}
	else
	{
		if (toManual > 5)
		{
			mosquitto_publish(mosq, &mid, "topic", sizeof(float), &angle, 2, 1);//spat' 0.5	
			mosquitto_publish(mosq, &mid, "topic", sizeof(float), &angle, 2, 1);//perehod manual
			toManual = 0;
		}
		else
		{
			toManual++;
			angle = 60;
			mosquitto_publish(mosq, &mid, "topic", sizeof(float), &angle, 2, 1);
			mosquitto_publish(mosq, &mid, "topic", sizeof(float), &angle, 2, 1);//spat' 0.5			
		}
	}
}
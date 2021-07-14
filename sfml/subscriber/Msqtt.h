#pragma once

#include "recognition.h"
#include <iostream>
#include <mosquittopp.h>
#include <mutex>
#pragma comment (lib, "mosquittopp.lib")
#pragma comment (lib, "mosquitto.lib")

extern std::mutex message_lock;
extern struct mosquitto_message recieved;
//extern struct mosquitto_message** recievedPtr;
extern int cnt;

class myMosq : public mosqpp::mosquittopp
{
private:
	const char* host;
	const char* id;
	const char* topic;
	int         port;
	int         keepalive;

	void on_connect(int rc);
	void on_disconnect(int rc);
	void on_publish(int mid);
	void on_message(const struct mosquitto_message* message);
public:
	myMosq(const char* id, const char* _topic, const char* host, int port);
	~myMosq();
	void send_message(const std::vector<std::vector<cv::Point>>& contours, const sf::Image& img);
};
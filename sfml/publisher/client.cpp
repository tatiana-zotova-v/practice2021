#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <cstdio>
#include <mosquittopp.h>
#include <string>
#include <chrono>
#include <thread>

#pragma comment (lib, "mosquittopp.lib")
#pragma comment (lib, "mosquitto.lib")

struct mosquitto_message* recieved;

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
};

myMosq::myMosq(const char* _id, const char* _topic, const char* _host, int _port) : mosquittopp(_id)
{
	mosqpp::lib_init();
	this->keepalive = 60;
	this->id = _id;
	this->port = _port;
	this->host = _host;
	this->topic = _topic;
	connect_async(host,
		port,
		keepalive);
	loop_start();
};

myMosq::~myMosq() {
	loop_stop();
	mosqpp::lib_cleanup();
}

void myMosq::on_disconnect(int rc) {
	std::cout << ">> myMosq - disconnection(" << rc << ")" << std::endl;
}

void myMosq::on_connect(int rc)
{
	if (rc == 0) {
		std::cout << ">> myMosq - connected with server" << std::endl;
	}
	else {
		std::cout << ">> myMosq - Impossible to connect with server(" << rc << ")" << std::endl;
	}
}

void myMosq::on_publish(int mid)
{
	std::cout << ">> myMosq - Message (" << mid << ") succeed to be published " << std::endl;
}

void myMosq::on_message(const struct mosquitto_message* message) {
	std::cout << ">> myMosq recieved message" << std::endl;

	//	recieved = static_cast<mosquitto_message*>(malloc(sizeof(message)));
	recieved = new mosquitto_message;
	mosquitto_message_copy(recieved, message);
}

int main()
{
	myMosq client2("client2", "#", "127.0.0.1", 1883);
	int mid = 1;
	char message[] = "Hello i can not send a pic";
	for (int i = 0; i < sizeof(message) - 1; i++)
	{
		client2.publish(&mid, "topic", sizeof(char), &message[i], 2);
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}

	return 0;
}
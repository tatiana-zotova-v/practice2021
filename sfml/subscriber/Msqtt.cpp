#include "Msqtt.h"

struct mosquitto_message* recieved;
int cnt = 1;

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

void myMosq::send_message(const std::vector<std::vector<cv::Point>>& contours, const sf::Image& img)
{
	float angle;
	int mid = 2; // NULL mojno
	int toManual = 0;

	if (contours.size() == 1)
	{
		if (toManual) toManual = 0;
		float point = GetCentralPoint(contours, img);
		angle = Radian2Degrees(GetAngle(point));
		publish(&mid, "topic", sizeof(float), &angle, 2);//ugol
		publish(&mid, "topic", sizeof(float), &angle, 2);//spat'
		publish(&mid, "topic", sizeof(float), &angle, 2);//idti
//		std::cout << "point " << point << std::endl;
	}
	else
	{
		if (toManual > 5)
		{
			publish(&mid, "topic", sizeof(float), &angle, 2);//spat' 0.5	
			publish(&mid, "topic", sizeof(float), &angle, 2);//perehod manual
			toManual = 0;
		}
		else
		{
			toManual++;
			angle = 60;
			publish(&mid, "topic", sizeof(float), &angle, 2);
			publish(&mid, "topic", sizeof(float), &angle, 2);//spat' 0.5			
		}
	}
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
/*
	recieved.mid = message->mid;
	recieved.payloadlen = message->payloadlen;
	memmove(recieved.payload, message->payload, message->payloadlen);//memcpy 
	recieved.qos = message->qos;
	recieved.retain = message->retain;
//	memcpy(recieved.topic, message->topic, ???);
*/
//	recieved = static_cast<mosquitto_message*>(malloc(sizeof(message)));
	recieved = new mosquitto_message;
	mosquitto_message_copy(recieved, message);
	cnt = 0;
}
#pragma once

#include "recognition.h"
#include <iostream>
#include <mosquitto.h>
#include <mutex>
#include <string>

#pragma comment (lib, "mosquitto.lib")

extern std::mutex message_lock;
extern struct mosquitto_message recieved;
//extern struct mosquitto_message** recievedPtr;
extern std::vector<std::string> topics;

void OnConnect(struct mosquitto* mosc, void* obj, int rc);
void OnPublish(struct mosquitto* mosc, void* obj, int mid);
void OnSubscribe(struct mosquitto* mosc, void* obj, int mid, int qos_count, const int* granted_qos);
void OnMessage(struct mosquitto* mosc, void* obj, const struct mosquitto_message* message);
void OnDisconnect(struct mosquitto* mosc, void* obj, int rc);

void SendMessage(const std::vector<std::vector<cv::Point>>& contours, sf::Image* img, struct mosquitto* mosq);
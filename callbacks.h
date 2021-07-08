#ifndef CALLBACKS_H
#define CALLBACKS_H
#pragma once
#include <mosquitto.h>
#include <iostream>
#include <QByteArray>

extern struct mosquitto_message* recieved;

void OnConnect(struct mosquitto* mosc, void* obj, int rc);
void OnPublish(struct mosquitto* mosc, void* obj, int mid);
void OnSubscribe(struct mosquitto* mosc, void* obj, int mid, int qos_count, const int* granted_qos);
void OnMessage(struct mosquitto* mosc, void* obj, const struct mosquitto_message* message);
void OnDisconnect(struct mosquitto* mosc, void* obj, int rc);

#endif // CALLBACKS_H

#pragma once

#include "recognition.h"
#include <iostream>
#include <mosquitto.h>
#include <mutex>
#include <string>

#pragma comment (lib, "mosquitto.lib")

extern std::mutex message_lock;
extern struct mosquitto_message recieved;
extern std::vector<std::string> topics;

enum class Commands
{
	PAUSE = 0,
	AUTO_MODE = 1,
	MANUAL_MODE = 2,
	STR_M_F = 5,
	STR_M_B = 6,
	ROT_R = 7,
	ROT_L = 8,
	END_OF_STATE_SEQUENCE = 9,
	IN_SEQUENCE = 10,
};

enum class ControlType
{
	NO, 
	AUTO, 
	MANUAL,
};

enum class FindingStates
{
	FINDING_PUCK, 
	FINDING_GOAL,
};

void OnConnect(struct mosquitto* mosc, void* obj, int rc);
void OnPublish(struct mosquitto* mosc, void* obj, int mid);
void OnSubscribe(struct mosquitto* mosc, void* obj, int mid, int qos_count, const int* granted_qos);
void OnMessage(struct mosquitto* mosc, void* obj, const struct mosquitto_message* message);
void OnDisconnect(struct mosquitto* mosc, void* obj, int rc);

class MosquittoClient
{
public:
	MosquittoClient(std::string nameClient, std::string IP, int port, bool retain_, int qos_);
	~MosquittoClient();
protected:
	mosquitto* client;
	bool retain;
	int qos;
private:
	bool connect;
};

class Operator : public MosquittoClient
{
public:
	Operator(std::string nameClient, std::string IP, int port, bool retain, int qos);
	void SendSequenceCommands(bool found, int angle);
	void SendCommand(const std::string& topic, uint8_t* message, int size);
	Commands CurrentBotState();
	sf::Image ParsePic();
	void ParseCommand();
private:
	ControlType mode;
	FindingStates state;
	Commands botState;
	int toManual;
	//recognizer reco???
};
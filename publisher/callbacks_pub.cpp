#include "callbacks.h"

struct mosquitto_message* recieved;

void OnConnect(struct mosquitto* mosc, void* obj, int rc)
{
	std::cout << "MQTT connected with " << rc << " code" << std::endl;
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
    std::cout << "Yaaaahooo we have got a new message    " << message->payloadlen << std::endl;
}

void OnDisconnect(struct mosquitto* mosc, void* obj, int rc)
{
	std::cout << "MQTT disconnected with " << rc << " code" << std::endl;
}

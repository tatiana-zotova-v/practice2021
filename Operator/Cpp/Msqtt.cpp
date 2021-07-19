#include "Msqtt.h"

std::mutex message_lock;
struct mosquitto_message recieved;

std::vector<std::string> topics
{ 
	"/image",
	"/command/auto",
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
	std::cout << "The message with has been published" << std::endl;
}

void OnSubscribe(struct mosquitto* mosc, void* obj, int mid, int qos_count, const int* granted_qos)
{
	std::cout << "aoaoaoa subscribe!!!! aoaoao" << std::endl;
}
		
void OnMessage(struct mosquitto* mosc, void* obj, const struct mosquitto_message* message)
{
	//std::cout << "Yaaaahooo we have got a new message" << std::endl;
	mosquitto_message_copy(&recieved, message);
}

void OnDisconnect(struct mosquitto* mosc, void* obj, int rc)
{
	std::cout << ">> MQTT disconnected " << rc << std::endl;
	//connect = false;
}

MosquittoClient::MosquittoClient(std::string nameClient, std::string IP, int port, bool retain_, int qos_)
	: retain(retain_)
	, qos(qos_)
{
	mosquitto_lib_init();

	client = mosquitto_new(nameClient.c_str(), 0, nullptr);

	if (client)
	{
		mosquitto_connect_callback_set(client, OnConnect);
		mosquitto_publish_callback_set(client, OnPublish);
		mosquitto_subscribe_callback_set(client, OnSubscribe);
		mosquitto_message_callback_set(client, OnMessage);
		mosquitto_disconnect_callback_set(client, OnDisconnect);

		connect = (mosquitto_connect(client, IP.c_str(), port, 60) == MOSQ_ERR_SUCCESS)
			? true 
			: false;//5.145.246.237

		mosquitto_loop_start(client);

		mosquitto_subscribe(client, nullptr, "#", 2);

		recieved.payload = nullptr;
	}
	else
	{
		connect = false;
	}
}

MosquittoClient::~MosquittoClient()
{
	if (client)
	{
		if (connect)
		{
			mosquitto_disconnect(client);
		}
		mosquitto_loop_stop(client, false);
		mosquitto_destroy(client);
		mosquitto_lib_cleanup();
	}
}

Operator::Operator(std::string nameClient, std::string IP, int port, bool retain, int qos)
	: MosquittoClient(nameClient, IP, port, retain, qos)
{
	mode = ControlType::NO;
	state = FindingStates::FINDING_PUCK;
	botState = Commands::PAUSE;
	toManual = 0;
}

void Operator::SendSequenceCommands(bool found, int angle)
{
	int* mid = nullptr;
	int size = (angle > 10) ? 4 : 3;
	uint8_t* rotate = new uint8_t[size];
	rotate[0] = '4';
	rotate[1] = ';';
	for (int i = size - 1; i >= 2; i--)
	{
		rotate[i] = angle % 10 + '0';
		angle /= 10;
	}

	if (found)
	{
		if (toManual) toManual = 0;
		
		mosquitto_publish(client, mid, topics[1].c_str(), 4, rotate, qos, retain);//ugol
		mosquitto_publish(client, mid, topics[1].c_str(), 3, "3;1", qos, retain);//idti
		mosquitto_publish(client, mid, topics[1].c_str(), 3, "0;1", qos, retain);//spat'
	}
	else
	{
		if (toManual == 6)
		{
			mosquitto_publish(client, mid, topics[1].c_str(), 3, "0;1", qos, retain);//spat' 	
			mosquitto_publish(client, mid, topics[3].c_str(), 1, "2", qos, retain);//perehod manual
			toManual = 0;
		}
		else
		{
			toManual++;
			mosquitto_publish(client, mid, topics[1].c_str(), 4, rotate, qos, retain);
			mosquitto_publish(client, mid, topics[1].c_str(), 3, "0;1", qos, retain);
		}
	}
	delete[] rotate;
}

void Operator::SendCommand(const std::string& topic, uint8_t* message, int size)
{
	mosquitto_publish(client, nullptr, topic.c_str(), size, message, qos, retain);
}

sf::Image Operator::ParsePic()
{
	sf::Image image;									//если я работаю с функциями и классами картинок, то при входе в ф-ю вызывается конструктор, 
	std::vector<std::vector<cv::Point>> contours;
	sf::Image result;
	if (image.loadFromMemory(recieved.payload, recieved.payloadlen))	//а при выходе из функции вызывается деструктор => в динамическом выделении нет смысла...........
	{																	//опять всё менять?..............................................................................
		result = DefineGreen(image, contours);							//поменять названия - finObj?

		if (CurrentBotState() == Commands::END_OF_STATE_SEQUENCE)
		{
			bool found = (contours.size() == 1);
			int angle = found
				? GetAngle(GetCentralPoint(contours, image.getSize().x))
				: 60;
			SendSequenceCommands(found, angle);
		}

		recieved.payload = nullptr;
	}
	return result;
}

void Operator::ParseCommand()
{
	int ptrCommand;// = new int;
	ptrCommand = *static_cast<int*>(recieved.payload);
	botState = static_cast<Commands>(ptrCommand);
	//delete ptrCommand;
}

Commands Operator::CurrentBotState()
{
	return botState;
}
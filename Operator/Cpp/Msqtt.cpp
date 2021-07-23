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
	std::cout << "The message has been published" << std::endl;
}

void OnSubscribe(struct mosquitto* mosc, void* obj, int mid, int qos_count, const int* granted_qos)
{
	std::cout << "aoaoaoa subscribe!!!! aoaoao" << std::endl;
}

void OnMessage(struct mosquitto* mosc, void* obj, const struct mosquitto_message* message)
{
	mosquitto_message_copy(&recieved, message);
}

void OnDisconnect(struct mosquitto* mosc, void* obj, int rc)
{
	std::cout << ">> MQTT disconnected " << rc << std::endl;
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
			: false;

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
	botState = Commands::DEFAULT;
	toManual = 0;
}

void Operator::SendSequenceCommands(bool found, int angle)
{
	int* mid = nullptr;
	int size = 3;
	if (angle < 0) size++;
	if (std::abs(angle) > 9) size++;
	if (std::abs(angle) > 99) size++;

	uint8_t* rotate = new uint8_t[size];
	rotate[0] = '4';
	rotate[1] = ';';

	int i = 2;
	if (angle < 0)
	{
		rotate[i] = '-';
		i++;
	}

	for (int j = size - 1; j >= i; j--)
	{
		rotate[j] = std::abs(angle) % 10 + '0';
		angle /= 10;
	}

	if (found)
	{
		if (toManual) toManual = 0;

		mosquitto_publish(client, mid, topics[1].c_str(), size, rotate, qos, retain);
		mosquitto_publish(client, mid, topics[1].c_str(), 3, "0;1", qos, retain);
		mosquitto_publish(client, mid, topics[1].c_str(), 3, "3;1", qos, retain);
		mosquitto_publish(client, mid, topics[1].c_str(), 3, "0;1", qos, retain);
	}
	else
	{
		if (toManual == 10)
		{
			mosquitto_publish(client, mid, topics[1].c_str(), 3, "0;1", qos, retain); 	
			mosquitto_publish(client, mid, topics[3].c_str(), 1, "2", qos, retain);
			toManual = 0;
			mode = ControlType::MANUAL;
		}
		else
		{
			toManual++;
			mosquitto_publish(client, mid, topics[1].c_str(), 4, rotate, qos, retain);
			mosquitto_publish(client, mid, topics[1].c_str(), 3, "0;1", qos, retain);
		}
	}
	botState = Commands::IN_SEQUENCE;
	delete[] rotate;
}

void Operator::SendCommand(const std::string& topic, uint8_t* message, int size)
{
	mosquitto_publish(client, nullptr, topic.c_str(), size, message, qos, retain);
}

sf::Image Operator::ParsePic()
{
	sf::Image image;
	std::vector<std::vector<cv::Point>> contours;

	sf::Image result;
	if (image.loadFromMemory(recieved.payload, recieved.payloadlen))
	{
		FindingStates FindState = state;
		bool found;
		int angle;

		for (int i = 0; i <= static_cast<int>(FindState); i++)
		{
			result = DefineObj(image, contours, static_cast<FindingStates>(i));
			found = (contours.size() == 1);
			angle = found
				? GetAngle(GetCentralPoint(contours, image.getSize()).x)
				: 60;

			if (found && EndOfFind(static_cast<FindingStates>(i), contours, image.getSize()))
			{
				state = static_cast<FindingStates>(static_cast<int>(i) + 1);
			}
			else
			{
				state = static_cast<FindingStates>(i);
			}
			contours.clear();
		}

		if (FindState == FindingStates::FOUND_ALL)
		{
			uint8_t allFonud[] = { '0', ';', '0' };
			SendCommand(topics[1], allFonud, 3);
		}

		else if (CurrentBotState() == Commands::END_OF_STATE_SEQUENCE)
		{
			SendSequenceCommands(found, angle);
		}
	}
	return result;
}

void Operator::ParseCommand()
{
	uint8_t* ptrCommand = static_cast<uint8_t*>(recieved.payload);
	botState = static_cast<Commands>((*ptrCommand) - '0');
}

Commands Operator::CurrentBotState()
{
	return botState;
}

void Operator::SetState(FindingStates state_)
{
	state = state_;
}

void Operator::SetMode(ControlType type)
{
	mode = type;
}

FindingStates Operator::GetState() const
{
	return state;
}

ControlType Operator::GetMode() const
{
	return mode;
}

bool Operator::EndOfFind(FindingStates state_, const std::vector<std::vector<cv::Point>>& contours, const sf::Vector2u& imgSize)
{
	if (state_ == FindingStates::FINDING_PUCK)
	{
		return (GetCentralPoint(contours, imgSize).y > 0.9);
	}
	else if (state_ == FindingStates::FINDING_GOAL)
	{
		cv::Rect2f rect = cv::boundingRect(contours[0]);
		return (static_cast<float>(rect.area()) / static_cast<float>(imgSize.x * imgSize.y) > 0.2);
	}
}
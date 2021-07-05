#include "mainwindow.h"

#include <QApplication>
#include <QWidget>
#include <QIcon>
#include <mosquitto.h>
#include <string>
#include <thread>
#include <chrono>

#include "callbacks.h"

void Moqquitto_works(){
	try {
		errno = 0;
		mosquitto* mosq = nullptr;

		mosquitto_lib_init();

		mosq = mosquitto_new("client1", 0, nullptr);
		if (mosq){
			mosquitto_connect_callback_set(mosq, OnConnect);
			mosquitto_publish_callback_set(mosq, OnPublish);
			mosquitto_subscribe_callback_set(mosq, OnSubscribe);
			mosquitto_message_callback_set(mosq, OnMessage);
			mosquitto_disconnect_callback_set(mosq, OnDisconnect);

			mosquitto_connect(mosq, "5.145.237.181", 1883, 60);
			int mid0 = 54321;
			mosquitto_subscribe(mosq, &mid0, "#", 2);
			int mid = 12345;
			const char* msg = "hihihi";
			while(!mosquitto_loop(mosq, 60, 1000))
			{
				mosquitto_publish(mosq, &mid, "/image", sizeof (msg), msg, 2, 1);
				std::this_thread::sleep_for(std::chrono::seconds(20));
			}

			mosquitto_destroy(mosq);
			mosquitto_lib_cleanup();
		} else {
			errno_t err_num = errno;
			std::cerr << strerror(err_num) << std::endl;
		}
	}  catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainWindow w;
	w.show();
	std::thread th(Moqquitto_works);
	th.detach();
	return a.exec();
}

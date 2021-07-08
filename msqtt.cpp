#ifndef MOSQUITTO_CPP
#define MOSQUITTO_CPP

#include "msqtt.h"
#include <QFile>

QPixmap MosquittoWorks(){
	try {
		errno = 0;
		mosquitto* mosq = nullptr;

		mosquitto_lib_init();
/*
		mosq = mosquitto_new("client1", 0, nullptr);
		if (mosq){
			mosquitto_connect_callback_set(mosq, OnConnect);
			mosquitto_publish_callback_set(mosq, OnPublish);
			mosquitto_subscribe_callback_set(mosq, OnSubscribe);
			mosquitto_message_callback_set(mosq, OnMessage);
			mosquitto_disconnect_callback_set(mosq, OnDisconnect);

			mosquitto_connect(mosq, "127.0.0.1", 1883, 60);//"5.145.237.181"
			int mid0 = 5;
			*/
			int a = mosquitto_subscribe_simple(&recieved, 1, false, "#", 2, "127.0.0.1", 1883, "client1", 60, true, NULL, NULL, NULL, NULL);
			std::cout << "a = " << a << std::endl;
			std::cout << recieved->payloadlen << std::endl;
			QByteArray *ba = static_cast<QByteArray*>(recieved->payload);
			if (ba == NULL){std::cout << "aaaa" << std::endl;}
			QByteArray ba2 = *ba;
			int n = ba2.size();
			std::cout << (n == recieved->payloadlen) << std::endl;
			std::cout << n << std::endl;
			//if(!ba->isNull() && !ba->isEmpty()){
		//		for (int i = 0; i != n; i++){

				//	std::cout << ba->data()[n] << std::endl;
	//			}
//			}

//			while(n){
	//			n--;
		//		std::cout << n << " : " << ba.data()[n] << std::endl;
			//}
			QPixmap mp;
//			std::cout << "still alive" << std::endl;
//			std::cout << mp.loadFromData(ba, "JPG") << std::endl;
//			std::cout << "still alive" << std::endl;
	//		QImage qImg ((uchar*)ba.data(), 100, 75, QImage::Format_RGBA64);
		//	mp = QPixmap::fromImage(qImg);
		//	QFile f("C:/Users/MI/QtProject/bot_HMI/robo.jpg");
			//f.open(QIODevice::WriteOnly);
			//mp.save(&f, "JPG");
			return mp;
/*			int mid = 1;
		//	QImage picture("C:/Users/MI/QtProject/bot_HMI/robot.jpg");

			const char* msg = "hihihi";

			mosquitto_publish(mosq, &mid, "/image", sizeof(msg), msg, 2, 1);

			while(!mosquitto_loop(mosq, 60, 1000))
			{
				std::this_thread::sleep_for(std::chrono::seconds(5));
			}

			mosquitto_destroy(mosq);*/
			mosquitto_lib_cleanup();/*
		} else {
			errno_t err_num = errno;
			std::cerr << strerror(err_num) << std::endl;
		}*/
	}  catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}


#endif // MOSQUITTO_CPP

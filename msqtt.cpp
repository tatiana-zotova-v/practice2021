#ifndef MOSQUITTO_CPP
#define MOSQUITTO_CPP

#include "msqtt.h"
#include <QFile>

QPixmap MosquittoWorks(){
	try {
		errno = 0;

		mosquitto_lib_init();

		int mid0 = 5;

		int a = mosquitto_subscribe_simple(&recieved, 1, false, "#", 2, "127.0.0.1", 1883, "client1", 60, true, NULL, NULL, NULL, NULL);

		std::cout << "a = " << a << std::endl;
		std::cout << recieved->payloadlen << std::endl;

		QByteArray *ba = static_cast<QByteArray*>(recieved->payload);

		if (ba == NULL){std::cout << "aaaa" << std::endl;}

		QByteArray ba2 = *ba;
		int n = ba2.size();

		std::cout << (n == recieved->payloadlen) << std::endl;
		std::cout << n << std::endl;

		QPixmap mp;

//		std::cout << "still alive" << std::endl;
//		std::cout << mp.loadFromData(ba, "JPG") << std::endl;
//		std::cout << "still alive" << std::endl;
//		QImage qImg ((uchar*)ba.data(), 100, 75, QImage::Format_RGBA64);
//		mp = QPixmap::fromImage(qImg);

		mosquitto_lib_cleanup();

	}  catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}
}


#endif // MOSQUITTO_CPP

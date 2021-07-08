#include "mainwindow.h"
#include "mosquitto.h"
#include <iostream>
#include "callbacks.h"

#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QBuffer>
#include <thread>
#include <chrono>

void MqttWorks(mosquitto_message &msg){
    try {
        errno = 0;
        mosquitto* mosq = nullptr;

        mosquitto_lib_init();

        mosq = mosquitto_new("client2", 0, nullptr);
        if (mosq){
            mosquitto_connect_callback_set(mosq, OnConnect);
            mosquitto_publish_callback_set(mosq, OnPublish);
            mosquitto_subscribe_callback_set(mosq, OnSubscribe);
            mosquitto_message_callback_set(mosq, OnMessage);
            mosquitto_disconnect_callback_set(mosq, OnDisconnect);

            mosquitto_connect(mosq, "127.0.0.1", 1883, 60);
            mosquitto_loop(mosq, 60, 1000);
            int mid = 54321;
            mosquitto_subscribe(mosq, &mid, "/image", 2);
            QImage img("C:/Users/MI/QtProject/bot_HMI/robot.jpg");

            std::cout << img.width() << "    " << img.height() << std::endl;
            std::cout << img.sizeInBytes() << std::endl;

            QPixmap pm = QPixmap::fromImage(img);

            std::cout << pm.isNull() << std::endl;

            QByteArray *ba = new QByteArray;
            QBuffer b(ba);
            b.open(QIODevice::WriteOnly);

            std::cout << pm.save(&b, "JPG") << std::endl;
            std::cout << ba->isNull() << "   " << ba->isEmpty() << std::endl;
            std::cout << ba->size() << std::endl;
            std::cout << sizeof(ba) << std::endl;

            mosquitto_publish(mosq, &mid, "/image", ba->size(), ba, 2, 1);

            std::cout << "still alive" << std::endl;

            while(!mosquitto_loop(mosq, 60, 1000))
            {
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

    std::thread th(MqttWorks, std::ref(recieved));
    MainWindow w;
    w.show();

    return a.exec();
}

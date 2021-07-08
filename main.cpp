#include "mainwindow.h"

#include <QApplication>
#include <QWidget>
#include <QIcon>
#include <QImage>
#include <string>

#include "msqtt.h"

extern struct mosquitto_message* recieved;

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QPixmap pm;
	std::thread th(MosquittoWorks);
	MainWindow w;
//	while(!pm){}
//	w.SetPic(pm);
	w.show();

	//th.join();
	return a.exec();
}

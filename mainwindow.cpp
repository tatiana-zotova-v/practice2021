#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	setWindowIcon(QIcon("bot.png")); //opyat' ne rabotaet (v kocnstruktore toje)
	SetTheme();
}

void MainWindow::SetTheme()
{
	QPixmap pic("C:/Users/MI/QtProject/bot_HMI/hqdefault.jpg");
	ui->label->setPixmap(pic);
	ui->centralwidget->setStyleSheet("QLabel {"
									 "border: 2px;"
									 "border-radius: 4px;"
									 "padding: 2px;"
									 "background-color: rgba(232,233,242,255);"
									 "}"
									 //"QLabel{border-color: rgba(39,49,106,255);}" - does not working
									 "QPushButton{"
									 "border: 1px solid transparent;text-align: center;"
									 "color:rgba(255,255,255,255);"
									 "border-radius: 8px;"
									 "border-width: 3px;"
									 "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,"
									 "stop: 0 rgba(64, 97, 190, 200), stop: 1 rgba(188,194,230,255));"
									 "max-width: 35px;"
									 "max-height: 25px;}"
									 "QPushButton {"
									 "border-color: rgba(121, 145, 210, 255);}"
									 "QPushButton:pressed {"
									 "background-color: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 1,"
									 "stop: 0 rgba(44, 67, 130, 200), stop: 1 rgba(168, 176, 223, 255));}"
									 );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_move_forward_clicked()
{

}

void MainWindow::on_brake_clicked()
{

}

void MainWindow::on_right_rotate_clicked()
{

}

void MainWindow::on_move_backward_clicked()
{

}

void MainWindow::on_left_rotate_clicked()
{

}

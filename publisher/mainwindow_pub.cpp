#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <mosquitto.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

void MainWindow::SetPic(QImage img){
    ui->label->setPixmap(
                QPixmap::fromImage(img).scaled(
                ui->label->size(),
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation)
                );
}

MainWindow::~MainWindow()
{
    delete ui;
}

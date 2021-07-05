#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma once
#include <QMainWindow>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
	void SetTheme();
    ~MainWindow();

private slots:
    void on_move_forward_clicked();

    void on_brake_clicked();

    void on_right_rotate_clicked();

    void on_move_backward_clicked();

    void on_left_rotate_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QFile>
#include <QtXml>
#include <QDateTime>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

#include "Thread.h"
#include "Serial.h"

extern "C"
{
#include "LPW_Encryptor.h"
}

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();

private slots:

    void on_ConnectButton_clicked();

    void on_DisconnectButton_clicked();

    void on_Key1_clicked();

    void on_Key2_clicked();

    void on_Key3_clicked();

    void on_Key4_clicked();

    void on_Key5_clicked();

    void on_Key6_clicked();

    void on_KeyUp_clicked();

    void on_Key7_clicked();

    void on_Key8_clicked();

    void on_Key9_clicked();

    void on_Key0_clicked();

    void on_KeyStar_clicked();

    void on_KeySharp_clicked();

    void on_KeyBackspace_clicked();

    void on_KeyDown_clicked();

    void on_KeyOK_clicked();

    void on_PathButton_clicked();

    void on_WorkingModeComboBox_currentIndexChanged(int index);

    void on_OuchButton_clicked(bool checked);

    void on_Instructions(QString ins);

private:

    Ui::MainWindow *ui;

    Thread *WorkingThread;
};

#endif // MAINWINDOW_H

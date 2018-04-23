#ifndef SERIAL_H
#define SERIAL_H

#include "ui_mainwindow.h"

#include <QDebug>

#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

class Serial
{
public:

    static Serial* Instance();

    int InitSerial(Ui::MainWindow *ui);

    int OpenSerial(QString port);

    int CloseSerial();

    void SendQuickResponseMessage(char content);

    void SendInstructions(char* content, int length);

protected:

    Serial();

private:

    QString PortName;

    static Serial* pThis;

    static Serial* _instance;

    QSerialPort *serial;
};


#endif // SERIAL_H

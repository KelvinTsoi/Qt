#include "Serial.h"

Serial* Serial::_instance = NULL;

Serial* Serial::pThis = NULL;

Serial::Serial()
{
    pThis = this;
}

Serial* Serial::Instance()
{
    if (_instance == 0)
    {
        _instance = new Serial();
    }
    return _instance;
}

int Serial::InitSerial(Ui::MainWindow *ui)
{
    //查找可用的串口
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        QSerialPort tmpserial;
        tmpserial.setPort(info);
        if(tmpserial.open(QIODevice::ReadWrite))
        {
            ui->ComComboBox->addItem(tmpserial.portName());
            tmpserial.close();
        }
    }

    return 0;
}

int Serial::OpenSerial(QString port)
{
    serial = new QSerialPort;

    //设置串口名
    serial->setPortName(port);

    //打开串口
    if(serial->open(QIODevice::ReadWrite))
    {
        //设置波特率
        serial->setBaudRate(115200);

        //设置数据位数
        serial->setDataBits(QSerialPort::Data8);

        //设置奇偶校验
        serial->setParity(QSerialPort::NoParity);

        //设置停止位
        serial->setStopBits(QSerialPort::OneStop);

        //设置流控制
        serial->setFlowControl(QSerialPort::NoFlowControl);

        qDebug() << "Open Serial Port Success" << endl;
        return 0;
    }
    else
    {
        qDebug() << "Open Serial Port Failed" << endl;
        return 1;
    }
}

int Serial::CloseSerial()
{
    //关闭串口
    serial->clear();
    serial->close();
    serial->deleteLater();
}

void Serial::SendQuickResponseMessage(char content)
{
    char msg[3] = {0x00};
    msg[0] = 'B';
    msg[1] = content;
    msg[2] = 'E';
    serial->write(msg, 3);
}

void Serial::SendInstructions(char* content, int length)
{
    char* msg = NULL;
    msg = (char*)malloc((length+2)*sizeof(char));
    msg[0] = 'A';
    for(int i = 1, j = 0; j < length; i++, j++)
    {
        msg[i] = content[j];
    }
    msg[length+2-1] = 'E';
    serial->write(msg, length+2);
    free(msg);
}

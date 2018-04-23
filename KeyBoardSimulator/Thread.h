#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include "QThread"
#include <QFileDialog>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QFile>
#include <QtXml>
#include <QDateTime>

#include "Serial.h"

extern "C"
{
#include "LPW_Encryptor.h"
}


class Thread : public QThread
{
    Q_OBJECT
public:

    Thread();

    void ThreadStart(QString phone, QString dir);

    void ThreadStop();

    bool ThreadState();

protected:

    void run();

private:

    int CipherTextCalculation(QString PhoneNumber, QString directory);

    int GetKeyConfigure(QString directory);

    int ParseKeyContent(QXmlStreamReader &reader);

signals:

    void ThreadSignal(QString instruction);

private:

    volatile bool enable;

    QString PhoneNumber;

    QString Directory;
};

#endif // WORKTHREAD_H

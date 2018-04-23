#include "Thread.h"

Thread::Thread()
{
    enable = false;
}

void Thread::ThreadStop()
{
    enable = false;
}

void Thread::ThreadStart(QString phone, QString dir)
{
    //Get Phone Number & XML Directory
    PhoneNumber = phone;
    Directory = dir;



    enable = true;
}

bool Thread::ThreadState()
{
    return enable;
}

void Thread::run()
{
    qDebug() << "Thread is on board" << endl;

    while(true)
    {
        if(enable)
        {
            QString PasswordStr;

            int Password = CipherTextCalculation(PhoneNumber, Directory);

            PasswordStr.sprintf("*%06d", Password);

            emit ThreadSignal(PasswordStr);

            sleep(5);
        }
    }
}

int Thread::CipherTextCalculation(QString phone, QString dir)
{
    bool ok = false;

    if(phone.length() != 11)
    {
        return -1;
    }

    int cipherKey = GetKeyConfigure(dir);                         //密码密钥
    int cipherText = 0;                                           //密码密文
    int plainText = phone.right(5).toInt(&ok, 10) * 10;           //密码明文

    cipherText = LPW_Encrypt(plainText, cipherKey);
    if(cipherText == -1 || cipherText == -2)
    {
        return -2;
    }
    else
    {
        return cipherText;
    }
}

int Thread::GetKeyConfigure(QString dir)
{
    //只读模式打开XML文件
    QFile file(dir);
    if (file.open(QFile::ReadOnly))
    {
        QXmlStreamReader reader(&file);
        while (!reader.atEnd())
        {
            //读取下一个元素
            QXmlStreamReader::TokenType nType = reader.readNext();
            switch (nType)
            {
                //开始元素
                case QXmlStreamReader::StartElement:
                {
                    QString strElementName = reader.name().toString();

                     //根元素
                    if (QString::compare(strElementName, "message") == 0)
                    {
                        //解析内容
                        file.close();
                        return ParseKeyContent(reader);
                    }
                    break;
                }

                //结束文档
                case QXmlStreamReader::EndDocument:
                {
                    break;
                }
            }
        }

    }
    else
    {
        return -1;
    }
}

int Thread::ParseKeyContent(QXmlStreamReader &reader)
{
    int ret = 0;
    bool ok = false;

    while (!reader.atEnd())
    {
        reader.readNext();

        //开始元素
        if (reader.isStartElement())
        {
            QString strElementName = reader.name().toString();
            if (QString::compare(strElementName, "date") == 0)
            {
                QXmlStreamAttributes attributesAlfa = reader.attributes();
                if (attributesAlfa.hasAttribute("d"))
                {
                    //匹对当前日期
                    if((ret = QString::compare(attributesAlfa.value("d").toString(), QDateTime::currentDateTime().toString("yyyy-MM-dd"))) == 0)
                    {
                        while (!reader.atEnd())
                        {
                            reader.readNext();
                            QString strElementNamer = reader.name().toString();
                            if (QString::compare(strElementNamer, "time") == 0 )
                            {
                                QXmlStreamAttributes attributesBeta = reader.attributes();
                                if (attributesBeta.hasAttribute("t"))
                                {
                                    if((ret = QString::compare(attributesBeta.value("t").toString(), QDateTime::currentDateTime().toString("h"))) == 0)
                                    {
                                       return QString("%1").arg(reader.readElementText()).toInt(&ok,10);
                                    }
                                }
                            }
                        }
                        return -1;
                    }
                }
            }
        }
    }
    return -2;
}

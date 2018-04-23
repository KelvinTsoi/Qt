#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /* IP格式正则表达式 */
    QRegExp regxIpFormat("((2[0-4]\\d|25[0-5]|[01]?\\d\\d?)\\.){3}(2[0-4]\\d|25[0-5]|[01]?\\d\\d?)");
    QValidator *validatorIpFormat = new QRegExpValidator(regxIpFormat,ui->txt_ip);
    ui->txt_ip->setValidator(validatorIpFormat);

    /* 端口号格式正则表达式 */
    //ui->txt_port->setValidator(new QIntValidator(0,65535,ui->txt_port));
    ui->txt_port->setEnabled(false);
    ui->txt_port_native->setEnabled(false);

    /* 位置编码有效长度限制 */
    ui->txt_deviceid->setMaxLength(40);

    /* 楼层编号有效范围 */
    ui->txt_basefloor->setValidator(new QIntValidator(-2,100,ui->txt_basefloor));
    ui->txt_toroom_l->setValidator(new QIntValidator(-2,100,ui->txt_toroom_l));

    /* 卡号有效长度 */
    ui->txt_cardnum->setMaxLength(19);

    bool ok;

    //固定窗口大小
    Qt::WindowFlags flags = 0;
    flags |= Qt::WindowMinimizeButtonHint;
    setFixedSize(760,524);


    //控制台默认设置GoHome报文数据，因此默认禁止控制台上所有LeaveHome信息的设置
    ui->txt_home_l->setEnabled(false);
    ui->txt_toroom_l->setEnabled(false);
    ui->box_triggertype_l->setEnabled(false);
    ui->button_leavehome_request->setEnabled(false);

    //初始化socket
    udp_socket_tx = new QUdpSocket(this);
    udp_socket_rx = new QUdpSocket(this);
    udp_socket_tx->bind(QHostAddress::Any, ui->txt_port_native->text().toInt(&ok));
    udp_socket_rx->bind(QHostAddress::Any, ui->txt_port->text().toInt(&ok));

    //初始化消息请求方的流水号
    sequent = 0;

    //绑定接收信号槽
    connect(udp_socket_rx, SIGNAL(readyRead()),this, SLOT(rx_udp()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_box_cmd_currentIndexChanged(const QString &arg1)
{
    if(arg1 == "GoHome")
    {
        //控制台GoHome报文数据
        ui->txt_basefloor->setEnabled(true);
        ui->txt_home->setEnabled(true);
        ui->box_triggertype->setEnabled(true);
        ui->txt_cardnum->setEnabled(true);
        ui->box_readernum->setEnabled(true);
        ui->box_unlock->setEnabled(true);
        ui->box_locknum->setEnabled(true);
        ui->button_gohome_request->setEnabled(true);

        //控制台LeaveHome报文数据
        ui->txt_home_l->setEnabled(false);
        ui->txt_toroom_l->setEnabled(false);
        ui->box_triggertype_l->setEnabled(false);
        ui->button_leavehome_request->setEnabled(false);
    }
    else if(arg1 == "LeaveHome")
    {
        //控制台LeaveHome报文数据
        ui->txt_home_l->setEnabled(true);
        ui->txt_toroom_l->setEnabled(true);
        ui->box_triggertype_l->setEnabled(true);
        ui->button_leavehome_request->setEnabled(true);

        //控制台GoHome报文数据
        ui->txt_basefloor->setEnabled(false);
        ui->txt_home->setEnabled(false);
        ui->box_triggertype->setEnabled(false);
        ui->txt_cardnum->setEnabled(false);
        ui->box_readernum->setEnabled(false);
        ui->box_unlock->setEnabled(false);
        ui->box_locknum->setEnabled(false);
        ui->button_gohome_request->setEnabled(false);
    }
}

void MainWindow::on_box_triggertype_currentIndexChanged(const QString &arg1)
{
    if(arg1 == "1->用户刷住户卡" || arg1 == "7->保安刷保安卡")
    {
        ui->txt_cardnum->setEnabled(true);
        ui->box_readernum->setEnabled(true);
    }
    else
    {
        ui->txt_cardnum->setEnabled(false);
        ui->box_readernum->setEnabled(false);
    }
}

void MainWindow::on_box_unlock_currentIndexChanged(const QString &arg1)
{
    if(arg1 == "0->不开启")
        ui->box_locknum->setEnabled(false);
    else
        ui->box_locknum->setEnabled(true);
}

void MainWindow::on_button_refresh_clicked()
{
    ui->txt_rx->setPlainText("");
}

int MainWindow::check_ip()
{
    bool ret = false;

    //检测当前网络环境中，控制台输入的IP地址是否属于任何网络适配器所配置的IP地址其中之一
    QString localhostName = QHostInfo::localHostName();
    QHostInfo info = QHostInfo::fromName(localhostName);
    foreach(QHostAddress address,info.addresses())
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol)
        {
            if(QHostAddress(ui->txt_ip->text()) == address)
                ret = true;
        }
    }

    if(!ret)
    {
        QMessageBox msgBox;
        msgBox.setText("本机不存在该IP地址，请确认后重新输入！");
        msgBox.exec();
        return ERROR;
    }

    return SUCCESS;
}

int MainWindow::check_required()
{
    QString carrier;

    carrier = ui->txt_ver->text();
    if(carrier.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("请输入协议版本号！");
        msgBox.exec();
        return ERROR;
    }

    carrier = ui->txt_password->text();
    if(carrier.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("请输入通讯密码！");
        msgBox.exec();
        return ERROR;
    }

    carrier = ui->txt_deviceid->text();
    if(carrier.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("请输入设备位置编码！");
        msgBox.exec();
        return ERROR;
    }

    return SUCCESS;
}

int MainWindow::check_GoHome_input()
{
    QString carrier;

    carrier = ui->txt_basefloor->text();
    if(carrier.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("请输入业主当前所在楼层！");
        msgBox.exec();
        return ERROR;
    }

    carrier = ui->txt_home->text();
    if(carrier.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("请输入业主家的楼层房号！");
        msgBox.exec();
        return ERROR;
    }

    //主楼层房号数据约束规则
    char* muti_home;
    char tmpCheck[1024];
    int roomCount = 0;
    QByteArray tpMutiHome = ui->txt_home->text().toLatin1();
    muti_home = tpMutiHome.data();
    int zz = 0;
    char* ptr;
    char* p;
    ptr = strtok_r(muti_home, HOME_SPLIT, &p);
    while (ptr != NULL)
    {
        memset(tmpCheck, 0x00, sizeof(tmpCheck));
        sscanf(ptr, "%[^;]", tmpCheck);

        int splitStringLoop = 0;
        int commCheck = 0;
        while(tmpCheck[splitStringLoop] != NULL)
        {
            if(strlen(tmpCheck) > 10)
            {
                QMessageBox msgBox;
                msgBox.setText("检测到业主楼层房号数据超出限制值，请重新检查数据！");
                msgBox.exec();
                return ERROR;
            }

            if(roomCount > 10)
            {
                QMessageBox msgBox;
                msgBox.setText("检测到业主套间数量超出限制值，请重新检查数据！");
                msgBox.exec();
                return ERROR;
            }

            if( !(tmpCheck[splitStringLoop] == 44 || (tmpCheck[splitStringLoop] >= 48 && tmpCheck[splitStringLoop] <= 57)) || (commCheck > 2) || (tmpCheck[0] == 44) )
            {
                QMessageBox msgBox;
                msgBox.setText("检测到错误的输入格式, 请按照格式输入业主楼层房号！\n格式：\n   单户:楼层,房号\n   多户:楼层1,房号1;楼层2,房号2;...");
                msgBox.exec();
                return ERROR;
            }

            if(tmpCheck[splitStringLoop] == 44)
                commCheck++;

            splitStringLoop++;
        }

        if(commCheck == 0)
        {
            QMessageBox msgBox;
            msgBox.setText("检测到错误的输入格式, 请按照格式输入业主楼层房号！\n格式：\n   单户:楼层,房号\n   多户:楼层1,房号1;楼层2,房号2;...");
            msgBox.exec();
            return ERROR;
        }

        if(splitStringLoop != 0)
        {
            if(tmpCheck[splitStringLoop - 1] == 44)
            {
                QMessageBox msgBox;
                msgBox.setText("检测到错误的输入格式, 请按照格式输入业主楼层房号！\n格式：\n   单户:楼层,房号\n   多户:楼层1,房号1;楼层2,房号2;...");
                msgBox.exec();
                return ERROR;
            }
        }

        ptr = strtok_r(NULL, HOME_SPLIT, &p);
        zz++;
        roomCount++;
    }

    if(ui->box_triggertype->currentIndex()== 0 || ui->box_triggertype->currentIndex() == 6)
    {
        carrier = ui->txt_cardnum->text();
        if(carrier.isEmpty())
        {
            QMessageBox msgBox;
            msgBox.setText("请输入卡号！");
            msgBox.exec();
            return ERROR;
        }
    }

    return SUCCESS;
}

int MainWindow::check_LeaveHome_input()
{
    QString carrier;

    carrier = ui->txt_home_l->text();
    if(carrier.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("请输入业主家的楼层房号！");
        msgBox.exec();
        return ERROR;
    }

    carrier = ui->txt_toroom_l->text();
    if(carrier.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("请输入业主去往楼层！");
        msgBox.exec();
        return ERROR;
    }

    char* single_home;
    char tmpCheck[1024];
    memset(tmpCheck, 0x00, sizeof(tmpCheck));
    QByteArray tpSingleHome = ui->txt_home_l->text().toLatin1();
    single_home = tpSingleHome.data();
    strcpy(tmpCheck, single_home);

    int splitStringLoop = 0;
    int commCheck = 0;
    while(tmpCheck[splitStringLoop] != NULL)
    {
        if(strlen(tmpCheck) > 10)
        {
            QMessageBox msgBox;
            msgBox.setText("检测到业主楼层房号数据超出限制值，请重新检查数据！");
            msgBox.exec();
            return ERROR;
        }

        if( !(tmpCheck[splitStringLoop] == 44 || (tmpCheck[splitStringLoop] >= 48 && tmpCheck[splitStringLoop] <= 57)) || (commCheck > 1) || (tmpCheck[0] == 44) )
        {
            QMessageBox msgBox;
            msgBox.setText("检测到错误的输入格式, 请按照格式输入业主楼层房号！\n格式：\n   单户:楼层,房号\n");
            msgBox.exec();
            return ERROR;
        }

        if(tmpCheck[splitStringLoop] == 44)
            commCheck++;

        splitStringLoop++;
    }

    if(commCheck == 0)
    {
        QMessageBox msgBox;
        msgBox.setText("检测到错误的输入格式, 请按照格式输入业主楼层房号！\n格式：\n   单户:楼层,房号\n");
        msgBox.exec();
        return ERROR;
    }

    if(splitStringLoop != 0)
    {
        if(tmpCheck[splitStringLoop - 1] == 44)
        {
            QMessageBox msgBox;
            msgBox.setText("检测到错误的输入格式, 请按照格式输入业主楼层房号！\n格式：\n   单户:楼层,房号\n");
            msgBox.exec();
            return ERROR;
        }
    }

    return SUCCESS;
}

int MainWindow::get_time()
{
    long long time_last;
    time_last = time(NULL);
    return (int)time_last;
}

int MainWindow::get_rand()
{
    float min = 1.0;
    float max = 9999.0;
    srand((int) time(0));
    return 1 + (int) (max * rand() / (RAND_MAX + min));
}

int MainWindow::generate_CheckCode(char src[], char dst[])
{
    //获取MD5值
    unsigned char md5Value[16];
    memset(md5Value, 0x00, sizeof(md5Value));
    MD5_CTX mdContext;
    MD5Init(&mdContext);
    MD5Update(&mdContext, (unsigned char *)src, strlen(src));
    MD5Final(&mdContext, md5Value);

    //由unsigned char中的16进制字面值转换为相应的字符串字面值
    for(unsigned int i = 0; i < 16; i++)
        sprintf(&dst[2*i], "%2.2X", md5Value[i]);

    return SUCCESS;
}

void MainWindow::rx_udp()
{
    //等待即将来临的udp数据包
    while (udp_socket_rx->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udp_socket_rx->pendingDatagramSize());

        QHostAddress sender;
        quint16 senderPort;

        udp_socket_rx->readDatagram(datagram.data(), datagram.size(),&sender, &senderPort);

        //提取socket中的ip地址,用于校验数据包,
        //消息的来源IP地址和消息里声明的FromIp参数比对,不一致,则说明该消息是第三方重发的
        QString get_source_address = sender.toString();
        QStringList splite_result = get_source_address.split(':');
        if(splite_result.size() != 0)
            package_source = splite_result.at(splite_result.size()-1);

        //计算当前接收数据时间
        char timeStr[MIN_SIZE];
        struct tm *p;
        time(&time_of_arrival);
        p = gmtime(&time_of_arrival);
        memset(timeStr, 0, MIN_SIZE);
        sprintf(timeStr, "[ %d-%02d-%02d %02d:%02d:%02d ]", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday, 8 + p->tm_hour, p->tm_min, p->tm_sec);
        QString timeQStr;
        timeQStr = QString(QLatin1String(timeStr));

        //若检测后报文正确，则回显到消息监控控件中
        if(check_package(datagram) != ERROR)
        {
            ui->txt_rx->append("-----Receive Respone-----\nTime: ");
            ui->txt_rx->insertPlainText(timeQStr);
            ui->txt_rx->moveCursor(QTextCursor::End);
            ui->txt_rx->append("Source: ");
            ui->txt_rx->insertPlainText(package_source);
            ui->txt_rx->moveCursor(QTextCursor::End);
            ui->txt_rx->append("\n-----Message Content List-----");
            ui->txt_rx->append(datagram);
            ui->txt_rx->append("\n");
        }
    }
}

int MainWindow::check_package(QString rev)
{
    bool statusCodeExist = false;
    bool ipAddrcheck     = false;
    bool packTimeOut     = false;

    char* tmp;
    QByteArray tp = rev.toLatin1();
    tmp = tp.data();

    char msg[MAX_SIZE];
    memset(msg, 0x00, MAX_SIZE);

    for(unsigned int i = 0; i < strlen(tmp); i++)
        msg[i] = tmp[i];

    char* ptr;
    char* p;
    ptr = strtok_r(msg, SPLIT, &p);

    while (ptr != NULL)
    {
        char bufHead[DATA_HEAD_SIZE];
        char bufTail[DATA_TAIL_SIZE];
        memset(bufHead, 0x00, DATA_HEAD_SIZE);
        memset(bufTail, 0x00, DATA_TAIL_SIZE);
        sscanf(ptr, "%[^:]", bufHead);

        //检测是否存在StatusCode字段
        if (strcmp(bufHead, "StatusCode") == 0)
            statusCodeExist = true;
        //检测报文中的FromIP字段值是否与接收到udp报文的来源地址一致
        else if(strcmp(bufHead, "FromIP") == 0)
        {
            sscanf(ptr, "%*[^:]:%s", bufTail);
            char* tmpAddr;
            QByteArray tpAddr = package_source.toLatin1();
            tmpAddr = tpAddr.data();
            if(strcmp(bufTail, tmpAddr) == 0)
                ipAddrcheck = true;
        }
        //检测该报文是否已经超时
        else if(strcmp(bufHead, "Time") == 0)
        {
             sscanf(ptr, "%*[^:]:%s", bufTail);
             if(time_of_arrival - atoi(bufTail) < OVERTIME)
                packTimeOut = true;
        }

        ptr = strtok_r(NULL, SPLIT, &p);
    }

    if(statusCodeExist && ipAddrcheck && packTimeOut)
        return SUCCESS;
    else
        return ERROR;
}

void MainWindow::on_button_gohome_request_clicked()
{
    // 检查IP地址设置
    if(check_ip() != SUCCESS)
        return;

    // 检查必要信息是否已经输入
    if(check_required() != SUCCESS)
        return;

    // 检查回家报文必要信息是否已经输入
    if(check_GoHome_input() != SUCCESS)
        return;

    //初始化GoHome报文数据结构
    GoHomeMsg req_msg;
    memset(req_msg.Cmd, 0x00, sizeof(req_msg.Cmd));
    memset(req_msg.Ver, 0x00, sizeof(req_msg.Ver));
    req_msg.Seq = 0;
    req_msg.Time = 0;
    memset(req_msg.FromIp, 0x00, sizeof(req_msg.FromIp));
    memset(req_msg.ToIp, 0x00, sizeof(req_msg.ToIp));
    memset(req_msg.ReqUser, 0x00, sizeof(req_msg.ReqUser));
    memset(req_msg.RspUser, 0x00, sizeof(req_msg.RspUser));
    memset(req_msg.Session, 0x00, sizeof(req_msg.Session));
    req_msg.BaseFloor = 0;
    int zz = 0;
    while(zz != sizeof(req_msg.Home[0]))
    {
        memset(req_msg.Home[zz], 0x00, sizeof(req_msg.Home[zz]));
        zz++;
    }
    req_msg.TriggerType = 0;
    req_msg.CardNum = 0;
    req_msg.Unlock = 0;
    req_msg.LockNum = 0;
    req_msg.ReaderNum = 0;
    memset(req_msg.CheckCode, 0x00, sizeof(req_msg.CheckCode));

    //根据控制台输入信息组装GoHome请求报文并发送
    MsgSend_ReqGoHome(req_msg);
}

void MainWindow::on_button_leavehome_request_clicked()
{
    // 检查IP地址设置
    if(check_ip() != SUCCESS)
        return;

    // 检查必要信息是否已经输入
    if(check_required() != SUCCESS)
        return;

    // 检查回家报文必要信息是否已经输入
    if(check_LeaveHome_input() != SUCCESS)
        return;

    //初始化LeaveHome报文数据结构
    LeaveHomeMsg req_msg;
    memset(req_msg.Cmd, 0x00, sizeof(req_msg.Cmd));
    memset(req_msg.Ver, 0x00, sizeof(req_msg.Ver));
    req_msg.Seq = 0;
    req_msg.Time = 0;
    memset(req_msg.FromIp, 0x00, sizeof(req_msg.FromIp));
    memset(req_msg.ToIp, 0x00, sizeof(req_msg.ToIp));
    memset(req_msg.ReqUser, 0x00, sizeof(req_msg.ReqUser));
    memset(req_msg.RspUser, 0x00, sizeof(req_msg.RspUser));
    memset(req_msg.Session, 0x00, sizeof(req_msg.Session));
    memset(req_msg.Home, 0x00, sizeof(req_msg.Home));
    memset(req_msg.ToRoom, 0x00, sizeof(req_msg.ToRoom));
    req_msg.TriggerType = 0;
    memset(req_msg.CheckCode, 0x00, sizeof(req_msg.CheckCode));

    //根据控制台输入信息组装LeaveHome请求报文
    MsgSend_ReqLeaveHome(req_msg);
}

int MainWindow::MsgSend_ReqGoHome(GoHomeMsg &req_msg)
{
    bool ok;

    //请求信息ID
    char* command;
    QByteArray tpCommand = ui->box_cmd->itemText(ui->box_cmd->currentIndex()).toLatin1();
    command = tpCommand.data();
    strcpy(req_msg.Cmd, command);

    //协议版本
    char* protocal_version;
    QByteArray tpVersion = ui->txt_ver->text().toLatin1();
    protocal_version = tpVersion.data();
    strcpy(req_msg.Ver, protocal_version);

    //消息请求方的流水号
    req_msg.Seq = sequent;

    //1970年1月1日0时0分0秒至生成消息时经过的秒数
    req_msg.Time = get_time();

    //本地IP地址
    char* ip_address;
    QByteArray tpIp = ui->txt_ip->text().toLatin1();
    ip_address = tpIp.data();
    strcpy(req_msg.FromIp, ip_address);

    //广播地址
    strcpy(req_msg.ToIp, "255.255.255.255");

    //门口机设备标识
    char* device_id;
    QByteArray tpDeviceId = ui->txt_deviceid->text().toLatin1();
    device_id = tpDeviceId.data();
    strcpy(req_msg.ReqUser, device_id);

    //电梯设备标识
    strcpy(req_msg.RspUser, "everyone");

    //会话id
    sprintf(req_msg.Session, "%s-%s-%d-%4.4d", req_msg.Cmd, req_msg.ReqUser, req_msg.Seq, get_rand());

    //业主当前层数
    req_msg.BaseFloor = ui->txt_basefloor->text().toInt(&ok, 10);

    //业主楼层房号
    char* muti_home;
    QByteArray tpMutiHome = ui->txt_home->text().toLatin1();
    muti_home = tpMutiHome.data();
    int zz = 0;
    char* ptr;
    char* p;
    ptr = strtok_r(muti_home, HOME_SPLIT, &p);
    while (ptr != NULL)
    {
        sscanf(ptr, "%[^;]", req_msg.Home[zz]);
        ptr = strtok_r(NULL, HOME_SPLIT, &p);
        zz++;
    }

    //触发事件类型
    req_msg.TriggerType = ui->box_triggertype->currentIndex() + 1;

    //是否打开门锁
    req_msg.Unlock = 1 - ui->box_unlock->currentIndex();

    //门锁编号
    if(req_msg.Unlock != 0)
        req_msg.LockNum = ui->box_locknum->currentIndex() + 1;

    //刷卡的卡号以及读卡器编号
    if(req_msg.TriggerType == 1 || req_msg.TriggerType == 7)
    {
        req_msg.CardNum = ui->txt_cardnum->text().toInt(&ok, 10);
        req_msg.ReaderNum = ui->box_readernum->currentIndex() + 1;
    }

    //获取电梯联动通讯秘钥
    char* password;
    QByteArray tpPassWord = ui->txt_password->text().toLatin1();
    password = tpPassWord.data();

    //待计算的字符串（除Checkcode字段的GoHome报文段+\r\n+通讯密码）
    char splice_msg[MAX_SIZE];
    memset(splice_msg, 0x00, MAX_SIZE);
    sprintf(splice_msg, "Cmd:%s\r\nVer:%s\r\nSeq:%u\r\nTime:%d\r\nFromIP:%s\r\nToIP:%s\r\nReqUser:%s\r\nRspUser:%s\r\nSession:%s\r\nBaseFloor:%d\r\n", req_msg.Cmd, req_msg.Ver, req_msg.Seq, req_msg.Time, req_msg.FromIp, req_msg.ToIp, req_msg.ReqUser, req_msg.RspUser, req_msg.Session, req_msg.BaseFloor);
    zz = 0;
    while( req_msg.Home[zz][0] != '\0' )
    {
        strcat(splice_msg, "Home:");
        strcat(splice_msg, req_msg.Home[zz++]);
        strcat(splice_msg, "\r\n");
    }
    if(req_msg.TriggerType == 1 || req_msg.TriggerType == 7)
    {
        if(req_msg.Unlock == 0)
        {
            sprintf(splice_msg, "%sTriggerType:%u\r\nCardNum:%llu\r\nUnlock:%d\r\nReaderNum:%u\r\n%s\r\n", splice_msg,req_msg.TriggerType, req_msg.CardNum, req_msg.Unlock, req_msg.ReaderNum, password);
        }
        else
        {
            sprintf(splice_msg, "%sTriggerType:%u\r\nCardNum:%llu\r\nUnlock:%d\r\nLockNum:%u\r\nReaderNum:%u\r\n%s\r\n", splice_msg, req_msg.TriggerType, req_msg.CardNum, req_msg.Unlock, req_msg.LockNum, req_msg.ReaderNum, password);
        }
    }
    else
    {
        if(req_msg.Unlock == 0)
        {
            sprintf(splice_msg, "%sTriggerType:%u\r\nUnlock:%d\r\n%s\r\n", splice_msg, req_msg.TriggerType, req_msg.Unlock, password);
        }
        else
        {
            sprintf(splice_msg, "%sTriggerType:%u\r\nUnlock:%d\r\nLockNum:%u\r\n%s\r\n", splice_msg, req_msg.TriggerType, req_msg.Unlock, req_msg.LockNum, password);
        }
    }

    //计算报文中的CheckCode字段值
    generate_CheckCode(splice_msg, req_msg.CheckCode);

    //组装GoHome报文段
    char msg[MAX_SIZE];
    memset(msg, 0x00, MAX_SIZE);
    sprintf(msg, "Cmd:%s\r\nVer:%s\r\nSeq:%u\r\nTime:%d\r\nFromIP:%s\r\nToIP:%s\r\nReqUser:%s\r\nRspUser:%s\r\nSession:%s\r\nBaseFloor:%d\r\n", req_msg.Cmd, req_msg.Ver, req_msg.Seq, req_msg.Time, req_msg.FromIp, req_msg.ToIp, req_msg.ReqUser, req_msg.RspUser, req_msg.Session, req_msg.BaseFloor);
    zz = 0;
    while( req_msg.Home[zz][0] != '\0' )
    {
        strcat(msg, "Home:");
        strcat(msg, req_msg.Home[zz++]);
        strcat(msg, "\r\n");
    }
    if(req_msg.TriggerType == 1 || req_msg.TriggerType == 7)
    {
        if(req_msg.Unlock == 0)
        {
            sprintf(msg, "%sTriggerType:%u\r\nCardNum:%llu\r\nUnlock:%d\r\nReaderNum:%u\r\nCheckCode:%s\r\n", msg,req_msg.TriggerType, req_msg.CardNum, req_msg.Unlock, req_msg.ReaderNum, req_msg.CheckCode);
        }
        else
        {
            sprintf(msg, "%sTriggerType:%u\r\nCardNum:%llu\r\nUnlock:%d\r\nLockNum:%u\r\nReaderNum:%u\r\nCheckCode:%s\r\n", msg, req_msg.TriggerType, req_msg.CardNum, req_msg.Unlock, req_msg.LockNum, req_msg.ReaderNum, req_msg.CheckCode);
        }
    }
    else
    {
        if(req_msg.Unlock == 0)
        {
            sprintf(msg, "%sTriggerType:%u\r\nUnlock:%d\r\nCheckCode:%s\r\n", msg, req_msg.TriggerType, req_msg.Unlock, req_msg.CheckCode);
        }
        else
        {
            sprintf(msg, "%sTriggerType:%u\r\nUnlock:%d\r\nLockNum:%u\r\nCheckCode:%s\r\n", msg, req_msg.TriggerType, req_msg.Unlock, req_msg.LockNum, req_msg.CheckCode);
        }
    }

    //回显原生报文
    char MsgForTheShow[MAX_SIZE];
    memset(MsgForTheShow, 0x00, MAX_SIZE);
    sprintf(MsgForTheShow, "Cmd:%s\\r\\nVer:%s\\r\\nSeq:%u\\r\\nTime:%d\\r\\nFromIP:%s\\r\\nToIP:%s\\r\\nReqUser:%s\\r\\nRspUser:%s\\r\\nSession:%s\\r\\nBaseFloor:%d\\r\\n", req_msg.Cmd, req_msg.Ver, req_msg.Seq, req_msg.Time, req_msg.FromIp, req_msg.ToIp, req_msg.ReqUser, req_msg.RspUser, req_msg.Session, req_msg.BaseFloor);
    zz = 0;
    while( req_msg.Home[zz][0] != '\0' )
    {
        strcat(MsgForTheShow, "Home:");
        strcat(MsgForTheShow, req_msg.Home[zz++]);
        strcat(MsgForTheShow, "\\r\\n");
    }
    if(req_msg.TriggerType == 1 || req_msg.TriggerType == 7)
    {
        if(req_msg.Unlock == 0)
        {
            sprintf(MsgForTheShow, "%sTriggerType:%u\\r\\nCardNum:%llu\\r\\nUnlock:%d\\r\\nReaderNum:%u\\r\\nCheckCode:%s\\r\\n", MsgForTheShow, req_msg.TriggerType, req_msg.CardNum, req_msg.Unlock, req_msg.ReaderNum, req_msg.CheckCode);
        }
        else
        {
            sprintf(MsgForTheShow, "%sTriggerType:%u\\r\\nCardNum:%llu\\r\\nUnlock:%d\\r\\nLockNum:%u\\r\\nReaderNum:%u\\r\\nCheckCode:%s\\r\\n", MsgForTheShow, req_msg.TriggerType, req_msg.CardNum, req_msg.Unlock, req_msg.LockNum, req_msg.ReaderNum, req_msg.CheckCode);
        }
    }
    else
    {
        if(req_msg.Unlock == 0)
        {
            sprintf(MsgForTheShow, "%sTriggerType:%u\\r\\nUnlock:%d\\r\\nCheckCode:%s\\r\\n", MsgForTheShow, req_msg.TriggerType, req_msg.Unlock, req_msg.CheckCode);
        }
        else
        {
            sprintf(MsgForTheShow, "%sTriggerType:%u\\r\\nUnlock:%d\\r\\nLockNum:%u\\r\\nCheckCode:%s\\r\\n", MsgForTheShow, req_msg.TriggerType, req_msg.Unlock, req_msg.LockNum, req_msg.CheckCode);
        }
    }

    //计算当前发送数据时间
    char timeStr[MIN_SIZE];
    struct tm *pv;
    time(&time_of_arrival);
    pv = gmtime(&time_of_arrival);
    memset(timeStr, 0, MIN_SIZE);
    sprintf(timeStr, "[ %d-%02d-%02d %02d:%02d:%02d ]", 1900 + pv->tm_year, 1 + pv->tm_mon, pv->tm_mday, 8 + pv->tm_hour, pv->tm_min, pv->tm_sec);
    QString timeQStr;
    timeQStr = QString(QLatin1String(timeStr));

    //广播发送
    QByteArray datagram(msg);
    QByteArray demonstrate(MsgForTheShow);

    ui->txt_rx->append("-----Send Go Home Request-----\nTime: ");
    ui->txt_rx->insertPlainText(timeQStr);
    ui->txt_rx->moveCursor(QTextCursor::End);
    ui->txt_rx->append("Source: ");
    ui->txt_rx->insertPlainText(ui->txt_ip->text());
    ui->txt_rx->moveCursor(QTextCursor::End);
    ui->txt_rx->append("\n-----Message Content List-----");
    ui->txt_rx->append(datagram);
    ui->txt_rx->append("-----Original Message Content-----");
    ui->txt_rx->append(demonstrate);
    ui->txt_rx->moveCursor(QTextCursor::End);
    ui->txt_rx->append("\n");

    udp_socket_tx->writeDatagram(datagram, datagram.size(), QHostAddress::Broadcast, 19180);
    sequent++;
    return SUCCESS;
}

int MainWindow::MsgSend_ReqLeaveHome(LeaveHomeMsg &req_msg)
{
    //请求信息ID
    char* command;
    QByteArray tpCommand = ui->box_cmd->itemText(ui->box_cmd->currentIndex()).toLatin1();
    command = tpCommand.data();
    strcpy(req_msg.Cmd, command);

    //协议版本
    char* protocal_version;
    QByteArray tpVersion = ui->txt_ver->text().toLatin1();
    protocal_version = tpVersion.data();
    strcpy(req_msg.Ver, protocal_version);

    //消息请求方的流水号
    req_msg.Seq = sequent;

    //1970年1月1日0时0分0秒至生成消息时经过的秒数
    req_msg.Time = get_time();

    //本地IP地址
    char* ip_address;
    QByteArray tpIp = ui->txt_ip->text().toLatin1();
    ip_address = tpIp.data();
    strcpy(req_msg.FromIp, ip_address);

    //广播地址
    strcpy(req_msg.ToIp, "255.255.255.255");

    //门口机设备标识
    char* device_id;
    QByteArray tpDeviceId = ui->txt_deviceid->text().toLatin1();
    device_id = tpDeviceId.data();
    strcpy(req_msg.ReqUser, device_id);

    //电梯设备标识
    strcpy(req_msg.RspUser, "everyone");

    //会话id
    sprintf(req_msg.Session, "%s-%s-%d-%4.4d", req_msg.Cmd, req_msg.ReqUser, req_msg.Seq, get_rand());

    //主家的楼层房号
    char* single_home;
    QByteArray tpHome = ui->txt_home_l->text().toLatin1();
    single_home = tpHome.data();
    strcpy(req_msg.Home, single_home);

    //业主去往楼层
    char* to_room;
    QByteArray tpToRoom = ui->txt_toroom_l->text().toLatin1();
    to_room = tpToRoom.data();
    strcpy(req_msg.ToRoom, to_room);

    //触发事件类型
    req_msg.TriggerType = ui->box_triggertype_l->currentIndex() + 101;

    //获取电梯联动通讯秘钥
    char* password;
    QByteArray tpPassWord = ui->txt_password->text().toLatin1();
    password = tpPassWord.data();

    //待计算的字符串（回应内容+\r\n+通讯密码）
    char splice_msg[MAX_SIZE];
    memset(splice_msg, 0x00, MAX_SIZE);
    sprintf(splice_msg,"Cmd:%s\r\nVer:%s\r\nSeq:%u\r\nTime:%d\r\nFromIP:%s\r\nToIP:%s\r\nReqUser:%s\r\nRspUser:%s\r\nSession:%s\r\nHome:%s\r\nToRoom:%s\r\nTriggerType:%u\r\n%s\r\n", req_msg.Cmd, req_msg.Ver, req_msg.Seq, req_msg.Time, req_msg.FromIp, req_msg.ToIp, req_msg.ReqUser, req_msg.RspUser, req_msg.Session, req_msg.Home, req_msg.ToRoom, req_msg.TriggerType, password);

    //计算报文中的CheckCode字段值
    generate_CheckCode(splice_msg, req_msg.CheckCode);

    //组装LeaveHome报文段
    char msg[MAX_SIZE];
    memset(msg, 0x00, MAX_SIZE);
    sprintf(msg,"Cmd:%s\r\nVer:%s\r\nSeq:%u\r\nTime:%d\r\nFromIP:%s\r\nToIP:%s\r\nReqUser:%s\r\nRspUser:%s\r\nSession:%s\r\nHome:%s\r\nToRoom:%s\r\nTriggerType:%u\r\nCheckCode:%s\r\n", req_msg.Cmd, req_msg.Ver, req_msg.Seq, req_msg.Time, req_msg.FromIp, req_msg.ToIp, req_msg.ReqUser, req_msg.RspUser, req_msg.Session, req_msg.Home, req_msg.ToRoom, req_msg.TriggerType, req_msg.CheckCode);

    //回显原生报文
    char MsgForTheShow[MAX_SIZE];
    memset(MsgForTheShow, 0x00, MAX_SIZE);
    sprintf(MsgForTheShow,"Cmd:%s\\r\\nVer:%s\\r\\nSeq:%u\\r\\nTime:%d\\r\\nFromIP:%s\\r\\nToIP:%s\\r\\nReqUser:%s\\r\\nRspUser:%s\\r\\nSession:%s\\r\\nHome:%s\\r\\nToRoom:%s\\r\\nTriggerType:%u\\r\\nCheckCode:%s\\r\\n", req_msg.Cmd, req_msg.Ver, req_msg.Seq, req_msg.Time, req_msg.FromIp, req_msg.ToIp, req_msg.ReqUser, req_msg.RspUser, req_msg.Session, req_msg.Home, req_msg.ToRoom, req_msg.TriggerType, req_msg.CheckCode);

    //计算当前发送数据时间
    char timeStr[MIN_SIZE];
    struct tm *pv;
    time(&time_of_arrival);
    pv = gmtime(&time_of_arrival);
    memset(timeStr, 0, MIN_SIZE);
    sprintf(timeStr, "[ %d-%02d-%02d %02d:%02d:%02d ]", 1900 + pv->tm_year, 1 + pv->tm_mon, pv->tm_mday, 8 + pv->tm_hour, pv->tm_min, pv->tm_sec);
    QString timeQStr;
    timeQStr = QString(QLatin1String(timeStr));

    //广播发送
    QByteArray datagram(msg);
    QByteArray demonstrate(MsgForTheShow);

    ui->txt_rx->append("\n-----Send Leave Home Request-----\nTime: ");
    ui->txt_rx->insertPlainText(timeQStr);
    ui->txt_rx->moveCursor(QTextCursor::End);
    ui->txt_rx->append("Source: ");
    ui->txt_rx->insertPlainText(ui->txt_ip->text());
    ui->txt_rx->moveCursor(QTextCursor::End);
    ui->txt_rx->append("\n-----Message Content List-----");
    ui->txt_rx->append(datagram);
    ui->txt_rx->append("-----Original Message Content-----");
    ui->txt_rx->append(demonstrate);
    ui->txt_rx->moveCursor(QTextCursor::End);
    ui->txt_rx->append("\n");

    udp_socket_tx->writeDatagram(datagram, datagram.size(), QHostAddress::Broadcast, 19180);
    sequent++;
    return SUCCESS;
}

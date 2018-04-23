#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QtNetwork/QUdpSocket>
#include <QHostInfo>
#include <QThread>
#include <QTimer>
#include <QString>
#include <QScrollArea>

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

extern "C"
{
#include "md5.h"
}

// 函数返回码
#define SUCCESS             0       /* 执行成功 */
#define ERROR               100     /* 发生错误 */
#define GOHOME              101     /* 回家报文标识 */
#define LEAVEHOME           102     /* 离家报文标识 */
#define RESPONE             103     /* 回应报文标识 */

// 标签头尾字符串长度
#define DATA_HEAD_SIZE      128
#define DATA_TAIL_SIZE      512

// 报文最大长度
#define MAX_SIZE            2048
#define MIN_SIZE            128

// 报文分割字符
#define SPLIT               "\r\n"

// Home字段控制台输入分隔符
#define HOME_SPLIT          ";"

// 报文超时时长
#define OVERTIME            30

// 回家发送报文结构
typedef struct
{
    char    Cmd[20];        /* 请求消息ID */
    char    Ver[5];         /* 协议版本 */
    uint    Seq;            /* 消息请求方的流水号 */
    int     Time;           /* 1970年1月1日0时0分0秒至生成消息时经过的秒数 */
    char    FromIp[16];     /* 消息发送方的IP地址 */
    char    ToIp[16];       /* 期望本消息被那个IP地址的设备都接收处理 */
    char    ReqUser[40];    /* 会话发起方的设备标识 */
    char    RspUser[40];    /* 会话发起方希望处理这条消息的接收方的设备标识 */
    char    Session[100];    /* 会话id */
    int     BaseFloor;      /* 业主当前站在哪一层楼 */
    char    Home[10][10];   /* 业主家的楼层房号 */
    uint    TriggerType;    /* 触发这个接口的事件类型 */
    quint64 CardNum;        /* 刷卡的卡号*/
    bool    Unlock;         /* 是否打开门锁 */
    uint    LockNum;        /* 门锁编号 */
    uint    ReaderNum;      /* 读卡器编号 */
    char    CheckCode[40];  /* 消息校验码，用于判断消息是否被篡改 */
}GoHomeMsg;

// 离家发送报文结构
typedef struct
{
    char    Cmd[20];        /* 请求消息ID */
    char    Ver[5];         /* 协议版本 */
    uint    Seq;            /* 消息请求方的流水号 */
    int     Time;           /* 1970年1月1日0时0分0秒至生成消息时经过的秒数 */
    char    FromIp[16];     /* 消息发送方的IP地址 */
    char    ToIp[16];       /* 期望本消息被那个IP地址的设备都接收处理 */
    char    ReqUser[40];    /* 会话发起方的设备标识 */
    char    RspUser[40];    /* 会话发起方希望处理这条消息的接收方的设备标识 */
    char    Session[100];    /* 会话id */
    char    Home[10];       /* 业主家的楼层房号 */
    char    ToRoom[10];     /* 业主想从自家去哪户人家的楼层房号 */
    uint    TriggerType;    /* 触发这个接口的事件类型 */
    char    CheckCode[40];  /* 消息校验码，用于判断消息是否被篡改 */
}LeaveHomeMsg;

// 响应报文结构
typedef struct
{
    int     StatusCode;     /* 状态码 */
    char    Ver[5];         /* 协议版本 */
    uint    Seq;            /* 消息请求方的流水号 */
    int     Time;           /* 1970年1月1日0时0分0秒至生成消息时经过的秒数 */
    char    FromIp[16];     /* 消息发送方的IP地址 */
    char    ToIp[16];       /* 期望本消息被那个IP地址的设备都接收处理 */
    char    ReqUser[40];    /* 会话发起方的设备标识 */
    char    RspUser[40];    /* 会话发起方希望处理这条消息的接收方的设备标识 */
    char    Session[100];   /* 会话id */
    char    CheckCode[40];  /* 消息校验码，用于判断消息是否被篡改 */
} ResponseMsg;

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

    /*
     * @函数功能：在程序界面的"请求消息ID"项中，根据选择的内容来禁止某些控件的使用
     * @参数：arg1 下拉自选框中的字符串
     */
    void on_box_cmd_currentIndexChanged(const QString &arg1);

    /*
     * @函数功能：在程序界面的"请求消息ID"项中，根据选择的内容来禁止某些控件的使用
     * @参数：arg1 下拉自选框中的字符串
     */
    void on_box_triggertype_currentIndexChanged(const QString &arg1);

    /*
     * @函数功能：在程序界面的"开启门锁"项中，根据选择的内容来禁止某些控件的使用
     * @参数：arg1 下拉自选框中的字符串
     */
    void on_box_unlock_currentIndexChanged(const QString &arg1);

    /*
     * @函数功能：在程序界面的触发"清空来往消息"按键，清空“来往消息监控”控件中的内容
     */
    void on_button_refresh_clicked();

    /*
     * @函数功能：在程序界面的触发"发送回家请求报文"按键，发送出回家请求广播报文
     */
    void on_button_gohome_request_clicked();

    /*
     * @函数功能：在程序界面的触发"发送离家请求报文"按键，发送出离家请求广播报文
     */
    void on_button_leavehome_request_clicked();

    /*
     * @函数功能：等待即将来临的udp数据包
     */
    void rx_udp();

private:

    /*
     * @函数功能：检测在界面“本机Ip地址”输入的Ip地址是否为本机当前可用的Ip地址
     * @返回值：返回函数返回码,参考mainwindow.h
     */
    int check_ip();

    /*
     * @函数功能：检测在界面“基础报文信息”信息栏中，所用输入控件是否正确输入
     * @返回值：返回函数返回码,参考mainwindow.h
     */
    int check_required();

    /*
     * @函数功能：检测在界面“回家请求报文信息”信息栏中，所用输入控件是否正确输入
     * @返回值：返回函数返回码,参考mainwindow.h
     */
    int check_GoHome_input();

    /*
     * @函数功能：检测在界面“离家请求报文信息”信息栏中，所用输入控件是否正确输入
     * @返回值：返回函数返回码,参考mainwindow.h
     */
    int check_LeaveHome_input();

    /*
     * @函数功能：获取当前时间值
     * @返回值：4字节整形数据（1970年1月1日0时0分0秒至生成消息时经过的秒数）
     */
    int get_time();

    /*
     * @函数功能：获取4位整形随机数
     * @返回值：4位整形随机数
     */
    int get_rand();

    /*
     * @函数功能：检查接收到的报文是否合法
     * @参数：rev 接收到的报文字符串
     * @返回值：返回函数返回码,参考mainwindow.h
     */
    int check_package(QString rev);

    /*
     * @函数功能：打包发送回家报文
     * @参数：req_msg 回家报文中的数据
     * @返回值：返回函数返回码,参考mainwindow.h
     */
    int MsgSend_ReqGoHome(GoHomeMsg &req_msg);

    /*
     * @函数功能：打包发送离家报文
     * @参数：req_msg 离家报文中的数据
     * @返回值：返回函数返回码,参考mainwindow.h
     */
    int MsgSend_ReqLeaveHome(LeaveHomeMsg &req_msg);

    /*
     * @函数功能：生成checkcode
     * @参数：src 待计算的字符串（除Checkcode字段的GoHome报文段+\r\n+通讯密码）
     * @参数：dst 生成的chekcode
     * @返回值：返回函数返回码,参考mainwindow.h
     */
    int generate_CheckCode(char src[], char dst[]);

public:

    /* 发送socket */
    QUdpSocket *udp_socket_tx;

    /* 接收socket */
    QUdpSocket *udp_socket_rx;

    /* 接收到数据包的源地址 */
    QString package_source;

    /* 数据包到达时间 */
    time_t time_of_arrival;

    /* 报文发送流水号 */
    int sequent;

private:

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H

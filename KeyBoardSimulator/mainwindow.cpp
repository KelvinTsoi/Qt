#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //初始化串口参数
    Serial::Instance()->InitSerial(ui);

    //固定主窗口不能改动
    this->setFixedSize(460, 425);

    //限制文本框输入电话号码
    QRegExp regx("[0-9]+$");
    QValidator *validator = new QRegExpValidator(regx, ui->PhoneLineEdit );
    ui->PhoneLineEdit->setValidator(validator);

    //工作模式下拉条
    ui->WorkingModeComboBox->addItem(tr("   Manual"));
    ui->WorkingModeComboBox->addItem(tr("   Auto"));

    //置灰主窗口功能
    ui->DisconnectButton->setEnabled(false);
    ui->AutoModeGroupBox->setEnabled(false);
    ui->WorkingModeGroupBox->setEnabled(false);
    ui->ManualModeGroupBox->setEnabled(false);

    //启动线程
    WorkingThread = new Thread();

    //启动线程循环
    WorkingThread->start();

    //信号槽
    connect(WorkingThread, SIGNAL(ThreadSignal(QString)), this, SLOT(on_Instructions(QString)));

    qDebug() << "Main Window init completed" << endl;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_ConnectButton_clicked()
{
    //Connect按钮置灰
    ui->ConnectButton->setEnabled(false);

    //Disconnect按钮使能
    ui->DisconnectButton->setEnabled(true);

    //使能主窗口功能
    ui->AutoModeGroupBox->setEnabled(false);
    ui->WorkingModeGroupBox->setEnabled(true);
    ui->ManualModeGroupBox->setEnabled(true);

    //启动串口
    if(Serial::Instance()->OpenSerial(ui->ComComboBox->currentText()) != 0)
    {
        QMessageBox::critical(this,"Critical",tr("Serial Port Open Error"));
    }
}

void MainWindow::on_DisconnectButton_clicked()
{
    //关闭串口
    Serial::Instance()->CloseSerial();

    //Connect按钮使能
    ui->ConnectButton->setEnabled(true);

    //Disconnect按钮置灰
    ui->DisconnectButton->setEnabled(false);

    //置灰主窗口功能
    ui->AutoModeGroupBox->setEnabled(false);
    ui->WorkingModeGroupBox->setEnabled(false);
    ui->ManualModeGroupBox->setEnabled(false);
}


void MainWindow::on_WorkingModeComboBox_currentIndexChanged(int index)
{
    if(index == 0)
    {
        ui->AutoModeGroupBox->setEnabled(false);
        ui->ManualModeGroupBox->setEnabled(true);
    }
    else if(index == 1)
    {
        ui->AutoModeGroupBox->setEnabled(true);
        ui->ManualModeGroupBox->setEnabled(false);
    }
    else
    {
        ;
    }
}

void MainWindow::on_PathButton_clicked()
{
    //查找文件
    QString directory = QFileDialog::getOpenFileName(NULL,"Traversal",".","*.xml");
    if(!directory.isEmpty())
    {
        if(ui->DirectoryComboBox->findText(directory) == -1)\
        {
            ui->DirectoryComboBox->addItem(directory);
        }
        ui->DirectoryComboBox->setCurrentIndex(ui->DirectoryComboBox->findText(directory));
    }
}

void MainWindow::on_OuchButton_clicked(bool checked)
{
    //Start Auto Process
    if(checked)
    {
        //检测电话号码
        QString PhoneNumber = ui->PhoneLineEdit->text();
        if(PhoneNumber.length() != 11)
        {
            QMessageBox::critical(this,"Critical",tr("Illegal Phone Number"));
            return;
        }

        //检测XML文件是否存在
        QString Directory = ui->DirectoryComboBox->currentText();
        QFile file(Directory);
        if (!file.open(QFile::ReadOnly))
        {
            QMessageBox::critical(this,"Critical",tr("XML Open Error"));
            return;
        }
        else
        {
            file.close();
        }

        //Start Thread
        WorkingThread->ThreadStart(PhoneNumber, Directory);
    }
    //Cancel Auto Process
    else
    {
        //Stop Thread
        WorkingThread->ThreadStop();
    }
}

void MainWindow::on_Instructions(QString ins)
{
    char* pwList;

    QByteArray tp = ins.toLatin1();

    pwList = tp.data();

    Serial::Instance()->SendInstructions(pwList, 7);
}

void MainWindow::on_Key1_clicked()
{
    Serial::Instance()->SendQuickResponseMessage('1');
}

void MainWindow::on_Key2_clicked()
{
    Serial::Instance()->SendQuickResponseMessage('2');
}

void MainWindow::on_Key3_clicked()
{
    Serial::Instance()->SendQuickResponseMessage('3');
}

void MainWindow::on_Key4_clicked()
{
    Serial::Instance()->SendQuickResponseMessage('4');
}

void MainWindow::on_Key5_clicked()
{
    Serial::Instance()->SendQuickResponseMessage('5');
}

void MainWindow::on_Key6_clicked()
{
    Serial::Instance()->SendQuickResponseMessage('6');
}

void MainWindow::on_Key7_clicked()
{
    Serial::Instance()->SendQuickResponseMessage('7');
}

void MainWindow::on_Key8_clicked()
{
    Serial::Instance()->SendQuickResponseMessage('8');
}

void MainWindow::on_Key9_clicked()
{
    Serial::Instance()->SendQuickResponseMessage('9');
}

void MainWindow::on_Key0_clicked()
{
    Serial::Instance()->SendQuickResponseMessage('0');
}

void MainWindow::on_KeyStar_clicked()
{
    Serial::Instance()->SendQuickResponseMessage('*');
}

void MainWindow::on_KeySharp_clicked()
{
    Serial::Instance()->SendQuickResponseMessage('#');
}

void MainWindow::on_KeyBackspace_clicked()
{
    Serial::Instance()->SendQuickResponseMessage('c');
}

void MainWindow::on_KeyUp_clicked()
{
    Serial::Instance()->SendQuickResponseMessage('u');
}

void MainWindow::on_KeyDown_clicked()
{
    Serial::Instance()->SendQuickResponseMessage('d');
}

void MainWindow::on_KeyOK_clicked()
{
    Serial::Instance()->SendQuickResponseMessage('o');
}


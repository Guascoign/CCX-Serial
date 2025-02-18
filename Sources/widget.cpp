#include "widget.h"
#include "ncombobox.h"
#include "./ui_widget.h"
#include <QDateTime>
#include <QLineSeries>
#include <QMainWindow>
#include <QRandomGenerator>
#include <QtMath>
#include <QValueAxis>
#include <QSerialPortInfo>
#include <QStandardItemModel>
#include <QInputDialog>
#include "customplot.h"
#include "plotthread.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setLayout(ui->gridLayoutGlobal);//布局缩放绑定

    System_Init();//系统初始化
}

Widget::~Widget()
{
    delete ui;
}

// 系统初始化
void Widget::System_Init()
{
    qDebug() << "Qt version:" << QT_VERSION_STR;
    qDebug() << "主线程ID：" << QThread::currentThreadId();
//----------------------时间初始化----------------------
    Update_RTC();//更新时间,打开定时器
    // 设置定时器每秒更新一次时间
    timeUpdateTimer = new QTimer(this);
    connect(timeUpdateTimer, &QTimer::timeout, this, &Widget::Update_RTC);
    timeUpdateTimer->start(TIMEOUT1S);
    animateProgressBar(0, 10, 100);
//---------------------界面初始化----------------------
    ui->Addons_groupBox->hide();//默认隐藏扩展
    // 设置默认值
    ui->BaudRate_comboBox->setCurrentIndex(6);// 115200
    ui->DataBits_comboBox->setCurrentIndex(3);// 8
    ui->Parity_comboBox->setCurrentIndex(0);// None
    ui->StopBits_comboBox->setCurrentIndex(0);// 1
    ui->FlowControl_comboBox->setCurrentIndex(0);// None
    ui->AUTOSEND_time_spinBox->setValue(1000); // 初始化自动发送时间为1000ms
    animateProgressBar(10, 20, 100);

//---------------------串口初始化----------------------
    connect(ui->Serial_number_comboBox,SIGNAL(clicked()),this,SLOT(Update_SerialPort()));//点击串口下拉框刷新串口
    Update_SerialPort();
    animateProgressBar(20, 30, 100);

//---------------------图表初始化----------------------
    //ui->Wave_ChartView
    ui->Wave_ChartView->addGraph();
    ui->Wave_ChartView->graph(0)->setPen(QPen(Qt::blue));
    ui->Wave_ChartView->addGraph();
    ui->Wave_ChartView->graph(1)->setPen(QPen(Qt::red));
    // 设置 X 轴为时间轴
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    ui->Wave_ChartView->xAxis->setTicker(timeTicker);
    ui->Wave_ChartView->yAxis->setRange(-10, 10); // 设置 Y 轴范围
    //启动刷新线
    PlotThread *plotThread = new PlotThread(this);//开启图像刷新线程
    plotThread->start();
    plotThread->addPlot(ui->Wave_ChartView);//添加刷新图像的句柄
    animateProgressBar(30, 100, 100);
}

//点击串口刷新串口列表
void Widget::on_Serial_number_comboBox_clicked()
{
    Update_SerialPort();
}

// 定时器事件回调函数
void Widget::timerEvent(QTimerEvent *event)
{

}

//更新串口到ComboBox
void Widget::Update_SerialPort()
{
    QStringList serialNamePort;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
        QString portInfo = QString("%1: %2").arg(info.portName()).arg(info.description());
        serialNamePort << portInfo;
    }
    ui->Serial_number_comboBox->clear();
    ui->Serial_number_comboBox->addItems(serialNamePort);
}

//更新RTC
void Widget::Update_RTC()
{
    QString currentTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    ui->time_label->setText(currentTime);
}

//进度条动画
void Widget::animateProgressBar(int startValue, int endValue, int duration) {
    QElapsedTimer timer;
    timer.start();
    while (timer.elapsed() < duration) {
        int value = startValue + (endValue - startValue) * timer.elapsed() / duration;
        ui->progressBar->setValue(value);
        QCoreApplication::processEvents();
    }
    ui->progressBar->setValue(endValue);
}

//清空发送数据
void Widget::on_Clear_sendbuf_pushButton_clicked(){
    ui->send_textEdit->clear();
    ui->info_label->setText("Clear Send Data!");
}

//清空接收数据
void Widget::on_Clear_recvbuf_pushButton_clicked(){
    ui->recv_textEdit->clear();
    ui->info_label->setText("Clear Recv Data!");
    ui->Rx_label->setText("Rx: 0 bytes");
    ui->Tx_label->setText("Tx: 0 bytes");
}

//自动发送
void Widget::on_AUTOSEND_pushButton_clicked()
{
    if(autoSend_Flag){
        autoSend_Flag = false;
        ui->AUTOSEND_pushButton->setText("自动发送");
    }
    else{
        autoSend_Flag = true;
        ui->AUTOSEND_pushButton->setText("停止发送");
    }
    
}

//接收数据处理槽函数 单帧图表数据 data0,data1
void Widget::ProcessData(QByteArray Recvbuff)
{
    // 假设数据格式为 "data0,data1"
    qDebug() << "接收到的数据：" << Recvbuff;

    QList<QByteArray> dataList = Recvbuff.split(',');

    if (dataList.size() == 2) {
        bool ok0, ok1;
        double data0 = dataList[0].toDouble(&ok0);
        double data1 = dataList[1].toDouble(&ok1);
        static QTime timeStart = QTime::currentTime();
        if (ok0 && ok1) {
            dataCount++;
            double key = timeStart.msecsTo(QTime::currentTime())/1000.0;
            ui->Wave_ChartView->graph(0)->addData(key, data0);
            ui->Wave_ChartView->graph(1)->addData(key, data1);
            ui->Wave_ChartView->xAxis->setRange(key - 100, key); // 动态调整 X 轴范围 显示最后100S
        } else {
            qDebug() << "数据解析失败：" << "data0:" << dataList[0] << ", data1:" << dataList[1];
        }
    } else {
        qDebug() << "数据格式错误：" << Recvbuff;
    }
}

// 接收数据的槽函数
void Widget::RcvData(QByteArray RecvBuff)
{
    QString     stringdata;

  
    stringdata = QString(RecvBuff);   /*ascii显示*/
    
    ui->recv_textEdit->insertPlainText(stringdata);
    ui->recv_textEdit->moveCursor(QTextCursor::End);
    dataRxNumber += RecvBuff.length();
    ui->Rx_label->setText(QString("Rx: %1 bytes").arg(dataRxNumber));



    RecvBuff.clear();//清除串口数据

    

}

// 发送数据
void Widget::on_SEND_pushButton_clicked() {

    if (Serial_State == 1){
        SendTextStr = ui->send_textEdit->document()->toPlainText();
        emit SendData(SendTextStr.toUtf8());//线程发送数据

        ui->recv_textEdit->moveCursor(QTextCursor::End);

        ui->recv_textEdit->insertPlainText(QString("%1").arg(SendTextStr));
        dataTxNumber += SendTextStr.toUtf8().size();
        ui->Tx_label->setText(QString("Tx: %1 bytes").arg(dataTxNumber));
    }
    else{
       ui->info_label->setText("请先打开串口");
    }
}

//保存为文件
void Widget::on_Save_pushButton_clicked()
{
    //文件为空则不保存
    if(ui->recv_textEdit->toPlainText().isEmpty()){
        ui->info_label->setText("No Data to Save!");
        return;
    }
    QString fileName = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".txt";
    QFile file(fileName);
    animateProgressBar(0, 50, 500); 
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Can't open file to write!");
        animateProgressBar(50, 0, 500); 
        return;
    }
    QTextStream out(&file);
    out << ui->recv_textEdit->toPlainText();
    file.close();
    animateProgressBar(50, 100, 500);
    ui->info_label->setText("Saved to " + fileName);
    ui->recv_textEdit->clear();
    ui->Rx_label->setText("Rx: 0 bytes");
    ui->Tx_label->setText("Tx: 0 bytes");
    totalTextSize = 0; // 重置总文本大小
    updateSizeLabel(); // 更新 size_label
}

// 更新 size_label
void Widget::updateSizeLabel() {
    QString sizeText;
    if (totalTextSize < 1024 * 1024) {
        sizeText = QString("Size: %1 Kb").arg(totalTextSize / 1024.0, 0, 'f', 2);
    } else if (totalTextSize < 1024 * 1024 * 1024) {
        sizeText = QString("Size: %1 Mb").arg(totalTextSize / (1024.0 * 1024.0), 0, 'f', 2);
    } else {
        sizeText = QString("Size: %1 Gb").arg(totalTextSize / (1024.0 * 1024.0 * 1024.0), 0, 'f', 2);
    }
    ui->size_label->setText(sizeText);
}

// 扩展开关
void Widget::on_Addons_checkBox_clicked(bool checked)
{
    if(checked){
        ui->Addons_groupBox->show();
        //设置串口区域最大宽度800：recv_textEdit FOMAT_groupBox_2 SEND_groupBox
        ui->REV_groupBox->setMaximumWidth(800);
        ui->FOMAT_groupBox_2->setMaximumWidth(800);
        ui->SEND_groupBox->setMaximumWidth(800);
        //打印 Addons_checkBox->on/off
        qDebug() <<"Addons_checkBox->" << checked << Qt::endl;
    }
    else{
        ui->Addons_groupBox->hide();
        ui->REV_groupBox->setMaximumWidth(16777215);
        ui->FOMAT_groupBox_2->setMaximumWidth(16777215);
        ui->SEND_groupBox->setMaximumWidth(16777215);
        qDebug() <<"Addons_checkBox->" << checked << Qt::endl;
    }
}

//串口错误信号上报槽
void Widget::updateInfoLabel(const QString &errorMessage)
{
    on_Opem_COM_pushButton_clicked();
    ui->info_label->setText(errorMessage);
}

// 打开串口
void Widget::on_Opem_COM_pushButton_clicked()
{
    if(Serial_State == 0){
        qint32 state = 0; // 串口状态
        Serial = new ComSerialPort(ui->Serial_number_comboBox->currentText(), ui->BaudRate_comboBox->currentText().toInt(), &state);
        if (state) { // 打开串口成功
            QString portName = ui->Serial_number_comboBox->currentText();
            // 解析 PortName，提取实际的串口名称
            QString actualPortName = portName.split(":").first(); // 获取 "COMX" 部分
            ui->info_label->setText(QString("%1 打开成功！").arg(portName));
            ui->COMnum_Label->setText(QString("串口号：%1 ").arg(actualPortName));
            Serial_State = 1;
            // 获取串口数据
            connect(this, &Widget::SendData, Serial, &ComSerialPort::SendSerialData, Qt::AutoConnection);//线程发送数据
            connect(Serial, SIGNAL(UpdateData(QByteArray)), this, SLOT(RcvData(QByteArray)), Qt::AutoConnection);//线程接收数据
            connect(Serial, SIGNAL(UpdateData(QByteArray)), this, SLOT(ProcessData(QByteArray)), Qt::AutoConnection);// 接收数据处理
            connect(Serial, &ComSerialPort::UpdateError, this, &Widget::updateInfoLabel);//串口错误信号
            ui->Opem_COM_pushButton->setText("关闭串口"); // 更新按钮文本
            ui->Opem_COM_pushButton->setChecked(true);
            ui->Serial_number_comboBox->setEnabled(false);
            ui->BaudRate_comboBox->setEnabled(false);
            ui->DataBits_comboBox->setEnabled(false);
            ui->Parity_comboBox->setEnabled(false);
            ui->StopBits_comboBox->setEnabled(false);
            ui->FlowControl_comboBox->setEnabled(false);
            ui->SEND_pushButton->setEnabled(true);
            ui->AUTOSEND_pushButton->setEnabled(true);
            //recv_textEdit不可鼠标选中编辑
            ui->recv_textEdit->setReadOnly(true);
        } else {
            ui->info_label->setText(QString("ERROR：%1 打开失败！").arg(ui->Serial_number_comboBox->currentText()));
        }
    }
    else{
        Serial->CloseSerial();
        Serial->deleteLater();
        Serial_State = 0;
        ui->info_label->setText(QString("%1 关闭成功！").arg(ui->Serial_number_comboBox->currentText()));
        ui->COMnum_Label->setText("串口号：");
        ui->Opem_COM_pushButton->setText("打开串口");
        ui->Opem_COM_pushButton->setChecked(false);
        ui->Serial_number_comboBox->setEnabled(true);
        ui->BaudRate_comboBox->setEnabled(true);
        ui->DataBits_comboBox->setEnabled(true);
        ui->Parity_comboBox->setEnabled(true);
        ui->StopBits_comboBox->setEnabled(true);
        ui->FlowControl_comboBox->setEnabled(true);
        ui->SEND_pushButton->setEnabled(false);
        ui->AUTOSEND_pushButton->setEnabled(false);
        //recv_textEdit可鼠标选中编辑
        ui->recv_textEdit->setReadOnly(false);
        //关闭自动发送
        autoSend_Flag = false;
        ui->AUTOSEND_pushButton->setText("自动发送");
    }
}

//时间显示
void Widget::on_time_checkBox_clicked(bool checked)
{
    if(checked){
        showTime_Flag = false;
    }
    else{
        showTime_Flag = true;
    }
}

//自动换行
void Widget::on_auto_reline_pushButton_clicked()
{
    if(autoReline_Flag){
        autoReline_Flag = false;
        ui->auto_reline_pushButton->setText("自动换行");
    }
    else{
        autoReline_Flag = true;
        ui->auto_reline_pushButton->setText("无换行");
    }
}

//接收区格式化
void Widget::on_recv_format_pushButton_clicked()
{
    if(recv_format_Flag){
        recv_format_Flag = false;
        ui->recv_format_pushButton->setText("接收:ASCII");
    }
    else{
        recv_format_Flag = true;
        ui->recv_format_pushButton->setText("接收:HEX");
    }
}

//发送区格式化
void Widget::on_send_format_pushButton_clicked()
{
    if(send_format_Flag){
        send_format_Flag = false;
        ui->send_format_pushButton->setText("发送:ASCII");
    }
    else{
        send_format_Flag = true;
        ui->send_format_pushButton->setText("发送:HEX");
    }
}

//自动重连
void Widget::on_Auto_reconnect_checkBox_clicked(bool checked)
{
    if(checked){
        autoReconnect_Flag = true;
    }
    else{
        autoReconnect_Flag = false;
    }
}

//自动滚动
void Widget::on_Auto_roll_pushButton_clicked()
{
    if(autoScrollEnabled_Flag){
        autoScrollEnabled_Flag = false;
        ui->Auto_roll_pushButton->setText("自动滚动");
    }
    else{
        autoScrollEnabled_Flag = true;
        ui->Auto_roll_pushButton->setText("无滚动");
    }
}



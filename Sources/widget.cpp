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
    , dataHandleThread(new DataHandleThread) // 初始化数据处理线程
{
    ui->setupUi(this);
    this->setLayout(ui->gridLayoutGlobal);//布局缩放绑定

    System_Init();//系统初始化

}

Widget::~Widget()
{
    delete ui;
    dataHandleThread->quit(); // 退出数据处理线程
    dataHandleThread->wait(); // 等待线程退出
    delete dataHandleThread; // 删除数据处理线程

    plotThread->quit();
    plotThread->wait();
    delete plotThread;
}

// 系统初始化
void Widget::System_Init()
{
    ////qDebug() << "Qt version:" << QT_VERSION_STR;
    //qDebug() << "主线程ID：" << QThread::currentThreadId();
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
//---------------------数据处理线程初始化----------------------

    dataHandleThread->start();
    connect(dataHandleThread, &DataHandleThread::ProcessedData, this, &Widget::handleProcessedData);
//---------------------串口初始化----------------------
    connect(ui->Serial_number_comboBox,SIGNAL(clicked()),this,SLOT(Update_SerialPort()));//点击串口下拉框刷新串口
    Update_SerialPort();
    animateProgressBar(20, 30, 100);

//---------------------图表初始化----------------------
    //ui->Wave_ChartView
    //启动刷新线程
    plotThread = new PlotThread(this);
    plotThread->start();
    on_AddChart_pushButton_clicked();
    animateProgressBar(30, 100, 100);
}

void Widget::on_AddChart_pushButton_clicked() // 添加图表
{
    // 创建一个新的 CustomPlot 对象
    CustomPlot *newPlot = new CustomPlot(this);
    connect(newPlot, &CustomPlot::plotConstructed, plotThread, &PlotThread::addPlot);
    emit newPlot->plotConstructed(newPlot); // 触发信号
    //qDebug() << "创建图表句柄：" << newPlot ;
    
    // 获取当前布局中的图表数量
    int count = ui->gridLayout_6->count();

    // 计算行和列的索引
    int row = count / 2;
    int col = count % 2;

    // 将新图表添加到布局中的指定位置
    ui->gridLayout_6->addWidget(newPlot, row, col);

    // 重新布局
    ui->gridLayout_6->update();
}

void Widget::on_DeleteChart_pushButton_clicked() // 删除图表
{
    // 获取布局中的最后一个图表
    int count = ui->gridLayout_6->count();
    if (count > 0) {
        QLayoutItem *item = ui->gridLayout_6->takeAt(count - 1);
        if (item) {
            QWidget *widget = item->widget();
            if (widget) {
                CustomPlot *plot = qobject_cast<CustomPlot *>(widget);//转换类型
                if (plot) {
                    connect(plot, &CustomPlot::deleteplot, plotThread, &PlotThread::removePlot);//需要先删除线程中的定时器，否则定时器句柄溢出
                    emit plot->deleteplot(plot); // 触发信号
                }
                widget->deleteLater();
            }
            delete item;
        }
        // 重新布局
        ui->gridLayout_6->update();
    }
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


// 接收数据的槽函数
void Widget::RcvData(QByteArray RecvBuff)
{
    QString stringdata;
    stringdata = QString(RecvBuff);   /*ascii显示*/
    ui->recv_textEdit->insertPlainText(stringdata);
    ui->recv_textEdit->moveCursor(QTextCursor::End);
    dataRxNumber += RecvBuff.length();
    ui->Rx_label->setText(QString("Rx: %1 bytes").arg(dataRxNumber));
    RecvBuff.clear(); // 清除串口数据

    // 检查 dataRxNumber 是否达到 5000
    if (dataRxNumber >= 5000 * clearflag) {
        ui->recv_textEdit->clear(); // 清空接收区
        clearflag++;
    }
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
        //qDebug() <<"Addons_checkBox->" << checked << Qt::endl;
    }
    else{
        ui->Addons_groupBox->hide();
        ui->REV_groupBox->setMaximumWidth(16777215);
        ui->FOMAT_groupBox_2->setMaximumWidth(16777215);
        ui->SEND_groupBox->setMaximumWidth(16777215);
        //qDebug() <<"Addons_checkBox->" << checked << Qt::endl;
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
            connect(Serial, &ComSerialPort::DataToProcessingThread, this, &Widget::SendDataToProcessingThread);// 连接串口数据到数据处理线程
            connect(this, &Widget::SendDataToProcessingThread, dataHandleThread, &DataHandleThread::handleData);
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


void Widget::handleProcessedData(const QStringList &dataList)
{
    ui->listWidget->clear();
    int count = ui->gridLayout_6->count();
    QLayoutItem *item = ui->gridLayout_6->itemAt(count - 1);
    QWidget *widget = item->widget();
    CustomPlot *lastChart = qobject_cast<CustomPlot *>(widget);
    // for (const QString &data : dataList) {
    //     ui->listWidget->addItem(data);
    // }
    if (!datainit) {
        for (int i = 0; i < dataList.size(); ++i) {
            lastChart->addGraph();
            lastChart->graph(i)->setPen(QPen(QColor(QRandomGenerator::global()->bounded(256), 
                                                    QRandomGenerator::global()->bounded(256), 
                                                    QRandomGenerator::global()->bounded(256))));
        }
        datainit = true;
        QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
        timeTicker->setTimeFormat("%h:%m:%s");
        lastChart->xAxis->setTicker(timeTicker);
    }
    static QTime timeStart = QTime::currentTime();
    // 将处理后的数据填入最后一个图表
    lastChart->x_key = timeStart.msecsTo(QTime::currentTime())/1000.0; // 使用当前时间作为 key
    for (int i = 0; i < dataList.size(); ++i) {
        double value = dataList[i].toDouble();
        if (i < lastChart->graphCount()) {
            lastChart->graph(i)->addData(lastChart->x_key, value); // 添加数据到图表
        }
    }
}

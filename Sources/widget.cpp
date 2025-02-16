#include "widget.h"
#include "ncombobox.h"
#include "./ui_widget.h"
#include "chart.h"
#include "chartview.h"
#include <QDateTime>
#include <QLineSeries>
#include <QMainWindow>
#include <QRandomGenerator>
#include <QtMath>
#include <QValueAxis>
#include <QSerialPortInfo>
#include "wave.h"
#include <QStandardItemModel>
#include <QInputDialog>
#include <qcustomplot.h>

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
Wave *wave = new Wave(); // 确保使用指针
ui->Wave_ChartView->setRenderHint(QPainter::Antialiasing);// 设置抗锯齿
ui->period_lineEdit->setText("10");
ui->amplitude_lineEdit->setText("1");
ui->length_lineEdit->setText("10");
ui->noiseLevel_lineEdit->setText("0");
QStandardItemModel *model = new QStandardItemModel(this);
ui->show_columnView->setModel(model); // 设置模型

animateProgressBar(30, 100, 100);
}


// 生成波形
void Widget::on_generate_wave_pushButton_clicked()
{
    // 获取用户输入
    QString waveType = ui->waveType_comboBox->currentText(); // 获取波形类型
    double period = ui->period_lineEdit->text().toDouble(); // 获取周期
    double amplitude = ui->amplitude_lineEdit->text().toDouble(); // 获取幅度
    double length = ui->length_lineEdit->text().toDouble(); // 获取长度
    double noiseLevel = ui->noiseLevel_lineEdit->text().toDouble(); // 获取噪声级别

    // 计算当前波形数量
    int totalWaveCount = 0;
    QStandardItemModel *showModel = qobject_cast<QStandardItemModel*>(ui->show_columnView->model());
    QStandardItemModel *hideModel = qobject_cast<QStandardItemModel*>(ui->hide_columnView->model());

    if (showModel) {
        totalWaveCount += showModel->rowCount(); // 添加 show_columnView 的数量
    }
    if (hideModel) {
        totalWaveCount += hideModel->rowCount(); // 添加 hide_columnView 的数量
    }

    // 弹出输入框获取波形名称
    bool ok;
    QString seriesName = QInputDialog::getText(this, tr("输入波形名称"),
                                                 tr("波形名称:"), QLineEdit::Normal,
                                                 QString("波形%1").arg(totalWaveCount + 1), &ok);
    if (!ok || seriesName.isEmpty()) {
        ui->info_label->setText("未输入波形名称！"); // 显示提示信息
        return; // 如果用户取消或输入为空，则返回
    }

    // 检查是否重名
    QString originalName = seriesName;
    int suffix = 1;
    while (showModel && (showModel->findItems(seriesName).size() > 0 || (hideModel && hideModel->findItems(seriesName).size() > 0))) {
        seriesName = QString("%1_%2").arg(originalName).arg(suffix++);
    }

    // 检查其他输入是否有效
    if (period <= 0) {
        ui->info_label->setText("未输入有效的周期！");
        return;
    }
    if (amplitude <= 0) {
        ui->info_label->setText("未输入有效的幅度！");
        return;
    }
    if (length <= 0) {
        ui->info_label->setText("未输入有效的长度！");
        return;
    }
    if (noiseLevel < 0) {
        ui->info_label->setText("未输入有效的噪声级别！");
        return;
    }

    // 生成波形
    QLineSeries* newSeries = wave->Series_Create(waveType, period, length, noiseLevel, amplitude); // 生成波形
    wave->add_chart(ui->Wave_ChartView->chart(), newSeries, seriesName); // 将波形添加到图表并设置名称

    // 更新模型
    if (showModel) {
        showModel->appendRow(new QStandardItem(seriesName)); // 将新波形名称添加到模型
    }
    ui->info_label->setText(QString("%1 生成！").arg(seriesName));
    // 输出调试信息
    qDebug() << "生成波形:" << seriesName;
}

//删除波形
void Widget::on_delete_wave_pushButton_clicked()
{
    // 获取 show_columnView 的模型
    QStandardItemModel *showModel = qobject_cast<QStandardItemModel*>(ui->show_columnView->model());
    if (showModel) {
        // 获取选中的项
        QModelIndexList selectedIndexes = ui->show_columnView->selectionModel()->selectedIndexes();
        for (const QModelIndex &index : selectedIndexes) {
            if (index.isValid()) {
                QString chartName = showModel->item(index.row())->text(); // 获取选中的图表名称

                // 删除对应的波形
                for (auto series : ui->Wave_ChartView->chart()->series()) {
                    if (series->name() == chartName) {
                        series->setVisible(false); // 隐藏波形
                        break; // 找到并隐藏后退出循环
                    }
                }

                // 从 show_columnView 中移除选中的项
                showModel->removeRow(index.row());
                ui->info_label->setText(QString("%1 已删除").arg(chartName));
            }
        }
    }

    // 如果需要从 hide_columnView 中删除
    QStandardItemModel *hideModel = qobject_cast<QStandardItemModel*>(ui->hide_columnView->model());
    if (hideModel) {
        QModelIndexList selectedHideIndexes = ui->hide_columnView->selectionModel()->selectedIndexes();
        for (const QModelIndex &index : selectedHideIndexes) {
            if (index.isValid()) {
                QString chartName = hideModel->item(index.row())->text(); // 获取选中的图表名称

                // 删除对应的波形
                for (auto series : ui->Wave_ChartView->chart()->series()) {
                    if (series->name() == chartName) {
                        series->setVisible(false); // 隐藏波形
                        break; // 找到并隐藏后退出循环
                    }
                }

                // 从 hide_columnView 中移除选中的项
                hideModel->removeRow(index.row());
                ui->info_label->setText(QString("%1 已删除").arg(chartName));
            }
        }
    }
}

// 隐藏波形
void Widget::on_hide_wave_pushButton_clicked()
{
    // 获取 show_columnView 的模型
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->show_columnView->model());
    if (model) {
        // 获取 hide_columnView 的模型
        QStandardItemModel *hideModel = qobject_cast<QStandardItemModel*>(ui->hide_columnView->model());
        if (!hideModel) {
            hideModel = new QStandardItemModel(this); // 如果没有模型，则创建一个
            ui->hide_columnView->setModel(hideModel);
        }

        // 获取选中的项
        QModelIndexList selectedIndexes = ui->show_columnView->selectionModel()->selectedIndexes();
        for (const QModelIndex &index : selectedIndexes) {
            if (index.isValid()) {
                QString chartName = model->item(index.row())->text(); // 获取选中的图表名称

                // 隐藏对应的波形
                for (auto series : ui->Wave_ChartView->chart()->series()) {
                    if (series->name() == chartName) {
                        series->setVisible(false); // 隐藏波形
                        ui->info_label->setText(QString("%1 已隐藏").arg(chartName)); // 打印隐藏的波形名称
                        break; // 找到并隐藏后退出循环
                    }
                }

                // 从 show_columnView 中移除选中的项
                model->removeRow(index.row());

                // 将选中的项添加到 hide_columnView
                hideModel->appendRow(new QStandardItem(chartName)); // 添加到 hide_columnView 的模型
            }
        }
    }
}

//显示波形
void Widget::on_show_wave_pushButton_clicked()
{
    // 获取 hide_columnView 的模型
    QStandardItemModel *hideModel = qobject_cast<QStandardItemModel*>(ui->hide_columnView->model());
    if (hideModel) {
        // 获取 show_columnView 的模型
        QStandardItemModel *showModel = qobject_cast<QStandardItemModel*>(ui->show_columnView->model());
        if (!showModel) {
            showModel = new QStandardItemModel(this); // 如果没有模型，则创建一个
            ui->show_columnView->setModel(showModel);
        }

        // 获取选中的项
        QModelIndexList selectedIndexes = ui->hide_columnView->selectionModel()->selectedIndexes();
        for (const QModelIndex &index : selectedIndexes) {
            if (index.isValid()) {
                QString chartName = hideModel->item(index.row())->text(); // 获取选中的图表名称

                // 显示对应的波形
                for (auto series : ui->Wave_ChartView->chart()->series()) {
                    if (series->name() == chartName) {
                        series->setVisible(true); // 显示波形
                        break; // 找到并显示后退出循环
                    }
                }

                // 从 hide_columnView 中移除选中的项
                hideModel->removeRow(index.row());

                // 将选中的项添加到 show_columnView
                showModel->appendRow(new QStandardItem(chartName)); // 添加到 show_columnView 的模型
            }
        }
    }
}

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
    QString     stringdata;

  
    stringdata = QString(RecvBuff);   /*ascii显示*/
    
    ui->recv_textEdit->insertPlainText(stringdata);
    ui->recv_textEdit->moveCursor(QTextCursor::End);
    dataRxNumber += RecvBuff.length();
    ui->Rx_label->setText(QString("Rx: %1 bytes").arg(dataRxNumber));
    RecvBuff.clear();
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
    qDebug() << "主线程ID：" << QThread::currentThreadId();
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

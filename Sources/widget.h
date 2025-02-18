#ifndef WIDGET_H
#define WIDGET_H

#include <QDateTime>//时间
#include <QTimer>//定时器
#include <QElapsedTimer>//计时器
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <qcustomplot.h>
#include <QWidget>
#include "serialthread.h"
#include "plotthread.h"

#define TIMEOUT1S 1000
#define TIMEOUT2S 2000

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE


class CustomPlot; // 前向声明

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    virtual void timerEvent(QTimerEvent *event);//定时器事件

private slots:
    /********界面*********/
    void on_Clear_recvbuf_pushButton_clicked();//清空接收区信号槽
    void on_Clear_sendbuf_pushButton_clicked();//清空发送区信号槽
    void on_Save_pushButton_clicked();//保存数据信号槽
    void on_time_checkBox_clicked(bool checked);//时间显示信号槽
    void on_auto_reline_pushButton_clicked();//自动换行信号槽
    void on_recv_format_pushButton_clicked();//接收区格式化信号槽
    void on_send_format_pushButton_clicked();//发送区格式化信号槽
    void on_Auto_reconnect_checkBox_clicked(bool checked);//自动重连信号槽
    void on_Auto_roll_pushButton_clicked();//自动滚动信号槽
    /********串口*********/
    void RcvData(QByteArray RecvBuff);
    void updateInfoLabel(const QString &errorMessage);//串口错误信号上报槽
    void on_Addons_checkBox_clicked(bool checked);//打开/关闭插件信号槽
    void on_AUTOSEND_pushButton_clicked();//自动发送信号槽
    void on_SEND_pushButton_clicked();//发送信号槽
    void on_Opem_COM_pushButton_clicked();//打开串口信号槽
    void on_Serial_number_comboBox_clicked();//串口号信号槽
    /********信号处理*********/
    void ProcessData(QByteArray Recvbuff);//处理串口协议 分离出数据到图表渲染线程

signals:
    //信号量
    void SendData(QByteArray data);//串口接收到的数据
private:
    Ui::Widget *ui;
    /********串口*********/
    ComSerialPort *Serial;
    quint8  Serial_State = 0 ;      // 串口状态0关闭 1打开
    QString     SendTextStr;//发送文本
    quint32     dataRxNumber = 0 ;      // 记录Rx数据量
    quint32     dataTxNumber = 0 ;      // 记录Rx数据量
    /********  *********/
    void updateSizeLabel();//更新数据量
    void Update_RTC();//更新RTC
    void Update_SerialPort();//更新串口到ComboBox
    QTimer *timeUpdateTimer;//时间更新定时器
    void animateProgressBar(int startValue, int endValue, int duration); // 进度条动画
    void System_Init();//系统初始化
    int totalTextSize;//总数据量
    int autoSend_time;//自动发送时间
    /********标志位*********/
    bool autoScrollEnabled_Flag = false;//自动滚动标志
    bool autoReline_Flag = false;//自动换行标志
    bool autoReconnect_Flag = false;//自动重连标志
    bool showTime_Flag = false;//时间显示标志
    bool autoSend_Flag = false;//自动发送标志
    bool recv_format_Flag = false;//接收格式化标志
    bool send_format_Flag = false;//发送格式化标志
    /********图表*********/
    CustomPlot *customPlot; // 添加 customPlot 成员变量
    QTimer dataTimer;
    int dataCount=0;
};
#endif // WIDGET_H

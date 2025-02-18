/********************************************************************************
    * 文件名称 serialthread.h
    * 版     本：V1.0
    * 编写日期 ：2025-02-18
    * 功     能：串口收发线程
*********************************************************************************
V1.0 2025-02-18 First release @ZM
*********************************************************************************/
#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QThread>
#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QQueue>

class ComSerialPort : public QObject
{
    Q_OBJECT
public:
    explicit ComSerialPort(QString PortName,qint32 BaudRate,qint32* state,QObject *parent = nullptr);
    ~ComSerialPort();
    void CloseSerial();
    void handleSerialError(QSerialPort::SerialPortError error);

signals:
    void UpdateData(QByteArray data);//上传串口接收到的数据
    void UpdateError(const QString &errorMessage);//上传串口错误
    void DataToProcessingThread(QByteArray data); // 发送数据到数据处理线程
public slots:
    void RcvData();
    void SendSerialData(QByteArray data);

private:
    bool InitSerial(QString portname,qint32 BaudRate);
    QSerialPort *Serial;
    QThread* thread;
    QQueue<QByteArray> ringBuffer;
};


#endif // SERIALPORT_H

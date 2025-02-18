#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QThread>
#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>


class ComSerialPort : public QObject
{
    Q_OBJECT
public:
    explicit ComSerialPort(QString PortName,qint32 BaudRate,qint32* state,QObject *parent = nullptr);
    ~ComSerialPort();
    bool InitSerial(QString portname,qint32 BaudRate);
    void CloseSerial();
    void handleSerialError(QSerialPort::SerialPortError error);

signals:
    void UpdateData(QByteArray data);//上传串口接收到的数据
    void UpdateError(const QString &errorMessage);//上传串口错误

public slots:
    void RcvData();
    void SendSerialData(QByteArray data);

private:
    QSerialPort *Serial;
    QThread* thread;
};


#endif // SERIALPORT_H

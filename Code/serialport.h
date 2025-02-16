#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QThread>
#include <QObject>
#include <QSerialPort>


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
    void UpdateData(QByteArray data);
    void UpdateError(const QString &errorMessage);

public slots:
    void RcvData();
    void SendSerialData(QByteArray data);

private:
    QSerialPort *Serial;
    QThread* thread;
};


#endif // SERIALPORT_H

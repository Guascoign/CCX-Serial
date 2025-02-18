#include "serialthread.h"


ComSerialPort::ComSerialPort(QString PortName, qint32 BaudRate, qint32* state, QObject *parent) : QObject(parent)
{
    thread = new QThread();
    Serial = new QSerialPort();

    // 解析 PortName，提取实际的串口名称
    QString actualPortName = PortName.split(":").first(); // 获取 "COMX" 部分
    *state = InitSerial(actualPortName, BaudRate); // 使用实际的串口名称初始化

    this->moveToThread(thread);
    Serial->moveToThread(thread);
    thread->start();

    connect(thread, &QThread::finished, this, &QObject::deleteLater);
    // 连接串口错误信号
    connect(Serial, &QSerialPort::errorOccurred, this, &ComSerialPort::handleSerialError);
}

ComSerialPort::~ComSerialPort()
{

}

void ComSerialPort::handleSerialError(QSerialPort::SerialPortError error)
{
    switch (error)
    {
        case QSerialPort::DeviceNotFoundError:
            emit UpdateError("Error: 找不到设备。");
            break;
        case QSerialPort::PermissionError:
            emit UpdateError("Error: 没有足够的权限。");
            break;
        case QSerialPort::OpenError:
            emit UpdateError("Error: 打开设备时出错。");
            break;
        case QSerialPort::WriteError:
            emit UpdateError("Error: 写数据时出错。");
            break;
        case QSerialPort::ReadError:
            emit UpdateError("Error: 读数据时出错。");
            break;
        case QSerialPort::ResourceError:
            emit UpdateError("Error: 设备被意外移除或系统资源不足。");
            break;
        case QSerialPort::UnsupportedOperationError:
            emit UpdateError("Error: 不支持的操作。");
            break;
        case QSerialPort::UnknownError:
            emit UpdateError("Error: 未知错误。");
            break;
        case QSerialPort::TimeoutError:
            emit UpdateError("Error: 操作超时。");
            break;
        case QSerialPort::NotOpenError:
            emit UpdateError("Error: 尝试操作一个未打开的设备。");
            break;
        default:
            emit UpdateError("Error: 未知错误。");
            break;
    }
}

bool ComSerialPort::InitSerial(QString portname,qint32 BaudRate)
{
    Serial->setPortName(portname);
    if(!Serial->open(QIODevice::ReadWrite))
    {
        qDebug()<<"串口打开失败";
        return 0;
    }
    qDebug()<<"串口打开成功";
    Serial->setBaudRate(BaudRate);   // 默认115200
    Serial->setDataBits(QSerialPort::Data8);        // 默认
    Serial->setParity(QSerialPort::NoParity);
    Serial->setStopBits(QSerialPort::OneStop);

    // 读取数据
    connect(Serial,&QSerialPort::readyRead,this,&ComSerialPort::RcvData);

    return 1;
}

void ComSerialPort::CloseSerial()
{

    Serial->clear();
    Serial->close();

}

void ComSerialPort::RcvData()
{
    QByteArray buffer = Serial->readAll();
    qDebug() << "接收数据线程ID："<< QThread::currentThreadId() << "接收：" << buffer;
    emit  UpdateData(buffer);
}

void ComSerialPort::SendSerialData(QByteArray data)
{
    qDebug() << "发送数据线程ID："<< QThread::currentThreadId() << "发送：" << data;
    // 接收GUI数据并发送
    Serial->write(data);
    Serial->flush();
    Serial->waitForBytesWritten(10);
}


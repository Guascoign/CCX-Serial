/********************************************************************************
    * 文件名称 ：datahandelthread.cpp
    * 版     本：V1.0
    * 编写日期 ：2025-02-18
    * 功     能：数据处理线程
*********************************************************************************
V1.0 2025-02-18 First release @ZM
*********************************************************************************/
#include "datahandelthread.h"
#include <QDebug>
DataHandleThread::DataHandleThread(QObject *parent) : QThread(parent)
{
    qDebug() << "数处理线程ID：" << QThread::currentThreadId();
}

DataHandleThread::~DataHandleThread()
{
}

void DataHandleThread::handleData(const QByteArray &data)
{
    QString dataStr = QString::fromUtf8(data);
    QStringList dataList = dataStr.split(",");
    
    // 显示接收到的数据
    qDebug() << "数据列表：" << dataList;

    emit ProcessedData(dataList);
}

void DataHandleThread::run()
{
    exec();
}

/********************************************************************************
    * 文件名称 ：datahandelthread.h
    * 版     本：V1.0
    * 编写日期 ：2025-02-18
    * 功     能：数据处理线程
*********************************************************************************
V1.0 2025-02-18 First release @ZM
*********************************************************************************/
#ifndef DATAHANDELTHREAD_H
#define DATAHANDELTHREAD_H

#include <QThread>
#include <QStringList>

class DataHandleThread : public QThread
{
    Q_OBJECT
public:
    explicit DataHandleThread(QObject *parent = nullptr);
    ~DataHandleThread();

signals:
    void ProcessedData(const QStringList &dataList); // 处理完的数据

public slots:
    void handleData(const QByteArray &data); // 处理接收到的数据

protected:
    void run() override;
};

#endif // DATAHANDELTHREAD_H

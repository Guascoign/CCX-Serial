/********************************************************************************
    * 文件名称 plotthread.h
    * 版     本：V1.0
    * 编写日期 ：2025-02-18
    * 功     能：图表刷新线程
*********************************************************************************
V1.0 2025-02-18 First release @ZM
*********************************************************************************/
// 需要在主线程启动的时候开启图像刷新线程，新图表注册新定时器在线程中
// PlotThread *plotThread = new PlotThread(this);//开启图像刷新线程
// plotThread->start();

#ifndef PLOTTHREAD_H
#define PLOTTHREAD_H

#include <QThread>
#include <QTimer>
#include <QMap>
#include "customplot.h"

class CustomPlot;
class PlotThread : public QThread
{
    Q_OBJECT

public:
    explicit PlotThread(QObject *parent = nullptr);
    void addPlot(CustomPlot *plot); // 添加图像的方法
    void deletaPlot(CustomPlot *plot);//删除图像
    void removePlot(CustomPlot *plot);
protected:
    void run() override;
    void stop();

private slots:
    void handleRefreshRateChanged(CustomPlot *plot);//刷新时间修改，删除定时器重新生成新定时器
private:
    QMap<CustomPlot*, QTimer*> m_plots; // 图像和定时器的映射
};

#endif // PLOTTHREAD_H

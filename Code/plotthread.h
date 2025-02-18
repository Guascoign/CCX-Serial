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

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

protected:
    void run() override;
    void stop();

private slots:

private:
    QMap<CustomPlot*, QTimer*> m_plots; // 图像和定时器的映射
};

#endif // PLOTTHREAD_H

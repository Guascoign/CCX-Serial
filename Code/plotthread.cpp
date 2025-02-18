#include "plotthread.h"



PlotThread::PlotThread(QObject *parent) : QThread(parent)
{
}

void PlotThread::addPlot(CustomPlot *plot)
{
    qDebug() << "添加刷新定时器！" << plot << "刷新时间" << plot->rate << "跟踪模式" << plot->mode;
    QTimer *timer = new QTimer(this);

    // 添加图像刷新定时器
    connect(timer, &QTimer::timeout, [plot]() {
        plot->replot(); // 刷新图像
        
    });

    // 根据刷新率设置定时器间隔
    switch (plot->rate) {
        case Stop: {
            //timer->start(1); // 尽可能快地刷新
            break;
        }
        case Fast: {
            timer->start(1); // 尽可能快地刷新
            break;
        }
        case Rate_10ms: {
            timer->start(10); // 每10毫秒刷新一次
            break;
        }
        case Rate_20ms: {
            timer->start(20); // 每20毫秒刷新一次
            break;
        }
        case Rate_50ms: {
            timer->start(50); // 每50毫秒刷新一次
            break;
        }
        case Rate_100ms: {
            timer->start(100); // 每100毫秒刷新一次
            break;
        }
        case Rate_200ms: {
            timer->start(200); // 每200毫秒刷新一次
            break;
        }
        case Rate_500ms: {
            timer->start(500); // 每500毫秒刷新一次
            break;
        }
        case Rate_1s: {
            timer->start(1000); // 每秒刷新一次
            break;
        }
        case Rate_2s: {
            timer->start(2000); // 每2秒刷新一次
            break;
        }
        default: {
            timer->start(1000); // 默认每秒刷新一次
            break;
        }
    }

    m_plots.insert(plot, timer);
}

void PlotThread::run()
{
    qDebug() << "图像处理线程ID：" << QThread::currentThreadId();
    exec(); // 启动事件循环
}

void PlotThread::stop()
{
    // 这里可以添加停止线程的逻辑
}

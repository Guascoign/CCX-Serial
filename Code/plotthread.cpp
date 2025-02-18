/********************************************************************************
    * 文件名称 plotthread.cpp
    * 版     本：V1.0
    * 编写日期 ：2025-02-18
    * 功     能：图表刷新线程
*********************************************************************************
V1.0 2025-02-18 First release @ZM
*********************************************************************************/
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
        switch (plot->mode) {
            case Mode_None:
                // 不调整 X 轴范围
                break;
            case Mode_All:
                plot->rescaleAxes();
                break;
            case Mode_2s:
                plot->xAxis->setRange(plot->x_key - 2, plot->x_key); // 显示最后2s
                break;
            case Mode_5s:
                plot->xAxis->setRange(plot->x_key - 5, plot->x_key); // 显示最后5s
                break;
            case Mode_10s:
                plot->xAxis->setRange(plot->x_key - 10, plot->x_key); // 显示最后10s
                break;
            default:
                break;
        }
        plot->replot(); // 刷新图像
    });

    // 根据刷新率设置定时器间隔
    switch (plot->rate) {
        case Stop: {
            // 删除定时器
            delete timer;
            return; // 直接返回，不插入到 m_plots 中
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
     // 连接刷新率改变信号
     connect(plot, &CustomPlot::refreshRateChanged, this, &PlotThread::handleRefreshRateChanged);
}

void PlotThread::removePlot(CustomPlot *plot)
{
    if (m_plots.contains(plot)) {
        QTimer *timer = m_plots.take(plot);
        timer->stop();
        delete timer;
        qDebug() << "删除刷新定时器！" << plot;
    }
}

void PlotThread::run()
{
    qDebug() << "图像处理线程ID：" << QThread::currentThreadId();
    exec(); // 启动事件循环
}

void PlotThread::stop()
{
    // 停止所有定时器并清空 m_plots
    for (auto timer : m_plots) {
        timer->stop();
        delete timer;
    }
    m_plots.clear();
    qDebug() << "停止所有刷新定时器！";
}


void PlotThread::handleRefreshRateChanged(CustomPlot *plot)
{
    qDebug() << "handleRefreshRateChanged触发" << this ;
    removePlot(plot);
    addPlot(plot);
}


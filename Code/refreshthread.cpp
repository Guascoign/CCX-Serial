#include "refreshthread.h"
#include "customplot.h"
#include <QDebug>
#include <QTimer>

RefreshThread::RefreshThread(CustomPlot *plot, int interval, QObject *parent)
    : QThread(parent), m_plot(plot), m_interval(interval), m_running(true)
{
    
}

void RefreshThread::setTrackingMode(int mode)
{
    m_trackingMode = mode;
}

void RefreshThread::run()
{
    qDebug() << "刷新图表线程ID：" << QThread::currentThreadId();
    m_running = true;
    static QTime timeStart = QTime::currentTime(); // 初始化时间
    
    while (m_running) {
        QMetaObject::invokeMethod(m_plot, [this]() {
            if (!m_plot->selectedGraphs().isEmpty()) {
                // 如果有选中的波形，不更新波形
                return;
            }
            switch (m_trackingMode) {
                case 0:
                    // 不跟踪，不做任何操作
                    break;
                case 1:
                if (!m_plot->graph(0)->data()->isEmpty()) {
                    double currentTime = timeStart.msecsTo(QTime::currentTime()) / 1000.0; // 使用时间获取当前时间
                    m_plot->xAxis->setRange( 0 , currentTime);
                }
                    break;
                case 2:
                    if (!m_plot->graph(0)->data()->isEmpty()) {
                        double currentTime = timeStart.msecsTo(QTime::currentTime()) / 1000.0; // 使用时间获取当前时间
                        m_plot->xAxis->setRange(currentTime - 8 , currentTime);
                    }
                    break;
                case 3:
                    // 添加 Mode3 的逻辑
                    break;
                case 4:
                    // 添加 Mode4 的逻辑
                    break;
            }
            m_plot->replot();
        });
        qDebug() << "refreshed";
        QThread::msleep(m_interval);
    }
}


void RefreshThread::stop()
{
    qDebug() << "关闭图表线程ID：" << QThread::currentThreadId();
    m_running = false;
}

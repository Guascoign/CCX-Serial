#ifndef REFRESHTHREAD_H
#define REFRESHTHREAD_H

#include <QThread>
#include "customplot.h"

class CustomPlot; // 前向声明
class RefreshThread : public QThread
{
    Q_OBJECT

public:
    explicit RefreshThread(CustomPlot *plot, int interval, QObject *parent = nullptr);
    void run() override;
    void stop();
    void setTrackingMode(int mode); 

private:
    CustomPlot *m_plot;
    int m_interval;
    bool m_running;
    int m_trackingMode;
};

#endif // REFRESHTHREAD_H

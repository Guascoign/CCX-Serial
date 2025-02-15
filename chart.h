#ifndef CHART_H
#define CHART_H

#include <QChart>

QT_FORWARD_DECLARE_CLASS(QGestureEvent)

class Chart : public QChart

{
public:
    explicit Chart(QGraphicsItem *parent = nullptr, Qt::WindowFlags wFlags = {});

protected:
    bool sceneEvent(QEvent *event);

private:
    bool gestureEvent(QGestureEvent *event);
};

#endif

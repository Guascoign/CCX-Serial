#ifndef CHARTVIEW_H
#define CHARTVIEW_H

#include <QChartView>
#include <QRubberBand>

class ChartView : public QChartView

{
    Q_OBJECT
public:
ChartView(QWidget *parent = nullptr);
ChartView(QChart *chart, QWidget *parent = nullptr);
QStringList getChartNames();
protected:
    bool viewportEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void wheelEvent(QWheelEvent *event);        //鼠标滚轮事件,缩放


private:
    bool m_isTouching = false;
    bool m_isDragging = false; // 是否正在拖动
    QPoint m_dragStartPos; // 鼠标按下时的位置
};

#endif

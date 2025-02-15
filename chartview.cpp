#include "chartview.h"
#include <QChart>
#include <QMouseEvent>

ChartView::ChartView(QChart *chart, QWidget *parent)
    : QChartView(chart, parent)
{
    setRubberBand(QChartView::RectangleRubberBand);
}

ChartView::ChartView(QWidget *parent)
{
    setRubberBand(QChartView::RectangleRubberBand);
}

bool ChartView::viewportEvent(QEvent *event)
{
    if (event->type() == QEvent::TouchBegin) {
        // 默认情况下，触摸事件将转换为鼠标事件。所以
        // 活动结束后，我们也将获得鼠标活动，但我们想要
        // 仅作为手势处理触摸事件。所以我们需要这个保障
        // 阻止鼠标事件实际上是从触摸生成的。

        m_isTouching = true;

        // 处理手势时关闭动画
        // 只会减慢我们的速度。

        chart()->setAnimationOptions(QChart::NoAnimation);
    }
    return QChartView::viewportEvent(event);
}

void ChartView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        m_isDragging = true; // 开始拖动
        m_dragStartPos = event->pos(); // 记录鼠标按下时的位置
    } else if (!m_isTouching) {
        QChartView::mousePressEvent(event);
    }
}

void ChartView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_isDragging) {
        QPoint delta = event->pos() - m_dragStartPos; // 计算移动的距离
        chart()->scroll(-delta.x(), delta.y()); // 根据移动的距离滚动图表
        m_dragStartPos = event->pos(); // 更新鼠标位置
    } else if (!m_isTouching) {
        QChartView::mouseMoveEvent(event);
    }
}

void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        m_isDragging = false; // 停止拖动
    } else if (m_isTouching) {
        m_isTouching = false;
        chart()->setAnimationOptions(QChart::SeriesAnimations);
    }

    QChartView::mouseReleaseEvent(event);
}

void ChartView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Plus:
        chart()->zoomIn();
        break;
    case Qt::Key_Minus:
        chart()->zoomOut();
        break;
    case Qt::Key_Left:
        chart()->scroll(-10, 0);
        break;
    case Qt::Key_Right:
        chart()->scroll(10, 0);
        break;
    case Qt::Key_Up:
        chart()->scroll(0, 10);
        break;
    case Qt::Key_Down:
        chart()->scroll(0, -10);
        break;
    //空格还原缩放比例
    case Qt::Key_Space:
        chart()->zoomReset();
        break;
    default:
        QGraphicsView::keyPressEvent(event);
        break;
    }
}

void ChartView::wheelEvent(QWheelEvent *event)
{//鼠标滚轮事件处理，缩放
    QPoint numDegrees = event->angleDelta() / 8;
    if (!numDegrees.isNull())
    {
        QPoint numSteps = numDegrees / 15;  //步数
        int stepY=numSteps.y();     //垂直方向滚轮的滚动步数
        if (stepY >0)       //大于0，前向滚动，放大
            chart()->zoom(1.1*stepY);
        else
            chart()->zoom(-0.9*stepY);
    }
    event->accept();
}


QStringList ChartView::getChartNames()
{
    QStringList names;
    if (chart()) { // 确保图表存在
        for (auto series : chart()->series()) {
            names.append(series->name()); // 获取每个系列的名称并添加到列表
        }
    }
    return names; // 返回名称列表
}
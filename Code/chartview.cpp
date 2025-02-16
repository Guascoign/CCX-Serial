#include "chartview.h"
#include <QChart>
#include <QMouseEvent>
#include <QToolTip>
#include <QValueAxis>
#include <QLineSeries>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>

ChartView::ChartView(QChart *chart, QWidget *parent)
    : QChartView(chart, parent), m_isDragging(false), m_isTouching(false), m_marker(nullptr), m_markerVisible(false)
{
    setRubberBand(QChartView::RectangleRubberBand);
}

ChartView::ChartView(QWidget *parent)
    : QChartView(parent), m_isDragging(false), m_isTouching(false), m_marker(nullptr), m_markerVisible(false)
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
        return; // 直接返回，防止进一步处理
    } else if (!m_isTouching) {
        QChartView::mouseMoveEvent(event);
    }

    if (!m_markerVisible) return;

    // 新增代码：显示实时数据和圆点
    QPointF chartPos = chart()->mapToValue(event->pos());
    QString tooltipText = QString("X: %1, Y: %2").arg(chartPos.x(), 0, 'f', 2).arg(chartPos.y(), 0, 'f', 2);
    QToolTip::showText(event->globalPos(), tooltipText, this);

    if (!m_marker) {
        m_marker = new QGraphicsEllipseItem();
        m_marker->setBrush(Qt::red);
        chart()->scene()->addItem(m_marker);
    }

    // 查找最近的点
    QPointF closestPoint;
    qreal minDistance = std::numeric_limits<qreal>::max();
    for (auto series : chart()->series()) {
        QLineSeries *lineSeries = qobject_cast<QLineSeries *>(series);
        if (lineSeries) {
            for (const QPointF &point : lineSeries->points()) {
                qreal distance = std::hypot(point.x() - chartPos.x(), point.y() - chartPos.y());
                if (distance < minDistance) {
                    minDistance = distance;
                    closestPoint = point;
                }
            }
        }
    }

    // 更新标记位置
    QPointF markerPos = chart()->mapToPosition(closestPoint);
    m_marker->setRect(markerPos.x() - 2, markerPos.y() - 2, 4, 4);

    // 更新工具提示
    tooltipText = QString("X: %1, Y: %2").arg(closestPoint.x(), 0, 'f', 2).arg(closestPoint.y(), 0, 'f', 2);
    QToolTip::showText(event->globalPos(), tooltipText, this);
}

void ChartView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MiddleButton) {
        m_isDragging = false; // 停止拖动
    } else if (m_isTouching) {
        m_isTouching = false;
        chart()->setAnimationOptions(QChart::SeriesAnimations);
    }

    if (event->button() == Qt::LeftButton && m_dragStartPos != event->pos()) {
        QRectF rubberBandRect = this->rubberBandRect();
        if (rubberBandRect.width() > 0 && rubberBandRect.height() > 0) {
            if (m_dragStartPos.y() < event->pos().y()) {
                chart()->zoomIn(rubberBandRect);
            } else {
                chart()->zoomOut();
            }
        }
    }

    QChartView::mouseReleaseEvent(event);
}

void ChartView::keyPressEvent(QKeyEvent *event)
{
    if (m_isDragging) {
        return; // 如果正在拖动，忽略按键事件
    }

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
        centerChart(); // 调用居中方法
        break;
    case Qt::Key_Q:
        m_markerVisible = !m_markerVisible;
        if (m_marker) {
            m_marker->setVisible(m_markerVisible);
        }
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

void ChartView::centerChart()
{
    if (!chart()) return;

    qreal minX = std::numeric_limits<qreal>::max();
    qreal maxX = std::numeric_limits<qreal>::min();
    qreal minY = std::numeric_limits<qreal>::max();
    qreal maxY = std::numeric_limits<qreal>::min();

    for (auto series : chart()->series()) {
        QLineSeries *lineSeries = qobject_cast<QLineSeries *>(series);
        if (lineSeries) {
            for (const QPointF &point : lineSeries->points()) {
                if (point.x() < minX) minX = point.x();
                if (point.x() > maxX) maxX = point.x();
                if (point.y() < minY) minY = point.y();
                if (point.y() > maxY) maxY = point.y();
            }
        }
    }

    qreal centerX = (minX + maxX) / 2;
    qreal centerY = (minY + maxY) / 2;

    QPointF centerPoint = chart()->mapToPosition(QPointF(centerX, centerY));
    QPointF plotAreaCenter = chart()->plotArea().center();

    chart()->scroll(centerPoint.x() - plotAreaCenter.x(), centerPoint.y() - plotAreaCenter.y());
}
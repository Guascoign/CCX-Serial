#include "customplot.h"
#include "refreshthread.h"
#include <QKeyEvent>
#include <QMenu>
#include <QAction>
#include <QColorDialog>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QDialogButtonBox>

CustomPlot::CustomPlot(QWidget *parent) : QCustomPlot(parent), measurementEnabled(false), refreshThread(nullptr), trackingMode(2)
{
    setInteraction(QCP::iRangeDrag, true); // 启用左键拖拽
    setInteraction(QCP::iRangeZoom, true); // 启用滚轮放大缩小
    setInteraction(QCP::iSelectPlottables, true); // 启用选择曲线
    setInteraction(QCP::iSelectLegend, true); // 启用选择图例

    legend->setVisible(true); // 默认显示图例
    
    toggleGraphNameAction = new QAction("隐藏图例", this);
    connect(toggleGraphNameAction, &QAction::triggered, [this]() {
        bool visible = !legend->visible();
        legend->setVisible(visible);
        toggleGraphNameAction->setText(visible ? "隐藏图例" : "显示图例");
        replot();
    });

    toggleScatterPointsAction = new QAction("显示散点", this);
    connect(toggleScatterPointsAction, &QAction::triggered, this, &CustomPlot::toggleScatterPoints);

    toggleGridLinesAction = new QAction("显示网格", this);
    connect(toggleGridLinesAction, &QAction::triggered, this, &CustomPlot::toggleGridLines);

    toggleLineVisibilityAction = new QAction("隐藏连线", this);
    connect(toggleLineVisibilityAction, &QAction::triggered, this, &CustomPlot::toggleLineVisibility);

    toggleSelectedGraphVisibilityAction = new QAction("隐藏曲线", this);
    connect(toggleSelectedGraphVisibilityAction, &QAction::triggered, this, &CustomPlot::toggleSelectedGraphVisibility);

    changeSelectedGraphColorAction = new QAction("修改颜色", this);
    connect(changeSelectedGraphColorAction, &QAction::triggered, this, &CustomPlot::changeSelectedGraphColor);

    toggleMeasurementAction = new QAction("开启测量", this);
    connect(toggleMeasurementAction, &QAction::triggered, this, &CustomPlot::toggleMeasurement);

    changeSelectedGraphWidthMenu = new QMenu("设置粗细", this);
    QAction *thinAction = changeSelectedGraphWidthMenu->addAction("细");
    QAction *mediumAction = changeSelectedGraphWidthMenu->addAction("中");
    QAction *thickAction = changeSelectedGraphWidthMenu->addAction("粗");

    connect(thinAction, &QAction::triggered, [this]() { changeSelectedGraphWidth(1); });
    connect(mediumAction, &QAction::triggered, [this]() { changeSelectedGraphWidth(3); });
    connect(thickAction, &QAction::triggered, [this]() { changeSelectedGraphWidth(5); });

    tracer = new QCPItemTracer(this);
    tracer->setVisible(false);
    tracer->setStyle(QCPItemTracer::tsCircle);
    tracer->setPen(QPen(Qt::red));
    tracer->setBrush(Qt::red);
    tracer->setSize(7);

    verticalLine = new QCPItemLine(this);
    verticalLine->setVisible(false);
    verticalLine->setPen(QPen(Qt::DashLine));

    horizontalLine = new QCPItemLine(this);
    horizontalLine->setVisible(false);
    horizontalLine->setPen(QPen(Qt::DashLine));

    measurementText = new QCPItemText(this);
    measurementText->setVisible(false);
    measurementText->setPositionAlignment(Qt::AlignBottom | Qt::AlignRight);
    measurementText->position->setType(QCPItemPosition::ptAxisRectRatio);
    measurementText->position->setCoords(0.95, 0.95);
    measurementText->setText("X: 0, Y: 0");

    title = new QCPTextElement(this, "图表", QFont("sans", 12, QFont::Bold));
    plotLayout()->insertRow(0);
    plotLayout()->addElement(0, 0, title);

    connect(title, &QCPTextElement::doubleClicked, this, &CustomPlot::editTitle);

    connect(this, &QCustomPlot::legendDoubleClick, this, [this](QCPLegend *legend, QCPAbstractLegendItem *item) {
        Q_UNUSED(legend)
        if (item) {
            QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem *>(item);
            bool ok;
            QString newName = QInputDialog::getText(this, "修改曲线名称", "新名称:", QLineEdit::Normal, plItem->plottable()->name(), &ok);
            if (ok) {
                plItem->plottable()->setName(newName);
                replot();
            }
        }
    });

    connect(this, &QCustomPlot::selectionChangedByUser, this, &CustomPlot::updateGraphLegend);

    connect(this, &QCustomPlot::legendClick, this, [this](QCPLegend *legend, QCPAbstractLegendItem *item, QMouseEvent *event) {
        Q_UNUSED(legend)
        Q_UNUSED(event)
        if (item) {
            QCPPlottableLegendItem *plItem = qobject_cast<QCPPlottableLegendItem *>(item);
            if (plItem) {
                QCPGraph *graph = qobject_cast<QCPGraph *>(plItem->plottable());
                if (graph) {
                    graph->setSelection(QCPDataSelection(graph->data()->dataRange()));
                    replot();
                }
            }
        }
    });

    refreshRateMenu = new QMenu("设置刷新速率", this);
    QAction *noRefreshAction = refreshRateMenu->addAction("不刷新");
    QAction *refresh10msAction = refreshRateMenu->addAction("10ms");
    QAction *refresh20msAction = refreshRateMenu->addAction("20ms");
    QAction *refresh50msAction = refreshRateMenu->addAction("50ms");
    QAction *refresh100msAction = refreshRateMenu->addAction("100ms");
    QAction *refresh200msAction = refreshRateMenu->addAction("200ms");
    QAction *refresh500msAction = refreshRateMenu->addAction("500ms");
    QAction *refresh1sAction = refreshRateMenu->addAction("1s");
    QAction *refresh2sAction = refreshRateMenu->addAction("2s");

    connect(noRefreshAction, &QAction::triggered, [this]() { setRefreshRate(0); });
    connect(refresh10msAction, &QAction::triggered, [this]() { setRefreshRate(10); });
    connect(refresh20msAction, &QAction::triggered, [this]() { setRefreshRate(20); });
    connect(refresh50msAction, &QAction::triggered, [this]() { setRefreshRate(50); });
    connect(refresh100msAction, &QAction::triggered, [this]() { setRefreshRate(100); });
    connect(refresh200msAction, &QAction::triggered, [this]() { setRefreshRate(200); });
    connect(refresh500msAction, &QAction::triggered, [this]() { setRefreshRate(500); });
    connect(refresh1sAction, &QAction::triggered, [this]() { setRefreshRate(1000); });
    connect(refresh2sAction, &QAction::triggered, [this]() { setRefreshRate(2000); });

    trackingModeMenu = new QMenu("跟踪模式", this);
    QAction *noTrackingAction = trackingModeMenu->addAction("不跟踪");
    QAction *globalTrackingAction = trackingModeMenu->addAction("全局跟踪");
    QAction *last8PointsTrackingAction = trackingModeMenu->addAction("8S");

    connect(noTrackingAction, &QAction::triggered, [this]() { setTrackingMode(0); });
    connect(globalTrackingAction, &QAction::triggered, [this]() { setTrackingMode(1); });
    connect(last8PointsTrackingAction, &QAction::triggered, [this]() { setTrackingMode(2); });
    

}

CustomPlot::~CustomPlot()
{
    if (refreshThread) {
        refreshThread->stop();
        refreshThread->wait();
        delete refreshThread;
        refreshThread = nullptr;
    }
}

void CustomPlot::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space) {
        resetPlotScale();
    } else {
        QCustomPlot::keyPressEvent(event);
    }
}

void CustomPlot::resetPlotScale()
{
    rescaleAxes();
    replot();
}

void CustomPlot::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    if (selectedGraphs().isEmpty()) {
        menu.addAction(toggleGraphNameAction);
        menu.addAction(toggleMeasurementAction);
        menu.addMenu(refreshRateMenu);
        menu.addMenu(trackingModeMenu);
    } else {
        menu.addMenu(changeSelectedGraphWidthMenu);
        menu.addAction(toggleSelectedGraphVisibilityAction);
        menu.addAction(changeSelectedGraphColorAction);
        menu.addAction(toggleLineVisibilityAction);
        menu.addAction(toggleScatterPointsAction);
    }
    menu.exec(event->globalPos());
}

void CustomPlot::toggleScatterPoints()
{
    bool visible = false;
    for (QCPGraph *graph : selectedGraphs()) {
        visible = graph->scatterStyle().shape() == QCPScatterStyle::ssNone;
        graph->setScatterStyle(visible ? QCPScatterStyle::ssCircle : QCPScatterStyle::ssNone);
    }
    toggleScatterPointsAction->setText(visible ? "隐藏散点" : "显示散点");
    replot();
}

void CustomPlot::toggleGridLines()
{
    bool visible = !xAxis->grid()->visible();
    xAxis->grid()->setVisible(visible);
    yAxis->grid()->setVisible(visible);
    toggleGridLinesAction->setText(visible ? "隐藏网格" : "显示网格");
    replot();
}

void CustomPlot::toggleLineVisibility()
{
    bool visible = false;
    for (QCPGraph *graph : selectedGraphs()) {
        visible = graph->lineStyle() == QCPGraph::lsNone;
        graph->setLineStyle(visible ? QCPGraph::lsLine : QCPGraph::lsNone);
    }
    toggleLineVisibilityAction->setText(visible ? "隐藏连线" : "显示连线");
    replot();
}

void CustomPlot::toggleSelectedGraphVisibility()
{
    for (QCPGraph *graph : selectedGraphs()) {
        graph->setVisible(!graph->visible());
        toggleSelectedGraphVisibilityAction->setText(graph->visible() ? "隐藏曲线" : "显示曲线");
    }
    replot();
    updateGraphLegend();
}

void CustomPlot::changeSelectedGraphColor()
{
    for (QCPGraph *graph : selectedGraphs()) {
        QColor color = QColorDialog::getColor(graph->pen().color(), this, "选择颜色");
        if (color.isValid()) {
            graph->setPen(QPen(color));
        }
    }
    replot();
}

void CustomPlot::toggleMeasurement()
{
    measurementEnabled = !measurementEnabled;
    tracer->setVisible(measurementEnabled);
    verticalLine->setVisible(measurementEnabled);
    horizontalLine->setVisible(measurementEnabled);
    measurementText->setVisible(measurementEnabled);
    toggleMeasurementAction->setText(measurementEnabled ? "关闭测量" : "开启测量");
    replot();
}

void CustomPlot::mouseMoveEvent(QMouseEvent *event)
{
    if (measurementEnabled) {
        double x = xAxis->pixelToCoord(event->pos().x());
        double y = yAxis->pixelToCoord(event->pos().y());

        tracer->position->setCoords(x, y);
        verticalLine->start->setCoords(x, yAxis->range().lower);
        verticalLine->end->setCoords(x, yAxis->range().upper);
        horizontalLine->start->setCoords(xAxis->range().lower, y);
        horizontalLine->end->setCoords(xAxis->range().upper, y);

        if (!selectedGraphs().isEmpty()) {
            QCPGraph *graph = selectedGraphs().first();
            tracer->setGraph(graph);
            tracer->setGraphKey(x);
            y = tracer->position->value();
        }

        measurementText->setText(QString("X: %1, Y: %2").arg(tracer->position->key()).arg(tracer->position->value()));
        replot();
    }
    QCustomPlot::mouseMoveEvent(event);
}

void CustomPlot::updateGraphLegend()
{
    for (int i = 0; i < legend->itemCount(); ++i) {
        QCPPlottableLegendItem *item = qobject_cast<QCPPlottableLegendItem *>(legend->item(i));
        if (item) {
            QCPGraph *graph = qobject_cast<QCPGraph *>(item->plottable());
            if (graph) {
                QFont font = item->font();
                font.setItalic(!graph->visible());
                item->setFont(font);
                item->setTextColor(graph->visible() ? Qt::black : Qt::gray);
            }
        }
    }
    replot();
}

void CustomPlot::changeSelectedGraphWidth(int width)
{
    for (QCPGraph *graph : selectedGraphs()) {
        QPen pen = graph->pen();
        pen.setWidth(width);
        graph->setPen(pen);
    }
    replot();
}

void CustomPlot::editTitle()
{
    QDialog dialog(this);
    dialog.setWindowTitle("修改图表和坐标轴名称");

    QVBoxLayout layout(&dialog);

    QLineEdit titleEdit(title->text());
    QLineEdit xLabelEdit(xAxis->label());
    QLineEdit yLabelEdit(yAxis->label());

    layout.addWidget(new QLabel("图表名称:"));
    layout.addWidget(&titleEdit);
    layout.addWidget(new QLabel("X轴名称:"));
    layout.addWidget(&xLabelEdit);
    layout.addWidget(new QLabel("Y轴名称:"));
    layout.addWidget(&yLabelEdit);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    layout.addWidget(&buttonBox);

    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        title->setText(titleEdit.text());
        xAxis->setLabel(xLabelEdit.text());
        yAxis->setLabel(yLabelEdit.text());
        replot();
    }
}

void CustomPlot::setTrackingMode(int mode)
{
    trackingMode = mode;
    if (refreshThread) {
        refreshThread->setTrackingMode(mode);
    }
}

void CustomPlot::setRefreshRate(int interval)
{
    if (refreshThread) {
        refreshThread->stop();
        refreshThread->wait();
        delete refreshThread;
        refreshThread = nullptr;
    }

    if (interval > 0) {
        refreshThread = new RefreshThread(this, interval, this);
        refreshThread->setTrackingMode(trackingMode);
        refreshThread->start();
    }
}

void CustomPlot::wheelEvent(QWheelEvent *event)
{
    if (trackingMode == 1 || trackingMode == 2) {
        // 只缩放Y轴
        double factor = event->angleDelta().y() > 0 ? 0.9 : 1.1;
        yAxis->scaleRange(factor, yAxis->pixelToCoord(event->position().y()));
        replot();
    } else {
        // 默认行为
        QCustomPlot::wheelEvent(event);
    }
}

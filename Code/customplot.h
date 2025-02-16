#ifndef CUSTOMPLOT_H
#define CUSTOMPLOT_H

#include <QWidget>
#include <qcustomplot.h>
#include "refreshthread.h"

class RefreshThread; 
class CustomPlot : public QCustomPlot
{
    Q_OBJECT

public:
    explicit CustomPlot(QWidget *parent = nullptr);
    ~CustomPlot(); // 声明析构函数
    void setTrackingMode(int mode); 

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override; // 声明 wheelEvent 方法

private slots:
    void resetPlotScale();
    void toggleScatterPoints();
    void toggleGridLines();
    void toggleLineVisibility();
    void toggleSelectedGraphVisibility();
    void changeSelectedGraphColor();
    void toggleMeasurement();
    void updateGraphLegend();
    void editTitle();
    void changeSelectedGraphWidth(int width);
    void setRefreshRate(int interval);

private:
    QAction *toggleGraphNameAction;
    QAction *toggleScatterPointsAction;
    QAction *toggleGridLinesAction;
    QAction *toggleLineVisibilityAction;
    QAction *toggleSelectedGraphVisibilityAction;
    QAction *changeSelectedGraphColorAction;
    QAction *toggleMeasurementAction;
    QMenu *changeSelectedGraphWidthMenu;
    QMenu *refreshRateMenu;
    QMenu *trackingModeMenu;

    bool measurementEnabled;
    QCPItemTracer *tracer;
    QCPItemLine *verticalLine;
    QCPItemLine *horizontalLine;
    QCPItemText *measurementText;
    QCPTextElement *title;
    RefreshThread *refreshThread;
    int trackingMode; // 使用整数代替枚举
};

#endif // CUSTOMPLOT_H


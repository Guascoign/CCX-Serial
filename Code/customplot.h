/********************************************************************************
    * 文件名称 customplot.h
    * 版     本：V1.0
    * 编写日期 ：2025-02-18
    * 功     能：重写QCustmPlot
*********************************************************************************
V1.0 2025-02-18 First release @ZM
*********************************************************************************/
#ifndef CUSTOMPLOT_H
#define CUSTOMPLOT_H

#include <QWidget>
#include <qcustomplot.h>
#include "plotthread.h"

enum Refresh_rate{//刷新率设置
    Stop,       //停止
    Fast,       //尽可能快
    Rate_10ms,  //10ms刷新一次
    Rate_20ms,
    Rate_50ms,
    Rate_100ms,
    Rate_200ms,
    Rate_500ms,
    Rate_1s,
    Rate_2s
};

enum Track_mode{//跟踪模式设置
    Mode_None,//不跟踪波形
    Mode_All,//波形全局缩放
    Mode_2s,//只显示最后2s
    Mode_5s,//只显示最后5s
    Mode_10s//只显示最后10s
};

enum XAxis_mode{//x轴格式
    DTime,//变化时间
    Time,//实时时间
    Dot//点数
};
class CustomPlot : public QCustomPlot
{
    Q_OBJECT

public:
    explicit CustomPlot(QWidget *parent = nullptr);
    ~CustomPlot(); // 声明析构函数
    void setTrackingMode(int mode); 
    Track_mode mode = Mode_5s;
    Refresh_rate rate = Rate_10ms;
    XAxis_mode X_mode = DTime;
    double x_key = 0;//当前x轴坐标
    void setRefreshRate(Refresh_rate rate);
    void setTrackingMode(Track_mode mode);
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

signals:
    void refreshRateChanged(CustomPlot *plot);
    void trackingModeChanged(CustomPlot *plot);
    void plotConstructed(CustomPlot *plot);
    void deleteplot(CustomPlot *plot);

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
    QMenu *trackModeMenu;

    bool measurementEnabled;
    QCPItemTracer *tracer;
    QCPItemLine *verticalLine;
    QCPItemLine *horizontalLine;
    QCPItemText *measurementText;
    QCPTextElement *title;
};

#endif // CUSTOMPLOT_H


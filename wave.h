#ifndef WAVE_H
#define WAVE_H

#include <QObject>
#include <QThread>
#include <QWidget>
#include <QChartView>
#include <QRubberBand>
#include <QLineSeries>
#include "chart.h"
#include "chartview.h"

class Wave : public QObject
{
    Q_OBJECT
public:
    Wave();
    ~Wave();

    QLineSeries* Series_Create(const QString &waveType, double period, double length, double noiseLevel, double amplitude); // 波形类型、周期、长度、噪声、幅度
    void add_chart(QChart *chart, QLineSeries *series, const QString &name); // 添加波形到图表并设置名称
    void delete_chart(QChart *chart, QLineSeries *series); // 从图表中删除波形
};

#endif // WAVE_H
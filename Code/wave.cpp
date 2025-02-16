#include "wave.h"
#include <QLineSeries>
#include <QRandomGenerator>
#include <QChart>

Wave::Wave() 
{

}

Wave::~Wave() 
{

}

QLineSeries* Wave::Series_Create(const QString &waveType, double period, double length, double noiseLevel, double amplitude)
{
    auto series = new QLineSeries(); // 创建新的 QLineSeries 对象

    for (double i = 0; i < length; i += 0.1) {
        double value = 0;

        if (waveType == "正弦波") {
            value = amplitude * qSin((2 * M_PI / period) * i);
        } else if (waveType == "三角波") {
            value = (static_cast<int>(i / period) % 2 == 0) ? amplitude : -amplitude;
        } else if (waveType == "方波") {
            value = (2 * amplitude / period) * (i - period * static_cast<int>(i / period)) - amplitude;
        }

        // 添加更分散的噪声
        double noise = (QRandomGenerator::global()->bounded(noiseLevel)); // 使噪声与幅度相关
        value += noise; // 将噪声添加到值中

        series->append(i, value); // 将计算的值添加到 series
    }

    return series; // 返回生成的 QLineSeries 对象
}

void Wave::add_chart(QChart *chart, QLineSeries *series, const QString &name)
{
    if (chart && series) {
        series->setName(name); // 设置波形名称
        chart->addSeries(series); // 将波形添加到图表
        chart->createDefaultAxes(); // 创建默认坐标轴
        chart->setAnimationOptions(QChart::SeriesAnimations); // 设置动画选项
    }
}

void Wave::delete_chart(QChart *chart, QLineSeries *series)
{
    if (chart && series) {
        chart->removeSeries(series); // 从图表中删除波形
        delete series; // 删除曲线对象
    }
}
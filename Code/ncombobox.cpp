#include "ncombobox.h"

NComboBox::NComboBox(QWidget *parent) : QComboBox(parent)
{
    connect(this, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &NComboBox::onCurrentIndexChanged);
}

NComboBox::~NComboBox()
{

}

void NComboBox::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit clicked();
    }
    QComboBox::mousePressEvent(event);
}

void NComboBox::onCurrentIndexChanged(int index)
{
    emit itemSelected(index); // 发出选中项改变信号
}
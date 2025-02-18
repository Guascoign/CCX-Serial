/********************************************************************************
    * 文件名称 ncombobox.
    * 版     本：V1.0
    * 编写日期 ：2025-02-18
    * 功     能：重写Qncombobox
*********************************************************************************
V1.0 2025-02-18 First release @ZM
*********************************************************************************/
#ifndef NCOMBOBOX_H
#define NCOMBOBOX_H

#include <QComboBox>
#include <QMouseEvent>

class NComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit NComboBox(QWidget *parent = nullptr);
    ~NComboBox();

signals:
    void clicked(); // 鼠标点击信号
    void itemSelected(int index); // 选中项改变信号

protected:
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onCurrentIndexChanged(int index); // 处理选中项改变

};

#endif // NCOMBOBOX_H
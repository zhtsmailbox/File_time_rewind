#include "ProcessComboBox.h"

ProcessComboBox::ProcessComboBox(QWidget *parent) : QComboBox(parent) {}

void ProcessComboBox::showPopup()
{
    emit aboutToShowPopup();  // 发信号通知外部刷新
    QComboBox::showPopup();
}

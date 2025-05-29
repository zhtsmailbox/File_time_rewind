#ifndef PROCESSCOMBOBOX_H
#define PROCESSCOMBOBOX_H

#include <QComboBox>

class ProcessComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit ProcessComboBox(QWidget *parent = nullptr);

protected:
    void showPopup() override;

signals:
    void aboutToShowPopup();
};


#endif // PROCESSCOMBOBOX_H

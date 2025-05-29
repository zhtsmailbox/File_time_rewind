#ifndef DONATEDIALOG_H
#define DONATEDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>

class DonateDialog : public QDialog{
    Q_OBJECT
public:
    explicit DonateDialog(QWidget *parent = nullptr);
};

#endif // DONATEDIALOG_H

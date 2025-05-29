#include "donatedialog.h"

DonateDialog::DonateDialog(QWidget *parent)
    : QDialog(parent) {
    setWindowTitle(tr("支持作者"));
    setFixedSize(300, 400);

    QVBoxLayout *layout = new QVBoxLayout(this);

    QLabel *qrLabel = new QLabel(this);
    QPixmap qrPixmap(":/images/donate_qr.png");  // 资源文件或绝对路径
    qrLabel->setPixmap(qrPixmap.scaled(200, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    qrLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(qrLabel);

    QLabel *infoLabel = new QLabel(tr("感谢您使用我的小工具！/n您的支持和鼓励，是我不断改进和创作的动力。/n如果您觉得这个工具对您有帮助，欢迎通过打赏支持我。/n您的每一份心意，我都将铭记在心，继续努力做得更好。/n衷心感谢，愿您一切顺利！"), this);
    infoLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(infoLabel);

    QPushButton *homepageButton = new QPushButton(tr("访问我的主页"), this);
    layout->addWidget(homepageButton);
    connect(homepageButton, &QPushButton::clicked, []() {
        QDesktopServices::openUrl(QUrl("https://cpeak.top"));
    });

    QPushButton *closeButton = new QPushButton(tr("关闭"), this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(closeButton);
}

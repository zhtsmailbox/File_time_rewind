#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QCheckBox>
#include <QComboBox>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QDateTime>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QMainWindow>
#include <QMessageBox>
#include <QTranslator>
#include <QTimer>
#include <QThread>
#include <QTextEdit>
#include <QLabel>
#include <QLocale>
#include <QLineEdit>
#include <QString>
#include <QSpinBox>
#include <QScreen>
#include <QSettings>
#include <QPushButton>
#include <QPixmap>

#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_6_clicked();
    void on_pushButton_8_clicked();
    void on_spinBox_valueChanged(int arg1);
    void on_comboBox_2_currentIndexChanged(int index);
    void on_pushButton_5_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_4_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();
    void on_pushButton_9_clicked();
    void changeLanguage(const QString &language);
private:
    Ui::MainWindow *ui;
    QString systemLanguage;
    QLabel* statusIcon;
    QLabel* statusText;
    QTimer *m_timer;
    QTranslator translator;
    bool debugOutput = false;
    bool m_timerRunning = false;
    bool m_captureScreenshot = true;
    bool m_isSaving = false;
    int m_backupIntervalValue = 10;        // 默认10
    QString m_backupIntervalUnit = "分钟"; // 默认“分钟”
    QString saveFilePath;     // 当前选中的存档文件路径
    QString backupDirPath;    // 当前选中的备份目录路径
    struct BackupEntry {
        QString saveFileName;
        QString screenshotFileName;
    };
    struct ProcessHandle {
        QString name;
        DWORD pid;
        bool valid = false;
    };
    void saveConfig();
    void loadConfig();
    ProcessHandle findProcessByName(const QString &targetName);
    QVector<BackupEntry> recentBackups = QVector<BackupEntry>(5);
    QStringList getVisibleProcessNames();
    void refreshProcessList();
    QPixmap hbitmapToPixmap(HBITMAP hBitmap);
    void setStatus(QString text, QColor color);
    void appendLog(const QString &message, const QColor &color = Qt::black);
    void performBackup();
    void toggleTimer();
    QString calculateMD5(const QString &filePath);
    bool captureProcessWindow(const QString &processName, const QString &savePathBase, QString &outSavedPath);
    void updateStatus(bool running);
    void updateRecentBackupButtons();
    bool restoreBackup(int btnIndex);
    void showAboutDialog();
    void showDonateDialog();
};
#endif // MAINWINDOW_H

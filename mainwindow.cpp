#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ProcessComboBox.h"
#include "donatedialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    this->setWindowTitle(tr("File time rewind"));
    ui->setupUi(this);
    /*********************************************************************
     * 菜单栏配置
     *********************************************************************/

    QMenu *fileMenu = menuBar()->addMenu(tr("File"));
    QAction *saveConfigAction = new QAction(tr("Save Config"), this);
    QAction *loadConfigAction = new QAction(tr("Read Config"), this);
    QAction *exitAction = new QAction(tr("Exit"), this);

    fileMenu->addAction(saveConfigAction);
    fileMenu->addAction(loadConfigAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    QMenu *languageMenu = menuBar()->addMenu(tr("Language"));
    QAction *changeIntoEnglish = new QAction("English", this);
    QAction *changeIntoChinese = new QAction("中文", this);

    languageMenu->addAction(changeIntoEnglish);
    languageMenu->addAction(changeIntoChinese);

    QMenu *helpMenu = menuBar()->addMenu(tr("Help"));
    QAction *aboutAction = new QAction(tr("About"), this);
    helpMenu->addAction(aboutAction);

    // 信号槽连接
    connect(saveConfigAction, &QAction::triggered, this, &MainWindow::saveConfig);
    connect(loadConfigAction, &QAction::triggered, this, &MainWindow::loadConfig);
    connect(exitAction, &QAction::triggered, this, &MainWindow::close);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::showAboutDialog);

    connect(changeIntoEnglish, &QAction::triggered, this, [=](){changeLanguage("en");});
    connect(changeIntoChinese, &QAction::triggered, this, [=](){changeLanguage("zh_CN");});

    /*********************************************************************
     * 状态栏初始化
     *********************************************************************/
    // 添加图标 QLabel 和文字 QLabel
    statusIcon = new QLabel(this);
    statusText = new QLabel(this);

    // 设置初始状态（红灯 + 未运行）
    QPixmap redLight(10, 10);
    redLight.fill(Qt::red);
    statusIcon->setPixmap(redLight);
    statusIcon->setFixedSize(10, 10);
    statusText->setText(tr("Stop"));

    // 添加到状态栏
    statusBar()->addWidget(statusIcon);
    statusBar()->addWidget(statusText);

    /*********************************************************************
     * 组件配置
     *********************************************************************/
    //Texteditor Read only
    ui->textEdit->setReadOnly(true);

    // 设置spinbox范围和默认值
    ui->spinBox->setRange(0, 99);
    ui->spinBox->setValue(10);

    // 设置时间单位
    ui->comboBox_2->clear();
    ui->comboBox_2->addItem(tr("Second"));
    ui->comboBox_2->addItem(tr("Minute"));
    ui->comboBox_2->addItem(tr("Hour"));
    ui->comboBox_2->setCurrentIndex(1);  // 默认选择“分”
    ui->label_13->setText(tr("#1 Latest"));

    // comboBox 读取截图进程名称
    ProcessComboBox* pCombo = new ProcessComboBox(this);

    // 把原comboBox替换为自定义的ProcessComboBox，保持位置和大小
    QLayout* layout = ui->comboBox->parentWidget()->layout();
    if (layout) {
        layout->replaceWidget(ui->comboBox, pCombo);
    }

    ui->comboBox->deleteLater();
    ui->comboBox = pCombo;
    m_timer = new QTimer(this);

    connect(m_timer, &QTimer::timeout, this, &MainWindow::performBackup);
    connect(pCombo, &ProcessComboBox::aboutToShowPopup, this, &MainWindow::refreshProcessList);
    connect(ui->checkBox, &QCheckBox::toggled, this, [this](bool checked){
        m_captureScreenshot = checked;
        appendLog(QString(tr("screenshot enabled :%1")).arg(checked ? tr("Yes") : tr("Forbid")));
    });

    /*********************************************************************
     * 程序准备就绪
     *********************************************************************/
    loadConfig();
    updateRecentBackupButtons();

    debugOutput = true;

    systemLanguage = QLocale::system().name();
    if(systemLanguage == "zh_CN")
    {
        QString qmPath = QString(":/translations/filesaver_%1.qm").arg(systemLanguage);
        if (translator.load(qmPath)) {
            qApp->installTranslator(&translator);
            ui->retranslateUi(this);  // 更新 UI 文字
        }
    }else
    {
        QString qmPath = QString(":/translations/filesaver_en.qm");
        if (translator.load(qmPath)) {
            qApp->installTranslator(&translator);
            ui->retranslateUi(this);  // 更新 UI 文字
        }
    }
    appendLog(tr("Welcome to File Time Machine. With just a few simple Settings, you can fully save the content backup you need.\n1.First of all, we need to select the file you want to save, such as the archive file. \n2.Then we need to choose where to save the file. Of course, you can also use the default location, that is, the backup directory under the archive folder. \n3.Next is the archive interval time setting, which you can adjust as needed. \n4.Then comes the screenshot summary you need. It will save the corresponding process screenshots when saving. You just need to select the program you want to save (your program needs to be open and not minimized).\n5.then click Start/Stop to run the program. \nThe file record status will be displayed in the status bar, and the log file will be output here."), Qt::green);

}

MainWindow::~MainWindow()
{
    delete ui;
}

//File browse button
void MainWindow::on_pushButton_6_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Choose the backup file"));
    if (!filePath.isEmpty()) {
        ui->fileLineEdit->setText(filePath);
        saveFilePath = filePath;
        QFileInfo fileInfo(filePath);
        QDir parentDir = fileInfo.dir();
        QString backupPath = parentDir.absoluteFilePath("backup");
        // 如果 backup 目录不存在，则创建
        QDir dir;
        if (!dir.exists(backupPath)) {
            if (!dir.mkpath(backupPath)) {
                appendLog(tr("❌Can't creat backup directory: ") + backupPath, Qt::red);
                return;
            }else{
                appendLog(tr("✅Success creat backup directory: ") + backupPath, Qt::darkGreen);
            }
        }
        else{
            appendLog(tr("📁Backup directory exist"), Qt::gray);
        }
        ui->saveDirLineEdit->setText(backupPath);
        backupDirPath = backupPath;
        updateRecentBackupButtons();
    }
}

//添加日志信息到texteditor中
void MainWindow::appendLog(const QString &message, const QColor &color)
{
    if(debugOutput)
    {
        QTextCharFormat fmt;
        fmt.setForeground(color);
        QTextCursor cursor = ui->textEdit->textCursor();
        cursor.movePosition(QTextCursor::End);
        cursor.insertText(message + '\n', fmt);
        ui->textEdit->setTextCursor(cursor);
    }
}

QStringList MainWindow::getVisibleProcessNames() {
    QStringList processNames;

    // 获取所有顶层窗口句柄
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        if (!IsWindowVisible(hwnd)) return TRUE; // 只处理可见窗口
        DWORD pid;
        GetWindowThreadProcessId(hwnd, &pid);
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProcess) {
            TCHAR exeName[MAX_PATH] = {0};
            if (GetModuleFileNameEx(hProcess, nullptr, exeName, MAX_PATH)) {
                QString name = QString::fromWCharArray(exeName).split("\\").last();
                QStringList* list = reinterpret_cast<QStringList*>(lParam);
                if (!list->contains(name)) list->append(name);
            }
            CloseHandle(hProcess);
        }
        return TRUE;
    }, reinterpret_cast<LPARAM>(&processNames));
    return processNames;
}

void MainWindow::refreshProcessList() {
    ui->comboBox->clear();
    QStringList processList = getVisibleProcessNames();
    processList.sort(Qt::CaseInsensitive);
    ui->comboBox->addItems(processList);
    appendLog(QString(tr("🔍A total of %1 window process was found")).arg(processList.count()), Qt::blue);
}

//save directory browse button
void MainWindow::on_pushButton_8_clicked()
{
    QString dirPath = QFileDialog::getExistingDirectory(this, tr("Choose Backup file directory"));
    if (!dirPath.isEmpty()) {
        ui->saveDirLineEdit->setText(dirPath);
    }
}

void MainWindow::setStatus(QString text, QColor color) {
    QPixmap pix(10, 10);
    pix.fill(color);
    statusIcon->setPixmap(pix);
    statusText->setText(text);
}

void MainWindow::on_spinBox_valueChanged(int val)
{
    m_backupIntervalValue = val;
    appendLog(QString(tr("Backup interval %1 (%2)")).arg(m_backupIntervalValue).arg(m_backupIntervalUnit));
}

void MainWindow::on_comboBox_2_currentIndexChanged(int index)
{
    switch (index) {
    case 0:
        m_backupIntervalUnit = tr("Second");
        break;
    case 1:
        m_backupIntervalUnit = tr("Minute");
        break;
    case 2:
        m_backupIntervalUnit = tr("Hour");
        break;
    default:
        m_backupIntervalUnit = tr("Unknown unit");
        break;
    }

    appendLog(QString(tr("Backup interval changed into %1 (%2)")).arg(m_backupIntervalValue).arg(m_backupIntervalUnit));
}

void MainWindow::performBackup() {
    if (m_isSaving) {
        appendLog(tr("Last backup unfinished skip backup"), Qt::gray);
        return;
    }
    m_isSaving = true;

    QFileInfo srcInfo(saveFilePath);
    if (!srcInfo.exists()) {
        appendLog(tr("Source file doesn't exist，Cancel backup"), Qt::red);
        m_isSaving = false;
        return;
    }

    QDir dir(backupDirPath);
    QString baseName = srcInfo.completeBaseName();
    QString ext = srcInfo.suffix();
    QString timeStr = QDateTime::currentDateTime().toString(tr("yyyyMMdd_HHmmss"));
    QString finalName;
    int count = 1;
    do {
        finalName = QString(tr("%1_%2_%3.%4"))
        .arg(baseName).arg(timeStr).arg(count).arg(ext);
        count++;
    } while (dir.exists(finalName));

    QString destPath = dir.filePath(finalName);
    bool copySuccess = false;

    for (int attempt = 0; attempt < 5; ++attempt) {
        QFile::remove(destPath); // 清除旧的尝试
        if (!QFile::copy(saveFilePath, destPath)) {
            appendLog(tr("File copy failure，It might be a permission issue"), Qt::red);
            break;
        }

        QThread::msleep(100);
        if (this->calculateMD5(saveFilePath) == this->calculateMD5(destPath)) {
            appendLog(tr("Buckup Successed: ") + finalName, Qt::darkGreen);
            copySuccess = true;
            break;
        }
    }

    if (!copySuccess) {
        appendLog(tr("Backup failure：MD5 Check failure！"), Qt::red);
        QFile::remove(destPath);
    }
    else{
        //截屏并保存为同名文件
        QString shotBaseName = QString("%1").arg(baseName);
        QString selectedProcName = ui->comboBox->currentText().trimmed();
        QString screenshotPath;
        if (ui->checkBox->isChecked()) {
            if (captureProcessWindow(selectedProcName, shotBaseName, screenshotPath)) {
                appendLog(tr("scereenshot successed：") + screenshotPath, Qt::darkGreen);
            } else {
                appendLog(tr("screenshot failure"), Qt::red);
            }
        }
        //刷新恢复按钮
        updateRecentBackupButtons();
    }
    m_isSaving = false;
}

QString MainWindow::calculateMD5(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        appendLog(tr("Can't open file for MD5 calculation: ") + filePath, Qt::red);
        return QString();
    }
    QCryptographicHash hash(QCryptographicHash::Md5);
    if (!hash.addData(&file)) {
        appendLog(tr("Read file failure: ") + filePath, Qt::red);
        return QString();
    }
    return QString(hash.result().toHex());
}

bool MainWindow::captureProcessWindow(const QString &processName, const QString &savePathBase, QString &outSavedPath) {
    HWND hWnd = nullptr;
    DWORD targetPid = 0;
    // 枚举窗口找目标进程名
    struct Finder {
        QString name;
        HWND hwnd;
        DWORD pid;
    } finder = { processName, nullptr, 0 };
    auto callback = [](HWND hwnd, LPARAM lParam) -> BOOL {
        DWORD pid;
        GetWindowThreadProcessId(hwnd, &pid);

        if (!IsWindowVisible(hwnd) || IsIconic(hwnd)) return TRUE;

        HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        if (hProc) {
            TCHAR exeName[MAX_PATH] = {0};
            if (GetModuleFileNameEx(hProc, nullptr, exeName, MAX_PATH)) {
                QString procName = QFileInfo(QString::fromWCharArray(exeName)).fileName();
                Finder *f = reinterpret_cast<Finder*>(lParam);
                if (procName.compare(f->name, Qt::CaseInsensitive) == 0) {
                    f->hwnd = hwnd;
                    f->pid = pid;
                    CloseHandle(hProc);
                    return FALSE; // 找到就停止
                }
            }
            CloseHandle(hProc);
        }
        return TRUE;
    };
    EnumWindows(callback, reinterpret_cast<LPARAM>(&finder));
    hWnd = finder.hwnd;
    targetPid = finder.pid;
    if (!hWnd) {
        appendLog(tr("Can't find window process：") + processName, Qt::gray);
        return false;
    }
    // 获取窗口矩形
    RECT rect;
    if (!GetWindowRect(hWnd, &rect)) {
        appendLog(tr("Get window size failure"), Qt::red);
        return false;
    }
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    // 开始截图（使用 BitBlt）
    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    HGDIOBJ old = SelectObject(hdcMem, hBitmap);
    BOOL success = BitBlt(hdcMem, 0, 0, width, height, hdcScreen, rect.left, rect.top, SRCCOPY | CAPTUREBLT);
    QPixmap screenshot;
    if (success) {
        screenshot = hbitmapToPixmap(hBitmap);
    }
    // 清理资源
    SelectObject(hdcMem, old);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);
    if (screenshot.isNull()) {
        appendLog(tr("Screenshot capture failure"), Qt::red);
        return false;
    }
    // 保存文件
    QDir dir(backupDirPath);
    QString timeStr = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    int count = 1;
    QString shotName;
    do {
        shotName = QString("%1_%2_%3.png").arg(savePathBase).arg(timeStr).arg(count++);
    } while (dir.exists(shotName));

    QString fullPath = dir.filePath(shotName);
    if (screenshot.save(fullPath)) {
        outSavedPath = fullPath;
        appendLog(tr("Screenshot save success：") + fullPath, Qt::darkGreen);
        return true;
    } else {
        appendLog(tr("Screenshot save failure"), Qt::red);
        return false;
    }
}

void MainWindow::on_pushButton_5_clicked()
{
    if (ui->fileLineEdit->text().isEmpty())
    {
        appendLog(tr("Choose the File you want to backup first!"), Qt::red);
    }
    else
    {
        if (m_timerRunning) {
            // 停止计时器
            if (m_timer) {
                m_timer->stop();
            }
            m_timerRunning = false;
            ui->pushButton_5->setText(tr("Start countdown"));
            appendLog(tr("Stop countdown"), Qt::gray);

            // 更新状态栏
            updateStatus(false);
        } else {
            // 计算总间隔毫秒数
            int interval = m_backupIntervalValue;
            if (m_backupIntervalUnit == "时") {
                interval *= 60 * 60 * 1000;
            } else if (m_backupIntervalUnit == "分") {
                interval *= 60 * 1000;
            } else { // 秒
                interval *= 1000;
            }
            if (!m_timer) {
                m_timer = new QTimer(this);
                connect(m_timer, &QTimer::timeout, this, &MainWindow::performBackup);
            }
            m_timer->start(interval);
            m_timerRunning = true;
            ui->pushButton_5->setText(tr("Stop"));
            appendLog(QString(tr("Start countdown，Every %1 %2 backup one time"))
                          .arg(m_backupIntervalValue)
                          .arg(m_backupIntervalUnit), Qt::darkGreen);

            // 更新状态栏
            updateStatus(true);
        }
    }
    saveConfig();
}

void MainWindow::updateStatus(bool running)
{
    // 设置图标颜色
    QPixmap light(10, 10);
    light.fill(running ? Qt::green : Qt::red);
    statusIcon->setPixmap(light);
    // 设置文字
    statusText->setText(running ? tr("Running") : tr("Stoped"));
}

void MainWindow::updateRecentBackupButtons() {
    QDir dir(backupDirPath);
    QFileInfoList allFileInfos = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    if(ui->fileLineEdit->text().isEmpty())
    {
        ui->pushButton_7->setText("");
        ui->pushButton_4->setText("");
        ui->pushButton_3->setText("");
        ui->pushButton_2->setText("");
        ui->pushButton->setText("");
    }
    else
    {
        // 筛选非PNG（即存档文件）
        QList<QFileInfo> saveFileInfos;
        for (const QFileInfo& fi : allFileInfos) {
            if (!fi.fileName().endsWith(".png", Qt::CaseInsensitive)) {
                saveFileInfos.append(fi);
            }
        }
        // 按最后修改时间降序排序（最新在前）
        std::sort(saveFileInfos.begin(), saveFileInfos.end(), [](const QFileInfo &a, const QFileInfo &b) {
            return a.lastModified() > b.lastModified();
        });
        // 选出倒数第1,2,3,5,10个（即索引 0,1,2,4,9）
        QVector<int> pickIndexes = {0, 1, 2, 4, 9};
        QVector<BackupEntry> newBackups;
        for (int idx : pickIndexes) {
            if (idx < saveFileInfos.size()) {
                QString saveFile = saveFileInfos[idx].fileName();
                QString base = QFileInfo(saveFile).completeBaseName();
                QString shotFile = base + ".png";
                newBackups.append({saveFile, shotFile});
            } else {
                BackupEntry entry;
                entry.saveFileName = "";
                entry.screenshotFileName = "";
                newBackups.append(entry);
            }
        }
        recentBackups = newBackups;
        // 更新按钮界面（举例，假设按钮叫 ui->btnRestore1 ~ ui->btnRestore5）
        QPushButton* btns[5] = { ui->pushButton_7, ui->pushButton_4, ui->pushButton_3, ui->pushButton_2, ui->pushButton };
        for (int i = 0; i < 5; ++i) {
            QString shotFullPath = QDir(backupDirPath).filePath(recentBackups[i].screenshotFileName);
            if (!recentBackups[i].saveFileName.isEmpty()) {
                btns[i]->setText("");  // 不显示文字
                QString shotFullPath = QDir(backupDirPath).filePath(recentBackups[i].screenshotFileName);
                appendLog(QString(tr("Try to load the screenshot file：%1")).arg(shotFullPath));
                appendLog(QString(tr("File exist：%1")).arg(QFile::exists(shotFullPath) ? tr("yes") : tr("no")));
                appendLog(QString(tr("File size：%1 byte")).arg(QFileInfo(shotFullPath).size()));

                QImage testImage;
                bool loaded = testImage.load(shotFullPath);
                appendLog(QString(tr("QImage load result：%1, Size：%2*%3")).arg(loaded).arg(testImage.width()).arg(testImage.height()));

                QPixmap pixmap;
                loaded = pixmap.load(shotFullPath);
                appendLog(QString(tr("QPixmap load result：%1, Size：%2*%3")).arg(loaded).arg(pixmap.width()).arg(pixmap.height()));
                if (pixmap.isNull()) {
                    appendLog(QString(tr("Load screenshot failure: %1")).arg(shotFullPath), Qt::red);
                    btns[i]->setIcon(QIcon());  // 清空图标
                    btns[i]->setText(tr("No Image"));
                } else {
                    QPixmap scaledPixmap = pixmap.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                    btns[i]->setIcon(QIcon(scaledPixmap));
                    btns[i]->setIconSize(QSize(150, 150));
                    btns[i]->setText("");
                    btns[i]->setEnabled(true);
                }
            } else {
                btns[i]->setText(tr("No record"));
                btns[i]->setIcon(QIcon());
                btns[i]->setEnabled(false);
            }
        }
    }

}

void MainWindow::on_pushButton_7_clicked()
{
    restoreBackup(0);
}

void MainWindow::on_pushButton_4_clicked()
{
    restoreBackup(1);
}

void MainWindow::on_pushButton_3_clicked()
{
    restoreBackup(2);
}

void MainWindow::on_pushButton_2_clicked()
{
    restoreBackup(3);
}

void MainWindow::on_pushButton_clicked()
{
    restoreBackup(4);
}

bool MainWindow::restoreBackup(int btnIndex) {
    if (btnIndex < 0 || btnIndex >= recentBackups.size()) {
        appendLog(tr("Invalid Recovery Button Index"), Qt::red);
        return false;
    }
    QString backupFileName = recentBackups[btnIndex].saveFileName;
    if (backupFileName.isEmpty()) {
        appendLog(tr("No corresponding backup archive file"), Qt::red);
        return false;
    }
    QString backupFilePath = QDir(backupDirPath).filePath(backupFileName);
    QFileInfo origFileInfo(saveFilePath);
    if (!origFileInfo.exists()) {
        appendLog(tr("The current archive file does not exist,the original file cannot be backed up"), Qt::red);
        return false;
    }
    // 备份当前存档文件，加后缀 .Backup
    QString backupOrigFilePath = saveFilePath + ".Backup";
    // 如果备份文件已存在，可以先删除或覆盖
    if (QFile::exists(backupOrigFilePath)) {
        if (!QFile::remove(backupOrigFilePath)) {
            appendLog(tr("Unable to delete old backup files:") + backupOrigFilePath, Qt::red);
            return false;
        }
    }
    if (!QFile::copy(saveFilePath, backupOrigFilePath)) {
        appendLog(tr("Backup of current archive file failed:") + backupOrigFilePath, Qt::red);
        return false;
    }
    // 使用备份文件覆盖原存档文件
    if (!QFile::remove(saveFilePath)) {
        appendLog(tr("Failed to delete the original archive file:") + saveFilePath, Qt::red);
        return false;
    }
    if (!QFile::copy(backupFilePath, saveFilePath)) {
        appendLog(tr("Failed to restore archive file:") + backupFilePath, Qt::red);
        return false;
    }
    appendLog(tr("Successfully recovered archive file:") + backupFileName, Qt::darkGreen);
    return true;
}

QPixmap MainWindow::hbitmapToPixmap(HBITMAP hBitmap)
{
    BITMAP bm;
    GetObject(hBitmap, sizeof(bm), &bm);
    BITMAPINFOHEADER bi;
    ZeroMemory(&bi, sizeof(bi));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bm.bmWidth;
    bi.biHeight = -bm.bmHeight; // 负数表示图像是自上而下的
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    int imageSize = bm.bmWidth * bm.bmHeight * 4;
    std::vector<uchar> bits(imageSize);
    HDC hdc = GetDC(nullptr);
    GetDIBits(hdc, hBitmap, 0, bm.bmHeight, bits.data(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    ReleaseDC(nullptr, hdc);
    QImage image(bits.data(), bm.bmWidth, bm.bmHeight, QImage::Format_ARGB32);
    return QPixmap::fromImage(image.copy()); // copy to detach from bits.data
}

void MainWindow::showDonateDialog() {
    DonateDialog *dialog = new DonateDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void MainWindow::on_pushButton_9_clicked()
{
    showDonateDialog();
}

void MainWindow::saveConfig()
{
    QSettings settings("Cpeak", "File_time_rewind");
    settings.setValue("Config/SaveFilePath", saveFilePath);
    settings.setValue("Config/BackupDirectory", backupDirPath);
    settings.setValue("Config/BackupInterval", ui->spinBox->value());
    settings.setValue("Config/IntervalUnit", ui->comboBox_2->currentData());
    settings.setValue("Config/EnableScreenshot", ui->checkBox->isChecked());
    settings.setValue("Config/ScreenshotProcessName", ui->comboBox->currentText().trimmed());
    appendLog(tr("Config Saved！"), Qt::darkGreen);
}

void MainWindow::loadConfig()
{
    QSettings settings("Cpeak", "File_time_rewind");
    // 读取路径
    saveFilePath = settings.value("Config/SaveFilePath", "").toString();
    backupDirPath = settings.value("Config/BackupDirectory", "").toString();
    ui->spinBox->setValue(settings.value("Config/BackupInterval", "").toInt());
    ui->comboBox_2->setCurrentText(settings.value("Config/IntervalUnit", "").toString());
    // 设置到界面（如果你有路径输入框，可以更新它们）
    ui->fileLineEdit->setText(saveFilePath);  // 假设你有这个控件
    ui->saveDirLineEdit->setText(backupDirPath);
    //保存间隔
    // 是否截图
    bool enableScreenshot = settings.value("Config/EnableScreenshot", false).toBool();
    if(enableScreenshot)
        ui->checkBox->setChecked(enableScreenshot);
    // 截图进程名
    QString procName = settings.value("Config/ScreenshotProcessName", "").toString();
    ProcessHandle ProcessHwnd = findProcessByName(procName);
    if(ProcessHwnd.valid)
    {
        ui->comboBox->setCurrentText(ProcessHwnd.name);
    }
}

MainWindow::ProcessHandle MainWindow::findProcessByName(const QString &targetName) {
    ProcessHandle result;

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap == INVALID_HANDLE_VALUE)
        return result;

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnap, &pe32)) {
        do {
            QString procName = QString::fromWCharArray(pe32.szExeFile);
            if (procName.compare(targetName, Qt::CaseInsensitive) == 0) {
                result.name = procName;
                result.pid = pe32.th32ProcessID;
                result.valid = true;
                break;  // 找到就停止
            }
        } while (Process32Next(hSnap, &pe32));
    }

    CloseHandle(hSnap);
    return result;
}
void MainWindow::showAboutDialog() {
    QMessageBox::about(this, "About", "FileSaver v1.0\nAuth：Cpeak\nfunction：File time rewind");
}

void MainWindow::changeLanguage(const QString &langCode) {
    qApp->removeTranslator(&translator);  // 移除之前的翻译器（如果有）
    QString qmPath = QString(":/translations/filesaver_%1.qm").arg(langCode);
    if (translator.load(qmPath)) {
        qApp->installTranslator(&translator);
        ui->retranslateUi(this);  // 更新 UI 文字
    }
}

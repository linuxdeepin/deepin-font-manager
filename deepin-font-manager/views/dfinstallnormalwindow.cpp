#include "dfinstallnormalwindow.h"
#include "dfontmanager.h"
#include "globaldef.h"
#include "utils.h"
#include "dfmdbmanager.h"
#include "dfontpreviewlistdatathread.h"

#include <QResizeEvent>
#include <QVBoxLayout>

#include <DApplication>
#include <DApplicationHelper>
#include <DLog>

DWIDGET_USE_NAMESPACE

DFInstallNormalWindow::DFInstallNormalWindow(const QStringList &files, QWidget *parent)
    : DFontBaseDialog(parent)
    , m_installFiles(files)
    , m_fontInfoManager(DFontInfoManager::instance())
    , m_fontManager(DFontManager::instance())
    , m_verifyTimer(new QTimer(this))
{
//    setWindowOpacity(0.5);
    qDebug() << __FUNCTION__ << "install files " << files;
    initUI();
    initConnections();
    GetAllSysfiles();
}

DFInstallNormalWindow::~DFInstallNormalWindow()
{
    if (nullptr != m_pexceptionDlg) {
        delete m_pexceptionDlg;
    }
}

void DFInstallNormalWindow::initUI()
{
    setFixedSize(QSize(380, 136));
    setTitle(DApplication::translate("NormalInstallWindow", "Install Font"));

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(10, 0, 0, 0);

    QVBoxLayout *contentLayout = new QVBoxLayout();
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(10, 0, 0, 0);

    m_progressStepLabel = new DLabel(this);
    QFont pslFont = m_progressStepLabel->font();
    pslFont.setWeight(QFont::Medium);
    //pslFont.setPixelSize(14);
    m_progressStepLabel->setFont(pslFont);
    DFontSizeManager::instance()->bind(m_progressStepLabel, DFontSizeManager::T6);
    m_progressStepLabel->setFixedHeight(m_progressStepLabel->fontMetrics().height());
    m_progressStepLabel->setText(DApplication::translate("NormalInstallWindow", "Verifying..."));

    m_currentFontLabel = new DLabel(this);
//    QFont cflFont;
//    cflFont.setPixelSize(12);
//    m_currentFontLabel->setFont(cflFont);
    DFontSizeManager::instance()->bind(m_currentFontLabel, DFontSizeManager::T8);
    m_currentFontLabel->setFixedHeight(m_currentFontLabel->fontMetrics().height());
    m_currentFontLabel->setText("");
    DPalette pa = DApplicationHelper::instance()->palette(m_currentFontLabel);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::TextTips));
    m_currentFontLabel->setPalette(pa);

    m_progressBar = new DProgressBar(this);
    m_progressBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_progressBar->setFixedSize(380 - 42, 6);

    contentLayout->addSpacing(10);
    contentLayout->addWidget(m_progressStepLabel);
    contentLayout->addSpacing(7);
    contentLayout->addWidget(m_currentFontLabel);
    contentLayout->addSpacing(10);
    contentLayout->addWidget(m_progressBar);
    contentLayout->addSpacing(20);

    mainLayout->addLayout(contentLayout);

    m_mainFrame = new QWidget(this);
    m_mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_mainFrame->setLayout(mainLayout);

    addContent(m_mainFrame);
#ifdef FTM_DEBUG_LAYOUT_COLOR
    m_titleFrame->setStyleSheet("background: red");
    m_mainFrame->setStyleSheet("background: green");
    m_progressStepLabel->setStyleSheet("background: silver");
    m_currentFontLabel->setStyleSheet("background: silver");
    m_progressBar->setStyleSheet("background: silver");
#endif
}

void DFInstallNormalWindow::initVerifyTimer()
{
    m_verifyTimer->setSingleShot(true);
    m_verifyTimer->setTimerType(Qt::PreciseTimer);
    m_verifyTimer->start(VERIFY_DELYAY_TIME);
}

void DFInstallNormalWindow::initConnections()
{
    connect(m_verifyTimer.get(), &QTimer::timeout, this, [ = ]() {
        // Install the font list ,which may be changed in exception window
        batchInstall();
    });

    // Handle reinstall signal
    connect(this, &DFInstallNormalWindow::batchReinstall, this, [ = ](QStringList reinstallFiles, int m_totalCount) {
        // Reinstall the user selected files
        m_installFiles.clear();
        m_installState = InstallState::reinstall;

        foreach (auto it, reinstallFiles) {
            m_installFiles.append(it);
        }
        systemFontCount = m_systemFiles.size();
#ifdef QT_QML_DEBUG
        qDebug() << __FUNCTION__ << " [reinstallFiles=" << m_installFiles << "]";
#endif
        batchInstall();
    });

    connect(m_fontManager, &DFontManager::batchInstall, this,
            &DFInstallNormalWindow::onProgressChanged);

    connect(m_fontManager, &DFontManager::installFinished, this, [ = ](int state, QStringList fileList, int systemCount) {
        // ToDo:
        //   May send signal to mainwindow refresh new installed font
        // QMIT notfiyRefresh;
        qDebug() << __FUNCTION__ << " installed file list " << fileList << state;
        if (0 == state) {
            m_installFiles.clear();
            m_installState = InstallState::reinstall;

            // Update the installtion file list showed in exception dialog
            foreach (auto it, m_installedFiles) {
                m_installFiles.append(it);
            }

            foreach (auto it, m_damagedFiles) {
                m_installFiles.append(it);
            }

            //TODO:
            //   Notify UI refresh after installtion.
            // (need to refresh everytime???)

            for (QString file : fileList) {
                int index = file.indexOf("|");
                if (index >= 0) {
                    file = file.left(index);
                }

                if (!m_outfileList.contains(file))
                    m_outfileList << file;
            }

            qDebug() << __FUNCTION__ << "finishFontInstall outlist " << m_outfileList;
            emit finishFontInstall(m_outfileList);

            if (ifNeedShowExceptionWindow()) {
                showInstallErrDlg();
            } else {
                qDebug() << "quit install process!" << endl;
                this->close();
            }
        } else {
            // User cancel in athorisze window
            this->close();
        }

        m_fontManager->emit showFloatingMessage(fileList.size(), systemCount);

    });

    initVerifyTimer();
}

void DFInstallNormalWindow::GetAllSysfiles()
{
    DFMDBManager *dbManager = DFMDBManager::instance();
    QString systemFile;
    QList<DFontPreviewItemData> allFontInfo = dbManager->getAllFontInfo();
    for (auto font : allFontInfo) {
        if (font.fontInfo.filePath.contains("/usr/share/")) {
            systemFile.clear();
            systemFile.append(font.fontInfo.familyName).append(" ").append(font.fontInfo.styleName);
            m_AllSysFiles.append(systemFile);
        }
    }
}

void DFInstallNormalWindow::verifyFontFiles()
{
    // debug
    DFontInfo fontInfo;
    QList<DFontInfo> fontInfos;
    QList<DFontInfo> instFontInfos;

    m_damagedFiles.clear();
    m_installedFiles.clear();
    m_newInstallFiles.clear();
    m_systemFiles.clear();

    m_installErrorFontModelList.clear();
    foreach (auto it, m_installFiles) {
        fontInfo = m_fontInfoManager->getFontInfo(it);
        if (fontInfo.isError) {
            m_damagedFiles.append(it);

#ifdef QT_QML_DEBUG
            qDebug() << __FUNCTION__ << " (" << it << " :Damaged file)";
#endif
        } else if (fontInfo.isInstalled && !isSystemFont(fontInfo)) {
            if (!instFontInfos.contains(fontInfo)) {
                instFontInfos.append(fontInfo);
                m_installedFiles.append(it);
            }

#ifdef QT_QML_DEBUG
            qDebug() << __FUNCTION__ << " (" << it << " :Installed file)";
#endif
        } else if (isSystemFont(fontInfo)) {
            m_systemFiles.append(it);

#ifdef QT_QML_DEBUG
            qDebug() << __FUNCTION__ << " (" << it << " :Installed file)";
#endif
        } else if (!fontInfos.contains(fontInfo)) {
            fontInfos.append(fontInfo);
            m_newInstallFiles.append(it);

#ifdef QT_QML_DEBUG
            qDebug() << __FUNCTION__ << " (" << it << " :New file)";
#endif
        }
    }
}

bool DFInstallNormalWindow::ifNeedShowExceptionWindow() const
{
    // Skip Exception dialog
    if (m_isNeedSkipException) {
        return false;
    }
    // If have new install file,install first,then check the exception list
    if (InstallState::Install == m_installState && m_newInstallFiles.size() > 0) {
        return false;
    }

    // For all selected files is installed & damage
    if (InstallState::Install == m_installState
            && (m_installedFiles.size() > 0 || m_damagedFiles.size() > 0)) {
        return true;
    }

    if (InstallState::Install == m_installState && m_systemFiles.size() != 0) {
        return true;
    }

    if (InstallState::reinstall == m_installState && m_installedFiles.size() > 0) {
        return true;
    }

    if (InstallState::reinstall == m_installState && m_damagedFiles.size() > 0) {
        return true;
    }

    return false;
}

bool DFInstallNormalWindow::isSystemFont(DFontInfo &f)
{
    QString fontFullName = f.fullname + " " + f.styleName;
    foreach (auto it, m_AllSysFiles) {
        if (!it.compare(fontFullName)) {
            return true;
        }
    }
    return false;
}

void DFInstallNormalWindow::resizeEvent(QResizeEvent *event)
{
    DFontBaseDialog::resizeEvent(event);
}

void DFInstallNormalWindow::closeEvent(QCloseEvent *event)
{
    static bool flag = true;
    if (flag) {
        event->accept();
        // TODO: close dfontmanager thread and emit signal to update font show.
        if (m_fontManager) {
            m_fontManager->requestInterruption();
            m_fontManager->quit();
            m_fontManager->wait();
        }
    } else {
        event->ignore();
    }
}

void DFInstallNormalWindow::batchInstall()
{
    // Check&Sort uninstalled ,installed & damaged font file here
    verifyFontFiles();

    if (InstallState::Install == m_installState && ifNeedShowExceptionWindow()) {
#ifdef QT_QML_DEBUG
        qDebug() << "User selected files are installed & damaged!";
#endif

        showInstallErrDlg();
        return;
    }
    systemFontCount = m_systemFiles.size();

    QStringList installList;

    if (m_newInstallFiles.size() > 0) {
        foreach (auto it, m_newInstallFiles) {
            installList.append(it);
        }
    }

    m_newInstallFiles.clear();

    if (m_installState == InstallState::reinstall) {
        if (m_installedFiles.size() > 0) {
            QStringList filesInstalled;
            foreach (auto it, m_installedFiles) {
                installList.append(it);
                //delete the font file first
                DFontInfo fi = m_fontInfoManager->getFontInfo(it);
                QString filePath = DFMDBManager::instance()->isFontInfoExist(fi);
                if (QFileInfo(filePath).fileName() == QFileInfo(it).fileName()) {
                    qDebug() << __FUNCTION__ << "same file " << it << " will be overrided ";
                    continue;
                }
                filesInstalled << filePath;
            }
            //force delete fonts installed
            DFontPreviewListDataThread *dataThread = DFontPreviewListDataThread::instance();
            if (!filesInstalled.empty()) {
                dataThread->forceDeleteFiles(filesInstalled);
                qDebug() << __FUNCTION__ << " remove found installed font : " << filesInstalled;
            }
        }

        m_installedFiles.clear();
    }

    //ToDo:
    //    A temp resolution for installtion.
    //dfont-install don't need query database anymore
    QStringList installListWithFamliyName;
    foreach (auto it, installList) {
        DFontInfo fontInfo = m_fontInfoManager->getFontInfo(it);
        QString familyName = fontInfo.familyName;
        installListWithFamliyName.append(it + "|" + familyName);

        qDebug() << " Prepare install file: " << it + "|" + familyName;
    }

    m_fontManager->setType(DFontManager::Install);
    m_fontManager->setInstallFileList(installListWithFamliyName);
    m_fontManager->setSystemFontCount(systemFontCount);
    this->systemFontCount = 0;
    m_fontManager->start();
}


void DFInstallNormalWindow::batchReInstall()
{

}

void DFInstallNormalWindow::onCancelInstall()
{
#ifdef QT_QML_DEBUG
    qDebug() << __FUNCTION__ << " called";
#endif

    m_fontManager->terminate();
    m_fontManager->wait();

    this->accept();
}

void DFInstallNormalWindow::onContinueInstall(QStringList continueInstallFontFileList, int m_totalCount)
{
#ifdef QT_QML_DEBUG
    qDebug() << __FUNCTION__ << " called:" << continueInstallFontFileList;
#endif

    m_installState = InstallState::reinstall;

    Q_EMIT batchReinstall(continueInstallFontFileList, m_totalCount);
}

void DFInstallNormalWindow::onProgressChanged(const QString &filePath, const double &percent)
{
    if (filePath.isEmpty()) {
        return;
    }

    DFontInfo fontInfo = m_fontInfoManager->getFontInfo(filePath);
    m_currentFontLabel->setText(fontInfo.familyName);
    m_progressBar->setValue(static_cast<int>(percent));
    m_progressBar->setTextVisible(false);

    qDebug() << QString("font install progress: %1%").arg(percent);
}

void DFInstallNormalWindow::showInstallErrDlg()
{
    m_pexceptionDlg = new DFInstallErrorDialog(this, m_installFiles, m_AllSysFiles);

    connect(m_pexceptionDlg, &DFInstallErrorDialog::onCancelInstall, this,
            &DFInstallNormalWindow::onCancelInstall);
    connect(m_pexceptionDlg, &DFInstallErrorDialog::onContinueInstall, this,
            &DFInstallNormalWindow::onContinueInstall);

    m_pexceptionDlg->exec();
}

void DFInstallNormalWindow::setSkipException(bool skip)
{
    m_isNeedSkipException = skip;
}

void DFInstallNormalWindow::breakInstalltion()
{
    //Todo:
    //   Just close the installtion window
    if (m_pexceptionDlg->isVisible()) {
        m_pexceptionDlg->closed();
        m_pexceptionDlg->deleteLater();
    }

    this->closed();
}

#include "dfinstallnormalwindow.h"
#include "dfontmanager.h"
#include "globaldef.h"
#include "utils.h"

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

    initUI();
    initConnections();
}

DFInstallNormalWindow::~DFInstallNormalWindow() {
    if (nullptr != m_pexceptionDlg) {
        delete m_pexceptionDlg;
    }
}

void DFInstallNormalWindow::initUI()
{
    setFixedSize(QSize(380, 136));
    setTitle(DApplication::translate("NormalInstallWindow", "Font Installation"));

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(10, 0, 0, 0);

    QString fontFamily = Utils::loadFontFamilyFromFiles(":/images/SourceHanSansCN-Medium.ttf");

    QVBoxLayout *contentLayout = new QVBoxLayout();
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(10, 0, 0, 0);

    m_progressStepLabel = new DLabel(this);
    QFont pslFont(fontFamily);
    pslFont.setWeight(QFont::Medium);
    //pslFont.setPixelSize(14);
    m_progressStepLabel->setFont(pslFont);
    DFontSizeManager::instance()->bind(m_progressStepLabel, DFontSizeManager::T6);
    m_progressStepLabel->setFixedHeight(20);
    m_progressStepLabel->setText(DApplication::translate("NormalInstallWindow", "Verifying..."));

    m_currentFontLabel = new DLabel(this);
//    QFont cflFont;
//    cflFont.setPixelSize(12);
//    m_currentFontLabel->setFont(cflFont);
    DFontSizeManager::instance()->bind(m_currentFontLabel, DFontSizeManager::T8);
    m_currentFontLabel->setFixedHeight(18);
    m_currentFontLabel->setText("");
    DPalette pa = DApplicationHelper::instance()->palette(m_currentFontLabel);
    pa.setBrush(DPalette::WindowText, pa.color(DPalette::TextTips));
    m_currentFontLabel->setPalette(pa);

    m_progressBar = new DProgressBar(this);
    m_progressBar->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_progressBar->setFixedSize(380-42, 6);

    contentLayout->addSpacing(10);
    contentLayout->addWidget(m_progressStepLabel);
    contentLayout->addSpacing(7);
    contentLayout->addWidget(m_currentFontLabel);
    contentLayout->addSpacing(7);
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
    connect(m_verifyTimer.get(), &QTimer::timeout, this, [=]() {
        // Install the font list ,which may be changed in exception window
        batchInstall();
    });

    // Handle reinstall signal
    connect(this, &DFInstallNormalWindow::batchReinstall, this, [=](QStringList reinstallFiles) {
        // Reinstall the user selected files
        m_installFiles.clear();
        m_installState = InstallState::reinstall;

        foreach (auto it, reinstallFiles) {
            m_installFiles.append(it);
        }

#ifdef QT_QML_DEBUG
        qDebug() << __FUNCTION__ << " [reinstallFiles=" << m_installFiles << "]";
#endif
        batchInstall();
    });

    connect(m_fontManager, &DFontManager::batchInstall, this,
            &DFInstallNormalWindow::onProgressChanged);

    connect(m_fontManager, &DFontManager::batchInstall, this,
            &DFInstallNormalWindow::onProgressChanged);

    connect(m_fontManager, &DFontManager::installFinished, this, [=](int state) {
        // ToDo:
        //   May send signal to mainwindow refresh new installed font
        // QMIT notfiyRefresh;

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
            emit finishFontInstall();

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
    });

    initVerifyTimer();
}

void DFInstallNormalWindow::verifyFontFiles()
{
    // debug
    DFontInfo fontInfo;

    m_damagedFiles.clear();
    m_installedFiles.clear();
    m_newInstallFiles.clear();

    m_installErrorFontModelList.clear();
    foreach (auto it, m_installFiles) {
        fontInfo = m_fontInfoManager->getFontInfo(it);
        if (fontInfo.isError) {
            m_damagedFiles.append(it);

#ifdef QT_QML_DEBUG
            qDebug() << __FUNCTION__ << " (" << it << " :Damaged file)";
#endif
        } else if (fontInfo.isInstalled) {
            m_installedFiles.append(it);

#ifdef QT_QML_DEBUG
            qDebug() << __FUNCTION__ << " (" << it << " :Installed file)";
#endif
        } else {
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

    if (InstallState::reinstall == m_installState && m_installedFiles.size() > 0) {
        return true;
    }

    if (InstallState::reinstall == m_installState && m_damagedFiles.size() > 0) {
        return true;
    }

    return false;
}

void DFInstallNormalWindow::resizeEvent(QResizeEvent *event)
{
    DFontBaseDialog::resizeEvent(event);

    //m_mainFrame->resize(event->size().width(), event->size().height());
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

    QStringList installList;

    if (m_installState == InstallState::Install) {
        if (m_newInstallFiles.size() > 0) {
            foreach (auto it, m_newInstallFiles) {
                installList.append(it);
            }
        }

        m_newInstallFiles.clear();
    }

    if (m_installState == InstallState::reinstall) {
        if (m_installedFiles.size() > 0) {
            foreach (auto it, m_installedFiles) {
                installList.append(it);
            }
        }

        m_installedFiles.clear();
    }

    //ToDo:
    //    A temp resolution for installtion.
    //dfont-install don't need query database anymore
    QStringList installListWithFamliyName;
    foreach(auto it, installList) {
        DFontInfo fontInfo = m_fontInfoManager->getFontInfo(it);
        QString familyName = fontInfo.familyName;

         installListWithFamliyName.append(it+"|"+familyName);

         qDebug() << " Prepare install file: " << it+"|"+familyName;
    }

    m_fontManager->setType(DFontManager::Install);
    m_fontManager->setInstallFileList(installListWithFamliyName);
    m_fontManager->start();
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

void DFInstallNormalWindow::onContinueInstall(QStringList continueInstallFontFileList)
{
#ifdef QT_QML_DEBUG
    qDebug() << __FUNCTION__ << " called:" << continueInstallFontFileList;
#endif

    m_installState = InstallState::reinstall;

    Q_EMIT batchReinstall(continueInstallFontFileList);
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
}

void DFInstallNormalWindow::showInstallErrDlg()
{
    m_pexceptionDlg = new DFInstallErrorDialog(this, m_installFiles);

    connect(m_pexceptionDlg, &DFInstallErrorDialog::onCancelInstall, this,
            &DFInstallNormalWindow::onCancelInstall);
    connect(m_pexceptionDlg, &DFInstallErrorDialog::onContinueInstall, this,
            &DFInstallNormalWindow::onContinueInstall);
//    connect(m_pexceptionDlg, &DFInstallErrorDialog::closed, this,
//            &DFInstallNormalWindow::onCancelInstall);

    m_pexceptionDlg->exec();
}

void DFInstallNormalWindow::setSkipException(bool skip)
{
    m_isNeedSkipException = skip;
}

void DFInstallNormalWindow::breakInstalltion() {
    //Todo:
    //   Just close the installtion window
    if (m_pexceptionDlg->isVisible()) {
        m_pexceptionDlg->closed();
        m_pexceptionDlg->deleteLater();
    }

    this->closed();
}

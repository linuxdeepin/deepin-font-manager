#include "dfinstallnormalwindow.h"
#include "dfontmanager.h"
#include "globaldef.h"
#include "utils.h"
#include "dfmdbmanager.h"
#include "dfontpreviewlistdatathread.h"
#include "dfinstallerrordialog.h"
#include <QResizeEvent>
#include <QVBoxLayout>

#include <DApplication>
#include <DApplicationHelper>
#include <DLog>

DWIDGET_USE_NAMESPACE

const QString ONLYPROGRESS = "onlyprogress";

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
    getAllSysfiles();
    verifyFontFiles();
    initConnections();
}

DFInstallNormalWindow::~DFInstallNormalWindow()
{
    m_installFiles.clear();
    m_installedFiles.clear();
    m_newInstallFiles.clear();
    m_damagedFiles.clear();
    m_systemFiles.clear();
    m_outfileList.clear();
    m_errorList.clear();
    m_AllSysFiles.clear();
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
    connect(this, &DFInstallNormalWindow::batchReinstall, this, [ = ](QStringList reinstallFiles) {
        // Reinstall the user selected files
        m_installFiles.clear();
        m_installState = InstallState::reinstall;

        foreach (auto it, reinstallFiles) {
            m_installFiles.append(it);
        }
#ifdef QT_QML_DEBUG
//        qDebug() << __FUNCTION__ << " [reinstallFiles=" << m_installFiles << "]";
#endif
        batchReInstall();
    });

    connect(m_fontManager, &DFontManager::batchInstall, this,
            &DFInstallNormalWindow::onProgressChanged);

    connect(m_fontManager, &DFontManager::installFinished, this, [ = ](int state, QStringList fileList) {
        // ToDo:
        //   May send signal to mainwindow refresh new installed font
        // QMIT notfiyRefresh;

        if (0 == state) {
            m_installFiles.clear();
            m_installState = InstallState::Install;

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

//            emit finishFontInstall(m_outfileList);
        }

//        qDebug() << __FUNCTION__ << " installed file list ++++  " << fileList << state;
        emit  m_signalManager->sendInstallMessage(fileList.size());

    });

    connect(m_fontManager, &DFontManager::reInstallFinished, this, [ = ](int state, QStringList fileList) {
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

            for (QString file : fileList) {
                int index = file.indexOf("|");
                if (index >= 0) {
                    file = file.left(index);
                }

                if (!m_outfileList.contains(file))
                    m_outfileList << file;
            }
//            emit finishFontInstall(m_outfileList);

//            if (ifNeedShowExceptionWindow()) {
////                showInstallErrDlg();
//            } else {
//                qDebug() << "quit install process!" << endl;
//                this->close();
//            }
        } /*else {
            // User cancel in athorisze window
            this->close();
        }*/
//        qDebug() << __FUNCTION__ << " Reinstalled file list ++++ " << fileList << state;

        emit  m_signalManager->sendReInstallMessage(fileList.size());

    });

    connect(m_signalManager, &SignalManager::sendInstallMessage, this, [ = ](int totalCount) {
        getInstallMessage = true;
        totalInstallFont = totalInstallFont + totalCount;
        checkShowMessage();
    }, Qt::QueuedConnection);

    connect(m_signalManager, &SignalManager::sendReInstallMessage, this, [ = ](int totalCount) {
        getReInstallMessage = true;
        totalInstallFont = totalInstallFont + totalCount;
        checkShowMessage();
    }, Qt::UniqueConnection);

//    connect(m_signalManager, &SignalManager::requestInstallAdded, this, [ = ]() {
//        m_installAdded = true;
//        checkShowMessage();
//    }, Qt::UniqueConnection);

    connect(DFontPreviewListDataThread::instance(), &DFontPreviewListDataThread::requestBatchReInstallContinue, this, &DFInstallNormalWindow::batchReInstallContinue);

    initVerifyTimer();
}


//ut000442 从数据库中读取系统字体，用于之后的判断
void DFInstallNormalWindow::getAllSysfiles()
{
    QList<DFontPreviewItemData> allFontInfo = DFontPreviewListDataThread::instance()->getFontModelList();
    if (allFontInfo.isEmpty())
        allFontInfo = DFMDBManager::instance()->getAllFontInfo();
    for (auto font : allFontInfo) {
        if (font.fontInfo.filePath.contains("/usr/share/")) {
            QString systemFile;
            systemFile.append(font.fontInfo.familyName).append(font.fontInfo.styleName);
            m_AllSysFiles.append(systemFile);
        }
    }
}

//字体文件过滤器，过滤后得到需要新安装的字体，重复安装字体，损毁字体，系统字体
//过滤后进行安装
void DFInstallNormalWindow::verifyFontFiles()
{
    DFontInfo fontInfo;
    QList<DFontInfo> fontInfos;
    QList<DFontInfo> instFontInfos;

    m_damagedFiles.clear();
    m_installedFiles.clear();
    m_newInstallFiles.clear();
    m_systemFiles.clear();
    m_errorList.clear();

    foreach (auto it, m_installFiles) {
        fontInfo = m_fontInfoManager->getFontInfo(it, true);
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
    m_errorList = m_damagedFiles + m_installedFiles + m_systemFiles;
}


//检测是否要弹出字体验证框，存在重复安装字体，系统字体时，损坏字体时弹出字体验证框
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

    if (InstallState::Install == m_installState && m_systemFiles.size() > 0) {
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
    QString fontFullName = f.familyName + f.styleName;
//    foreach (auto it, m_AllSysFiles) {
//        if (!it.compare(fontFullName)) {
//            return true;
//        }
//    }
    if (m_AllSysFiles.contains(fontFullName)) {
        return true;
    } else {
        return false;
    }

}

void DFInstallNormalWindow::checkShowMessage()
{
//新安装的字体在安装完成时 getInstallMessage置为true。重复安装的字体安装完成时 getReInstallMessage置为true
//列表刷新完成后 m_installAdded置为true。三者都为true时表示一次安装过程结束。

    qDebug() << "Install over" << endl;
//    qDebug() << getInstallMessage << getReInstallMessage << m_installAdded << m_installFinishSent;
    if (getInstallMessage == true && getReInstallMessage == true) {
        qDebug() << "ReInstall over" << endl;
//        if (!m_installFinishSent) {
        emit m_signalManager->finishFontInstall(m_outfileList);
//            m_installFinishSent = true;
//        }
    }

    if (getInstallMessage == true && getReInstallMessage == true/* && m_installAdded*/) {
        qDebug() << "install refresh over";
//        onProgressChanged(ONLYPROGRESS, 100);
        getInstallMessage = false;
        getReInstallMessage = false;
//        m_installFinishSent = false;
//        m_installAdded = false;
        emit m_signalManager->showInstallFloatingMessage(totalInstallFont);
        m_outfileList.clear();
        if (totalInstallFont > 0) {
            emit m_signalManager->closeInstallDialog();
        }

        totalInstallFont = 0;

        //ut000442 bug25822 通过log推测此窗口提前关闭导致的问题,因为没有必现步骤,检查逻辑暂时没发现问题,暂时通过添加延迟来进行解决
        QTimer::singleShot(50, this, [ = ]() {
            this->close();
        });
    }

    if (getInstallMessage == true && getReInstallMessage == false) {
        if (ifNeedShowExceptionWindow()) {
            qDebug() << "need reinstall+++++++++++++++++++++++++++++++" << endl;
            showInstallErrDlg();
        } else {
            qDebug() << "no need reinstall+++++++++++++++++++++++++++++++" << endl;
            emit  m_signalManager->sendReInstallMessage(0);
        }
    }
    emit m_signalManager->setIsJustInstalled();
}

void DFInstallNormalWindow::resizeEvent(QResizeEvent *event)
{
    DFontBaseDialog::resizeEvent(event);
}

void DFInstallNormalWindow::paintEvent(QPaintEvent *event)
{
//  ut000442 优化显示效果，弹出对话框时可以动态调整label的高度，从而避免遮挡的出现

    DFontBaseDialog::paintEvent(event);
    m_progressStepLabel->setFixedHeight(m_progressStepLabel->fontMetrics().height());
}

void DFInstallNormalWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << __FUNCTION__;
//ut000442 去除这一段代码,如果这段代码有用可以告知一下

//    getInstallMessage = false;
//    getReInstallMessage = false;
//    static bool flag = true;
//    if (flag) {
//        event->accept();
//        // TODO: close dfontmanager thread and emit signal to update font show.
////        if (m_fontManager) {
////            m_fontManager->requestInterruption();
////            m_fontManager->quit();
////            m_fontManager->wait();
////        }
//    } else {
//        event->accept();
//    }
}
void DFInstallNormalWindow::batchInstall()
{
    // Check&Sort uninstalled ,installed & damaged font file here

    QStringList installList;

    if (m_newInstallFiles.size() > 0) {
        foreach (auto it, m_newInstallFiles) {
            installList.append(it);
        }
    }

    m_newInstallFiles.clear();

    //    if (m_installState == InstallState::reinstall) {
    //        if (m_installedFiles.size() > 0) {
    //            QStringList filesInstalled;
    //            foreach (auto it, m_installedFiles) {
    //                installList.append(it);
    //                //delete the font file first
    //                DFontInfo fi = m_fontInfoManager->getFontInfo(it);
    //                QString filePath = DFMDBManager::instance()->isFontInfoExist(fi);
    //                if (QFileInfo(filePath).fileName() == QFileInfo(it).fileName()) {
    //                    qDebug() << __FUNCTION__ << "same file " << it << " will be overrided ";
    //                    continue;
    //                }
    //                filesInstalled << filePath;
    //            }
    //            //force delete fonts installed
    //            DFontPreviewListDataThread *dataThread = DFontPreviewListDataThread::instance();
    //            if (!filesInstalled.empty()) {
    //                dataThread->forceDeleteFiles(filesInstalled);
    //                qDebug() << __FUNCTION__ << " remove found installed font : " << filesInstalled;
    //            }
    //        }

    //        m_installedFiles.clear();
    //    }

    //ToDo:
    //    A temp resolution for installtion.
    //dfont-install don't need query database anymore
    QStringList installListWithFamliyName;
    foreach (auto it, installList) {
        DFontInfo fontInfo = m_fontInfoManager->getFontInfo(it);
        QString familyName = fontInfo.familyName;
        installListWithFamliyName.append(it + "|" + familyName);

//        qDebug() << " Prepare install file: " << it + "|" + familyName;
    }

    m_fontManager->setType(DFontManager::Install);
    m_fontManager->setInstallFileList(installListWithFamliyName);
    m_fontManager->setSystemFontCount(systemFontCount);
    this->systemFontCount = 0;
    m_fontManager->start();
}


void DFInstallNormalWindow::batchReInstall()
{
    QStringList installList;

    if (m_installState == InstallState::reinstall) {
        if (m_installedFiles.size() > 0) {
            QStringList filesInstalled;
            foreach (auto it, m_installFiles) {
                installList.append(it);
                //delete the font file first
                DFontInfo fi = m_fontInfoManager->getFontInfo(it);
                QString filePath = DFMDBManager::instance()->isFontInfoExist(fi);
                if (QFileInfo(filePath).fileName() == QFileInfo(it).fileName()) {
//                    qDebug() << __FUNCTION__ << "same file " << it << " will be overrided ";
                    continue;
                }
                filesInstalled << filePath;
            }
            //force delete fonts installed
            DFontPreviewListDataThread *dataThread = DFontPreviewListDataThread::instance();
            if (!filesInstalled.empty()) {
                Q_EMIT dataThread->requestForceDeleteFiles(filesInstalled);
                //exit
                return;
//                qDebug() << __FUNCTION__ << " remove found installed font : " << filesInstalled;
            }
        }

        m_installedFiles.clear();
    }

    QStringList installListWithFamliyName;
    foreach (auto it, installList) {
        DFontInfo fontInfo = m_fontInfoManager->getFontInfo(it);
        QString familyName = fontInfo.familyName;
        installListWithFamliyName.append(it + "|" + familyName);

//        qDebug() << " Prepare install file: " << it + m_loadingSpinner"|" + familyName;
    }

    m_fontManager->setType(DFontManager::ReInstall);
    m_fontManager->setInstallFileList(installListWithFamliyName);
    m_fontManager->start();
}

void DFInstallNormalWindow::batchReInstallContinue()
{
    if (m_installState == InstallState::reinstall) {
        m_installedFiles.clear();
    }

    QStringList installListWithFamliyName;
    foreach (auto it, m_installFiles) {
        DFontInfo fontInfo = m_fontInfoManager->getFontInfo(it);
        QString familyName = fontInfo.familyName;
        installListWithFamliyName.append(it + "|" + familyName);

        qDebug() << " Prepare install file: " << it + "|" + familyName;
    }

    m_fontManager->setType(DFontManager::ReInstall);
    m_fontManager->setInstallFileList(installListWithFamliyName);
    m_fontManager->start();
}

void DFInstallNormalWindow::onCancelInstall()
{
#ifdef QT_QML_DEBUG
    qDebug() << __FUNCTION__ << " called";
#endif

    qDebug() << "cancel reinstall" << totalInstallFont << endl;
    emit m_signalManager->sendReInstallMessage(0);
    if (totalInstallFont > 0)
        Q_EMIT SignalManager::instance()->requestInstallAdded();
//ut000442 后面添加了关闭这个框的逻辑，不需要在这里关闭。
//    this->accept();
}

void DFInstallNormalWindow::onContinueInstall(const QStringList &continueInstallFontFileList)
{
#ifdef QT_QML_DEBUG
//    qDebug() << __FUNCTION__ << " called:" << continueInstallFontFileList;
#endif

//ut000442 安装重复字体过程开始时,将之前的进度条清空
    m_progressBar->setValue(0);
    m_installState = InstallState::reinstall;

    Q_EMIT batchReinstall(continueInstallFontFileList);
}

void DFInstallNormalWindow::onProgressChanged(const QString &familyName, const double &percent)
{
    if (familyName.isEmpty()) {
        return;
    }

    m_currentFontLabel->setText(familyName);

    m_progressBar->setValue(static_cast<int>(percent));
    m_progressBar->setTextVisible(false);

//    qDebug() << QString("font install progress: %1%").arg(percent);
//    ut000442 fix bug 21562.之前进度条到100的同时关闭这个窗口，导致
//    用户看不到进度条满的效果。
//    if (static_cast<int>(percent) == 100) {
//        QTimer::singleShot(50, this, [this]() {
//            this->close();
//        });
//    }
}
void DFInstallNormalWindow::showInstallErrDlg()
{
    m_pexceptionDlg = new DFInstallErrorDialog(this, m_errorList, m_AllSysFiles);

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
        m_pexceptionDlg->close();
        m_pexceptionDlg = nullptr;
    }

    this->close();
}
void DFInstallNormalWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        reject();
        close();
    }
}

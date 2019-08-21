#include "dfinstallnormalwindow.h"
#include "dfontmanager.h"
#include "globaldef.h"

#include <DLog>
#include <QFrame>
#include <QResizeEvent>
#include <QTimer>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

DFInstallNormalWindow::DFInstallNormalWindow(const QStringList &files, QWidget *parent)
    : DDialog(parent)
    , m_installFiles(files)
    , m_fontInfoManager(DFontInfoManager::instance())
    , m_fontManager(DFontManager::instance())
    , m_verifyTimer(new QTimer(this))
{
    initUI();
    initConnections();
}

void DFInstallNormalWindow::initUI()
{
    setFixedSize(QSize(380, 136));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(10, 0, 0, 0);

    QHBoxLayout *titleLayout = new QHBoxLayout(this);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0, 0, 0, 0);

    m_titleFrame = new QFrame(this);
    m_titleFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_titleFrame->setFixedHeight(50);

    m_logoLabel = new DLabel(this);
    m_logoLabel->setObjectName("logoLabel");
    m_logoLabel->setFixedSize(QSize(32, 32));
    m_logoLabel->setFocusPolicy(Qt::NoFocus);
    m_logoLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_logoLabel->setPixmap(QPixmap(":/images/deepin-font-manager.svg"));

    m_titleLabel = new DLabel(this);
    m_titleLabel->setObjectName("tileNameLabel");
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_titleLabel->setText("安装字体");

    // titleLayout->addSpacing(10);
    titleLayout->addWidget(m_logoLabel);
    titleLayout->addWidget(m_titleLabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    m_titleFrame->setLayout(titleLayout);

    mainLayout->addWidget(m_titleFrame, 0, Qt::AlignTop);

    m_progressStepLabel = new DLabel(this);
    m_progressStepLabel->setFixedHeight(20);
    m_progressStepLabel->setText("正在验证字体…");
    m_currentFontLabel = new DLabel(this);
    m_currentFontLabel->setFixedHeight(18);
    m_currentFontLabel->setText("SourceHanSansSC-ExtraLight.ttf");

    m_progressBar = new DProgressBar(this);
    m_progressBar->setFixedHeight(6);

    mainLayout->addSpacing(10);
    mainLayout->addWidget(m_progressStepLabel);
    mainLayout->addSpacing(7);
    mainLayout->addWidget(m_currentFontLabel);
    mainLayout->addSpacing(7);
    mainLayout->addWidget(m_progressBar);
    mainLayout->addSpacing(20);

    m_mainFrame = new QFrame(this);
    m_mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_mainFrame->setLayout(mainLayout);

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
        // Check installed & damaged font file here
        if (!verifyFontFiles()) {
            qDebug() << " start Exception dailog\n";
            DFInstallErrorDialog dlg;
            connect(&dlg, &DFInstallErrorDialog::onCancelInstall, this,
                    &DFInstallNormalWindow::onCancelInstall);
            connect(&dlg, &DFInstallErrorDialog::onContinueInstall, this,
                    &DFInstallNormalWindow::onContinueInstall);
            dlg.initInstallErrorFontViews(m_installErrorFontModelList);
            dlg.exec();

            return;
        }

        // Install the font list ,which may be changed in exception window
        batchInstall();
    });

    connect(m_fontManager, &DFontManager::batchInstall, this,
            &DFInstallNormalWindow::onProgressChanged);

    connect(m_fontManager, &DFontManager::batchInstall, this,
            &DFInstallNormalWindow::onProgressChanged);

    connect(m_fontManager, &DFontManager::installFinished, this, [=]() {
        // ToDo:
        //   May send signal to mainwindow refresh new installed font
        // QMIT notfiyRefresh;
        // this->close();
    });

    initVerifyTimer();
}

int DFInstallNormalWindow::verifyFontFiles()
{
    // debug
    DFontInfo *pfontInfo = nullptr;

    int damageFileCnt = 0;
    int installedFileCnt = 0;
    int newFileCnt = 0;

    m_installErrorFontModelList.clear();
    foreach (auto it, m_installFiles) {
        pfontInfo = m_fontInfoManager->getFontInfo(it);
        if (pfontInfo->isError) {
            damageFileCnt++;
            DFInstallErrorItemModel *itemModel = new DFInstallErrorItemModel;
            QFileInfo fileInfo(it);
            itemModel->bSelectable = false;
            itemModel->strFontFileName = fileInfo.fileName();
            itemModel->strFontFilePath = fileInfo.filePath();
            itemModel->strFontInstallStatus = QString("文件异常");
            m_installErrorFontModelList.push_back(itemModel);
            qDebug() << "verifyFontFiles->" << it << " :Damaged file";
        } else if (m_fontInfoManager->isFontInstalled(pfontInfo)) {
            installedFileCnt++;
            DFInstallErrorItemModel *itemModel = new DFInstallErrorItemModel;
            QFileInfo fileInfo(it);
            itemModel->bSelectable = true;
            itemModel->strFontFileName = fileInfo.fileName();
            itemModel->strFontFilePath = fileInfo.filePath();
            itemModel->strFontInstallStatus = QString("已安装相同版本");
            m_installErrorFontModelList.push_back(itemModel);
            qDebug() << "verifyFontFiles->" << it << " :Installed file";
        } else {
            newFileCnt++;
            qDebug() << "newFileCnt++" << it << " :new file";
        }
    }

    return !(damageFileCnt > 0 || installedFileCnt > 0);
}

void DFInstallNormalWindow::resizeEvent(QResizeEvent *event)
{
    DDialog::resizeEvent(event);
    qDebug() << "Dialog(" << event->size().width() << " ," << event->size().height() << ")";
    // m_titleFrame->resize(event->size().width() - 50, 50);
    m_mainFrame->resize(event->size().width(), event->size().height());
}

void DFInstallNormalWindow::batchInstall()
{
    m_fontManager->setType(DFontManager::Install);
    m_fontManager->setInstallFileList(m_installFiles);
    m_fontManager->start();
}

void DFInstallNormalWindow::onCancelInstall()
{
    qDebug() << "onCancelInstall";
    m_fontManager->terminate();
    m_fontManager->wait();

    this->accept();
}

void DFInstallNormalWindow::onContinueInstall(QStringList continueInstallFontFileList)
{
    qDebug() << "onContinueInstall" << continueInstallFontFileList;
    m_fontManager->setType(DFontManager::Install);
    m_fontManager->setInstallFileList(continueInstallFontFileList);
    m_fontManager->start();
}

void DFInstallNormalWindow::onProgressChanged(const QString &filePath, const double &percent)
{
    if (filePath.isEmpty()) {
        return;
    }

    DFontInfo *fontInfo = m_fontInfoManager->getFontInfo(filePath);
    m_currentFontLabel->setText(fontInfo->familyName);
    m_progressBar->setValue(static_cast<int>(percent));
}

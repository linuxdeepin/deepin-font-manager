#include "dfinstallnormalwindow.h"
#include "dfontmanager.h"
#include "dfontpreviewlistdatathread.h"
#include "dfinstallerrordialog.h"

#include <DApplication>
#include <DFontSizeManager>
#include <DTipLabel>

#include <QFileInfo>
#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

/*************************************************************************
 <Function>      DFInstallNormalWindow
 <Description>   构造函数
 <Author>
 <Input>
    <param1>     files           Description:待安装字体文件
    <param2>     parent          Description:父控件对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFInstallNormalWindow::DFInstallNormalWindow(const QStringList &files, QWidget *parent)
    : DFontBaseDialog(parent)
    , m_fontManager(DFontManager::instance())
    , m_dataThread(DFontPreviewListDataThread::instance())
{
    initUI();
    initConnections();

    Q_EMIT DFontPreviewListDataThread::instance()->requestBatchInstall(files);
    qDebug() << __FUNCTION__ << " end";
}

/*************************************************************************
 <Function>      ~DFInstallNormalWindow
 <Description>   析构函数处理函数
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFInstallNormalWindow::~DFInstallNormalWindow()
{
    qDebug() << __func__ << "start" << endl;
    this->hide();
    //如果有tab焦点且为取消或X号，则恢复tab焦点状态
    if (m_AddBtnHasTabs && !m_skipStateRecovery) {
        emit m_signalManager->requestSetTabFocusToAddBtn();
        //恢复标志位
        setAddBtnHasTabs(false);
    } else {
        //恢复标志位
        m_skipStateRecovery = false;
    }
    qDebug() << __func__ << "end" << this << endl;
}

/*************************************************************************
 <Function>      initUI
 <Description>   初始化主页面
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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
    DFontSizeManager::instance()->bind(m_progressStepLabel, DFontSizeManager::T6);
    m_progressStepLabel->setFixedHeight(m_progressStepLabel->fontMetrics().height());
    m_progressStepLabel->setText(DApplication::translate("NormalInstallWindow", "Verifying..."));

    m_currentFontLabel = new DTipLabel("", this);
    DFontSizeManager::instance()->bind(m_currentFontLabel, DFontSizeManager::T8);
    m_currentFontLabel->setAlignment(Qt::AlignLeft);
    m_currentFontLabel->setFixedHeight(m_currentFontLabel->fontMetrics().height());
    m_currentFontLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

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

/*************************************************************************
 <Function>      initConnections
 <Description>   初始化信号和槽connect连接函数
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::initConnections()
{
    connect(m_signalManager, &SignalManager::installDuringPopErrorDialog, m_dataThread, &DFontPreviewListDataThread::batchHalfwayInstall);

    connect(m_fontManager, &DFontManager::batchInstall, this, &DFInstallNormalWindow::onProgressChanged);

    connect(m_dataThread, &DFontPreviewListDataThread::installFinished, this, &DFInstallNormalWindow::onInstallFinished);

    connect(m_dataThread, &DFontPreviewListDataThread::reInstallFinished, this, &DFInstallNormalWindow::onReInstallFinished);

    connect(qApp, &QApplication::fontChanged, this, [ = ]() {
        m_progressStepLabel->setFixedHeight(m_progressStepLabel->fontMetrics().height());
        m_currentFontLabel->setFixedHeight(m_currentFontLabel->fontMetrics().height());
    });
}

/*************************************************************************
 <Function>      checkShowMessage
 <Description>   根据字体安装或重复安装状态更新标志位getInstallMessage
                 新安装的字体在安装完成时 getInstallMessage置为true。重复安装的字体安装完成时 getReInstallMessage置为true
                 列表刷新完成后 m_installAdded置为true。三者都为true时表示一次安装过程结束。
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::nextInstallStep()
{
    qDebug() << "Install over" << endl;

    if (getInstallMessage == true && getReInstallMessage == true) {
        qDebug() << "install refresh over";
        finishInstall();
    } else if (getInstallMessage == true && m_popedInstallErrorDialg == false) {
        if (m_dataThread->ifNeedShowExceptionWindow(m_isNeedSkipException)) {
            qDebug() << "need reinstall " << endl;
            showInstallErrDlg();
        } else {
            qDebug() << "no need reinstall" << endl;
            //不需恢复添加按钮tab状态
            m_skipStateRecovery = true;
            finishInstall();
        }
    }
}

/**
* @brief DFInstallNormalWindow::finishInstall 所有字体安装完成
* @param void
* @return void
*/
inline void DFInstallNormalWindow::finishInstall()
{
    getInstallMessage = false;
    getReInstallMessage = false;

    m_cancelInstall = false;
    close();
}

/*************************************************************************
 <Function>      closeEvent
 <Description>   重新实现关闭事件处理函数-停止字体管理线程
 <Author>
 <Input>
    <param1>     event           Description:事件对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);
    qDebug() << __FUNCTION__ << m_cancelInstall;

    if (m_cancelInstall) {
        m_fontManager->cancelInstall();
        reject();
    }
}


/*************************************************************************
 <Function>      onCancelInstall
 <Description>   重装验证页面，取消按钮处理函数-QStringList()
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::onCancelInstall()
{
#ifdef QT_QML_DEBUG
    qDebug() << __FUNCTION__ << " called";
#endif
    m_errCancelInstall = true;
    Q_EMIT m_dataThread->requestCancelReinstall();
}

/*************************************************************************
 <Function>      onContinueInstall
 <Description>   重装验证页面，继续按钮处理函数-继续安装
 <Author>
 <Input>
    <param1>     continueInstallFontFileList    Description:重装字体文件列表
 <Return>        null                           Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::onContinueInstall(const QStringList &continueInstallFontFileList)
{
#ifdef QT_QML_DEBUG
//    qDebug() << __FUNCTION__ << " called:" << continueInstallFontFileList;
#endif

//ut000442 安装重复字体过程开始时,将之前的进度条清空
    m_progressBar->setValue(0);
    Q_EMIT DFontPreviewListDataThread::instance()->requestBatchReInstall(continueInstallFontFileList);
    //继续安装不需恢复添加按钮tab聚焦状态
    m_skipStateRecovery = true;
}

/*************************************************************************
 <Function>      onProgressChanged
 <Description>   刷新安装进度显示内容
 <Author>
 <Input>
    <param1>     familyName      Description:当前字体组名
    <param2>     percent         Description:用于计算百分百参数
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::onProgressChanged(const QString &familyName, const double &percent)
{
    if (familyName.isEmpty()) {
        return;
    }

//    qDebug() << __FUNCTION__ << familyName << percent;
    m_currentFontLabel->setText(familyName);

    m_progressBar->setValue(static_cast<int>(percent));
    m_progressBar->setTextVisible(false);
}

/*************************************************************************
 <Function>      onInstallFinished
 <Description>   字体安装后的处理函数
 <Author>
 <Input>
    <param1>     state           Description:完成状态
    <param2>     fileList        Description:安装字体文件列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::onInstallFinished()
{
    qDebug() << __FUNCTION__;
    getInstallMessage = true;
    nextInstallStep();
}

/*************************************************************************
 <Function>      onReInstallFinished
 <Description>   字体重新安装后的处理函数
 <Author>
 <Input>
    <param1>     state           Description:完成状态
    <param2>     fileList        Description:重装字体文件列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::onReInstallFinished()
{
    getReInstallMessage = true;
    qDebug() << __FUNCTION__;
    if (getInstallMessage == true && getReInstallMessage == true) {
        qDebug() << "install refresh over";
        nextInstallStep();
    }
}

/*************************************************************************
 <Function>      showInstallErrDlg
 <Description>   弹出字体验证框
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::showInstallErrDlg()
{
    m_popedInstallErrorDialg = true;

    m_pexceptionDlg = new DFInstallErrorDialog(this, m_dataThread->m_errorList);
    m_pexceptionDlg->setParent(this);
    //取消安装
    connect(m_pexceptionDlg, &DFInstallErrorDialog::onCancelInstall, this,
            &DFInstallNormalWindow::onCancelInstall);
    //继续安装
    connect(m_pexceptionDlg, &DFInstallErrorDialog::onContinueInstall, this,
            &DFInstallNormalWindow::onContinueInstall);

    connect(m_pexceptionDlg, &DFInstallErrorDialog::destroyed, this, [ = ]() {
        if (!m_errCancelInstall)
            return;
        qDebug() << " DFInstallErrorDialog cancel called";
        Q_EMIT DFontManager::instance()->cacheFinish();
        finishInstall();
    });

    qDebug() << geometry().center() << "+" << m_pexceptionDlg->rect().center() << endl;

    m_pexceptionDlg->move(geometry().center() - m_pexceptionDlg->rect().center());
    m_pexceptionDlg->exec();
}

/*************************************************************************
 <Function>      setAddBtnHasTabs
 <Description>   记录addbutton是否有tab焦点
 <Author>
 <Input>
    <param1>     AddBtnHasTabs   Description:是否有焦点
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::setAddBtnHasTabs(bool AddBtnHasTabs)
{
    m_AddBtnHasTabs = AddBtnHasTabs;
}

/*************************************************************************
 <Function>      setSkipException
 <Description>   设置m_isNeedSkipException标志位状态
 <Author>
 <Input>
    <param1>     skip            Description:状态类型
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::setSkipException(bool skip)
{
    m_isNeedSkipException = skip;
}

/*************************************************************************
 <Function>      breakInstalltion
 <Description>   打断安装操作-关闭验证框
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::breakInstalltion()
{
    //Todo:
    //   Just close the installtion window
    if (m_pexceptionDlg->isVisible()) {
        m_pexceptionDlg->close();
        m_pexceptionDlg = nullptr;
    }

    this->hide();
}

/*************************************************************************
 <Function>      keyPressEvent
 <Description>   键盘press事件处理函数-esc键退出安装和关闭窗口
 <Author>
 <Input>
    <param1>     event           Description:事件对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallNormalWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        reject();
        close();
    }
}

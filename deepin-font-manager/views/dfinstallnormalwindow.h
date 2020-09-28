#ifndef DFINSTALLNORMALWINDOW_H
#define DFINSTALLNORMALWINDOW_H

#include "dfontbasedialog.h"
#include "dfontinfomanager.h"
#include "signalmanager.h"

#include <DFrame>
#include <DLabel>
#include <DProgressBar>

#include <QTimer>

DWIDGET_USE_NAMESPACE

//类声明
class DFontManager;
class DFontPreviewListDataThread;
class DFInstallErrorDialog;

/*************************************************************************
 <Class>         DFInstallNormalWindow
 <Description>   负责字体正常安装、中途安装、重复安装、取消安装等进程协调与调度工作。
 <Author>
 <Note>          null
*************************************************************************/
class DFInstallNormalWindow : public DFontBaseDialog
{
    Q_OBJECT
public:
    DFInstallNormalWindow(const QStringList &files = QStringList(), QWidget *parent = nullptr);
    ~DFInstallNormalWindow()Q_DECL_OVERRIDE;
    //设置m_isNeedSkipException标志位状态
    void setSkipException(bool skip);
    //打断安装操作-关闭验证框
    void breakInstalltion();

    void setAddBtnHasTabs(bool AddBtnHasTabs);

protected:
    static constexpr int VERIFY_DELYAY_TIME = 10;
    //初始化主页面
    void initUI();
    //初始化信号和槽connect连接函数
    void initConnections();
    // 根据字体安装或重复安装状态更新标志位getInstallMessage
    void nextInstallStep();
    //结束安装
    void finishInstall();

private:
    void reInstallFinished(const QStringList &fileList);
    void keyPressEvent(QKeyEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;

protected slots:
    //刷新安装进度显示内容
    void onProgressChanged(const QString &familyName, const double &percent);
    //字体安装后的处理函数
    void onInstallFinished();
    //字体重新安装后的处理函数
    void onReInstallFinished();
    //重装验证页面，取消按钮处理函数
    void onCancelInstall();
    //重装验证页面，继续按钮处理函数-继续安装
    void onContinueInstall(const QStringList &continueInstallFontFileList);
    //弹出字体验证框
    void showInstallErrDlg();

signals:
    //信号-安装完成
    void finishFontInstall(const QStringList &fileList);

private:
    bool getInstallMessage = false;
    bool getReInstallMessage = false;
    bool m_popedInstallErrorDialg = false;

    // Skip popup exception dialog if true
    bool m_isNeedSkipException {false};
    bool m_cancelInstall = true;
    bool m_errCancelInstall = false;
    bool m_AddBtnHasTabs{false};
    //是否无需恢复添加按钮tab状态
    bool m_skipStateRecovery{false};

    DFontManager *m_fontManager;
    SignalManager *m_signalManager = SignalManager::instance();

    QWidget *m_mainFrame {nullptr};

    DLabel *m_progressStepLabel {nullptr};
    DLabel *m_currentFontLabel {nullptr};
    DProgressBar *m_progressBar {nullptr};

    DFInstallErrorDialog *m_pexceptionDlg {nullptr};
    DFontPreviewListDataThread *m_dataThread;
};

#endif  // DFINSTALLNORMALWINDOW_H

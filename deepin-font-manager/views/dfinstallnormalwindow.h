#ifndef DFINSTALLNORMALWINDOW_H
#define DFINSTALLNORMALWINDOW_H

#include "dfontbasedialog.h"
#include "dfmdbmanager.h"
#include "dfinstallerrordialog.h"
#include "dfontinfomanager.h"
#include "signalmanager.h"

#include <QStringList>
#include <QHash>
#include <QTimer>

#include <DDialog>
#include <DFrame>
#include <DLabel>
#include <DProgressBar>
#include <DMessageManager>

DWIDGET_USE_NAMESPACE
//类声明
class DFontManager;
class DFontInfoManager;
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
    //判断当前字体是否为系统字体
    bool isSystemFont(DFontInfo &f);

protected:
    static constexpr int VERIFY_DELYAY_TIME = 50;
    //初始化主页面
    void initUI();
    //初始化信号和槽connect连接函数
    void initConnections();
    //从数据库中读取系统字体，用于之后的判断
    void getAllSysfiles();
    //初始化文件过滤定时器
    void initVerifyTimer();
    //字体文件过滤器，过滤后得到需要新安装的字体，重复安装字体，损毁字体，系统字体,以及字体验证框弹出时安装的字体
    void verifyFontFiles(bool isHalfwayInstall = false);
    //检测是否要弹出字体验证框，存在重复安装字体，系统字体时，损坏字体时弹出字体验证框
    bool ifNeedShowExceptionWindow() const;
    // 根据字体安装或重复安装状态更新标志位getInstallMessage
    void checkShowMessage();
    //获取新增字体文件
    void getNoSameFilesCount(const QStringList &filesList);
    //重新实现大小改变事件处理函数
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
    //重新实现重绘函数-根据字体属性刷新进度标签高度
    void paintEvent(QPaintEvent *event) override;

private:
    inline QString getFamilyName(const DFontInfo &fontInfo)
    {
        QString familyName = (fontInfo.familyName.isEmpty() || fontInfo.familyName.contains(QChar('?'))) ? fontInfo.fullname : fontInfo.familyName;
        return familyName;
    }

protected slots:
    //批量安装处理函数
    void batchInstall();
    //批量重新安装处理函数
    void batchReInstall(const QStringList &reinstallFiles);
    //字体验证框弹出时在文件管理器进行安装
    void batchHalfwayInstall(const QStringList &filelist);
    //重装验证页面，继续按钮处理函数-继续批量安装
    void batchReInstallContinue();
    //刷新安装进度显示内容
    void onProgressChanged(const QString &familyName, const double &percent);
    //字体安装后的处理函数
    void onInstallFinished(int state, const QStringList &fileList);
    //字体重新安装后的处理函数
    void onReInstallFinished(int state, const QStringList &fileList);
    //重装验证页面，取消按钮处理函数-QStringList()我安装了个寂寞
    void onCancelInstall();
    //重装验证页面，继续按钮处理函数-继续安装
    void onContinueInstall(const QStringList &continueInstallFontFileList);
    //弹出字体验证框
    void showInstallErrDlg();

signals:
    //信号-批量重新安装
    void batchReinstall(const QStringList &reinstallFiles);
    //信号-安装完成
    void finishFontInstall(const QStringList &fileList);

private:
    /*************************************************************************
     <Enum>          InstallState
     <Description>   字体安装状态
     <Author>
     <Value>
        <Value1>     Install               Description:安装
        <Value2>     reinstall             Description:重新安装
        <Value2>     damaged               Description:损坏字体
     <Note>          null
    *************************************************************************/
    enum InstallState { Install, reinstall, damaged };

    QStringList m_AllSysFilesfamilyName;

    QStringList m_installFiles;
    QStringList m_installedFiles;
    QStringList m_newInstallFiles;
    QStringList m_damagedFiles;
    QStringList m_systemFiles;
    QStringList m_outfileList;
    QStringList m_errorList;
    QStringList m_finishInstallFiles;

    QStringList m_installedFontsFamilyname;
    QStringList m_halfInstalledFiles;
    QStringList m_newHalfInstalledFiles;
    QStringList m_oldHalfInstalledFiles;

    QList<DFontInfo> m_installedFilesFontinfo;

    int totalSysFontCount = 0;
    bool getInstallMessage = false;
    bool getReInstallMessage = false;
    bool m_popedInstallErrorDialg = false;
//    bool m_installFinishSent = false;
//    bool m_installAdded = false;

    // Skip popup exception dialog if true
    bool m_isNeedSkipException {false};
    bool m_isFristThreadEnd = false;

    InstallState m_installState {Install};

    DFontInfoManager *m_fontInfoManager;
    DFontManager *m_fontManager;
    SignalManager *m_signalManager = SignalManager::instance();

    DLabel *m_logoLabel {nullptr};
    DLabel *m_titleLabel {nullptr};
    QWidget *m_titleFrame {nullptr};
    QWidget *m_mainFrame {nullptr};

    DLabel *m_progressStepLabel {nullptr};
    DLabel *m_currentFontLabel {nullptr};
    DProgressBar *m_progressBar {nullptr};

    DFInstallErrorDialog *m_pexceptionDlg {nullptr};
    void keyPressEvent(QKeyEvent *event) override;
    virtual void closeEvent(QCloseEvent *event) override;
    QScopedPointer<QTimer> m_verifyTimer {nullptr};
};

#endif  // DFINSTALLNORMALWINDOW_H

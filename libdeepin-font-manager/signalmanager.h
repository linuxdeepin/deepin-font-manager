#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H
#include <QObject>
#include <QString>

/*************************************************************************
 <Class>         SignalManager
 <Description>   信号管理类-负责信号分发管理
 <Author>
 <Note>          null
*************************************************************************/
class SignalManager : public QObject
{
    Q_OBJECT
public:
    static SignalManager *instance();
    ~SignalManager();
signals:
    //触发安装提示信息窗口
    void showInstallFloatingMessage(int totalCount);
    //触发安装提示操作
    void sendInstallMessage(const QStringList &m_finishFileList);
    //触发重装提示信息
    void sendReInstallMessage(const QStringList &m_finishFileList);
    //触发安装完成操作
    void installOver(int successInstallCount);
    //安装验证页面，触发取消继续安装操作
    void cancelInstall();
    //触发完成安装信号
    void finishFontInstall(const QStringList &fileList);
    //触发大小改变信号
    void sizeChange(int height);
    //触发菜单项切换显示
    void currentFontGroup(int currentFontGroup);
    //触发预览内容变化信号
    void prevFontChanged();
    //触发当前选中区域更新信号
    void refreshCurRect();
    //触发安装请求
    void requestInstallAdded();
    //触发弹出字体验证框请求
    void popInstallErrorDialog();
    //触发字体验证框隐藏信号
    void hideInstallErrorDialog();
    //触发字体验证框列表刷新信号
    void updateInstallErrorListview(QStringList &errorFileList, QStringList &halfInstalledFiles,
                                    QStringList &addHalfInstalledFiles, QStringList &oldHalfInstalledFiles);
    //触发字体验证框弹出中途安装信号
    void installDuringPopErrorDialog(const QStringList &fileList);
    //更新安装进程状态标志位信号
    void setIsJustInstalled();
    //触发安装进度页面关闭信号
    void closeInstallDialog();
    //触发显示安装验证框页面信号
    void showInstallErrorDialog();
    //触发左侧菜单栏是否可滚动信号
    void setSpliteWidgetScrollEnable(bool isInstalling);
    //触发字体列表刷新判断信号
    void freshListView();
    //触发字体列表改变信号
    void changeView();
    //触发删除确认取消信号
    void cancelDel();
    //触发更新预览大小跟随滑块信号
    void fontSizeRequestToSlider();
    //触发切换焦点至菜单
    void requestSetLeftSiderBarFocus();

private:
    explicit SignalManager(QObject *parent = nullptr);

private:
    static SignalManager *m_signalManager;
};

#endif // SIGNALMANAGER_H

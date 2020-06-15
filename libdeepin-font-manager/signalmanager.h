#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H
#include <QObject>
#include <QString>

class SignalManager : public QObject
{
    Q_OBJECT
public:
    static SignalManager *instance();

signals:
    void popUninstallDialog();
    void updateUninstallDialog(const QString &fontName, int index, int totalCount);
    void closeUninstallDialog();
    void showInstallFloatingMessage(int totalCount);
    void sendInstallMessage(const QStringList &m_finishFileList);
    void sendReInstallMessage(const QStringList &m_finishFileList);
    void startToInsert();
    void installOver(int successInstallCount);
    void finishFontInstall(const QStringList &fileList);
    void trigerCollect(bool favorite);
    void sizeChange(int height);
    void currentFontGroup(int currentFontGroup);
    void prevFontChanged();
    void refreshCurRect();
    void requestInstallAdded();
    void popInstallErrorDialog();
    void hideInstallErrorDialog();
    void updateInstallErrorListview(QStringList &errorFileList, QStringList &halfInstalledFiles,
                                    QStringList &addHalfInstalledFiles, QStringList &oldHalfInstalledFiles);
    void installDuringPopErrorDialog(const QStringList &fileList);
    void setIsJustInstalled();
    void closeInstallDialog();
    void showInstallErrorDialog();
    void setSpliteWidgetScrollEnable(bool isInstalling);
    void freshListView();
    void refreshFocus(bool isJustInstalled, int count);
    void changeView();
    void cancelDel();/*删除确认取消*/
    void fontSizeRequestToSlider();
private:
    explicit SignalManager(QObject *parent = nullptr);

private:
    static SignalManager *m_signalManager;
};

#endif // SIGNALMANAGER_H

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
    void sendInstallMessage(int totalCount);
    void sendReInstallMessage(int totalCount);
    void startToInsert();
    void finishFontInstall(const QStringList &fileList);
    void trigerCollect(bool favorite);
    void sizeChange(int height);
    void currentFontGroup(int currentFontGroup);
    void prevFontChanged();
    void refreshCurRect();
    void requestInstallAdded();
    void setIsJustInstalled();
    void closeInstallDialog();
    void refreshFocus(int count);//刷新聚焦状态
    void changeView();
private:
    explicit SignalManager(QObject *parent = nullptr);

private:
    static SignalManager *m_signalManager;
};

#endif // SIGNALMANAGER_H

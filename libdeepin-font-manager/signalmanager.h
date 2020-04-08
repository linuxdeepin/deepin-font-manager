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
    void deledFont(QString &fontPath);
    void updateUninstallDialog(QString &fontName, int index, int totalCount);
    void closeUninstallDialog();
    void showInstallFloatingMessage(int totalCount);
    void sendInstallMessage(int totalCount);
    void sendReInstallMessage(int totalCount);
    void startToInsert();
    void finishFontInstall(const QStringList &fileList);


private:
    explicit SignalManager(QObject *parent = nullptr);

private:
    static SignalManager *m_signalManager;
};

#endif // SIGNALMANAGER_H

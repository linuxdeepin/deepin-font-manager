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
    void showFloatingMessage(int totalCount, int systemFontCount);
    void sendInstallMessage(int totalCount, int systemFontCount);
    void sendReInstallMessage(int totalCount, int systemFontCount);


private:
    explicit SignalManager(QObject *parent = nullptr);

private:
    static SignalManager *m_signalManager;
};

#endif // SIGNALMANAGER_H

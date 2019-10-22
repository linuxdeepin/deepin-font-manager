#ifndef SINGLEFONTAPPLICATION_H
#define SINGLEFONTAPPLICATION_H

#include <DApplication>
#include <DMainWindow>

DWIDGET_USE_NAMESPACE

class QLocalServer;
class SingleFontApplication : public DApplication
{
    Q_OBJECT
public:
    explicit SingleFontApplication(int &argc, char **argv);

    ~SingleFontApplication();

    bool isRunning();      //Instance already exist
    void activateWindow();
    void setMainWindow(DMainWindow *mainWindow);

private slots:
    void newLocalConnection();

private:
    void initLocalConnection();
    void newLocalServer();
    void parseCmdLine();

    QStringList m_selectedFiles;

    QScopedPointer<DMainWindow> m_qspMainWnd;  // MainWindow ptr
    QScopedPointer<DMainWindow> m_qspQuickWnd;  // QuickInstall Window ptr

    bool m_isRunning {false};   //Instance already exist
    QScopedPointer<QLocalServer> m_qspLocalServer;     // socket Server
    QString m_serverName;            // Server Name
};

#endif // SINGLEFONTAPPLICATION_H

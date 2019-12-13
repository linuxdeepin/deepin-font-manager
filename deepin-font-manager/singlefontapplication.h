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

    void activateWindow();
    void setMainWindow(DMainWindow *mainWindow);
    bool parseCmdLine();

private slots:
    void onNewProcessInstance(qint64 pid, const QStringList &arguments);

private:

    QStringList m_selectedFiles;

    QScopedPointer<DMainWindow> m_qspMainWnd;  // MainWindow ptr
    QScopedPointer<DMainWindow> m_qspQuickWnd;  // QuickInstall Window ptr
};

#endif // SINGLEFONTAPPLICATION_H

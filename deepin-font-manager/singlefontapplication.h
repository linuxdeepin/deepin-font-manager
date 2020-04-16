#ifndef SINGLEFONTAPPLICATION_H
#define SINGLEFONTAPPLICATION_H

#include <DApplication>
#include <DMainWindow>
#include <DSpinner>

#include <QSet>

DWIDGET_USE_NAMESPACE

//class DSpinner;
class QLocalServer;

class SingleFontApplication : public DApplication
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FontManager")
public:
    explicit SingleFontApplication(int &argc, char **argv);

    ~SingleFontApplication();

    void activateWindow();
    void setMainWindow(DMainWindow *mainWindow);
    bool parseCmdLine();

public slots:
    Q_SCRIPTABLE void installFonts(QStringList fontPathList);

private slots:
    void onNewProcessInstance(qint64 pid, const QStringList &arguments);
    void slotBatchInstallFonts();

private:
    QStringList m_selectedFiles;

    QScopedPointer<DMainWindow> m_qspMainWnd;  // MainWindow ptr
    QScopedPointer<DMainWindow> m_qspQuickWnd;  // QuickInstall Window ptr

    QSet<QString>   waitForInstallSet;   /* 可能存在多次安装相同的一个字体，将会接收到多次消息，QSet可以用来剔除重复的安装请求 UT000591 */
};

#endif // SINGLEFONTAPPLICATION_H

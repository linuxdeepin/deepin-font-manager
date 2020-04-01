#include "singlefontapplication.h"
#include "views/dfontmgrmainwindow.h"
#include "views/dfquickinstallwindow.h"
#include "globaldef.h"
#include "utils.h"

#include <QFileInfo>
#include <QLocalServer>
#include <QLocalSocket>
#include <QCommandLineParser>

#include <DWidgetUtil>
#include <DGuiApplicationHelper>
#include <DSpinner>

#include <sys/types.h>
#include <unistd.h>

DCORE_USE_NAMESPACE

SingleFontApplication::SingleFontApplication(int &argc, char **argv)
    : DApplication(argc, argv)
    , m_qspMainWnd(nullptr)
    , m_qspQuickWnd(nullptr)
{
    connect(DGuiApplicationHelper::instance()
            , &DGuiApplicationHelper::newProcessInstance, this
            , &SingleFontApplication::onNewProcessInstance);

    initWaitForInstallTimer();
}

SingleFontApplication::~SingleFontApplication()
{
}

void SingleFontApplication::setMainWindow(DMainWindow *mainWindow)
{
    m_qspMainWnd.reset(mainWindow);
    m_qspMainWnd->setMinimumSize(DEFAULT_WINDOWS_WIDTH, DEFAULT_WINDOWS_HEIGHT);
    m_qspMainWnd->setWindowIcon(QIcon::fromTheme(DEEPIN_FONT_MANAGER));
}

bool SingleFontApplication::parseCmdLine()
{
    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin Font Manager.");
    parser.addHelpOption();
    parser.addVersionOption();

    parser.process(*this);

    //Clear old parameter
    if (!m_selectedFiles.isEmpty()) {
        m_selectedFiles.clear();
    }

    QStringList paraList = parser.positionalArguments();
    for (auto it : paraList) {
        if (Utils::isFontMimeType(it)) {
            m_selectedFiles.append(it);
        }
    }

    if (paraList.size() > 0 && m_selectedFiles.size() == 0) {
        qDebug() << __FUNCTION__ << "invalid :" << paraList;
        return false;
    }

    qDebug() << __FUNCTION__ << m_selectedFiles;
    return true;
}

void SingleFontApplication::activateWindow()
{
    //If quick install mode
    if (m_selectedFiles.size() < 0) {
        qDebug() << "Active quick install window to install file:" << m_selectedFiles;

        //Hide normal window in quick mode
        if (nullptr != m_qspMainWnd.get()) {
            //Force quit installtion
            reinterpret_cast<DFontMgrMainWindow *>(
                m_qspMainWnd.get())->forceNoramlInstalltionQuitIfNeeded();

            m_qspMainWnd->hide();
        }

        //Init quick window at first time
        if (nullptr == m_qspQuickWnd.get()) {
            m_qspQuickWnd.reset(new DFQuickInstallWindow());
            Dtk::Widget::moveToCenter(m_qspQuickWnd.get());

            DFQuickInstallWindow *qw = qobject_cast<DFQuickInstallWindow *>(m_qspQuickWnd.get());

            connect(qw, &DFQuickInstallWindow::requestShowMainWindow, this, [ = ](QStringList fileList) {
                qDebug() << "requestShowMainWindow " << fileList;
                if (nullptr == m_qspMainWnd.get()) {
                    m_qspMainWnd.reset(new
                                       DFontMgrMainWindow());
                    int windowWidth = reinterpret_cast<DFontMgrMainWindow *>(
                                          m_qspMainWnd.get())->m_winWidth;
                    int windowHeight = reinterpret_cast<DFontMgrMainWindow *>(
                                           m_qspMainWnd.get())->m_winHight;

                    m_qspMainWnd->setMinimumSize(DEFAULT_WINDOWS_WIDTH, DEFAULT_WINDOWS_HEIGHT);
                    if (DEFAULT_WINDOWS_WIDTH <= windowWidth && DEFAULT_WINDOWS_HEIGHT <= windowHeight) {
                        m_qspMainWnd->resize(windowWidth, windowHeight);
                    }

                    Dtk::Widget::moveToCenter(m_qspMainWnd.get());

                    m_qspMainWnd->show();
                    DFontMgrMainWindow *mw = qobject_cast<DFontMgrMainWindow *>(m_qspMainWnd.get());
                    mw->setFileList(fileList);
                } else {
                    m_qspMainWnd->setWindowState(Qt::WindowActive);
                    m_qspMainWnd->activateWindow(); // Reactive main window
                    //m_qspMainWnd->resize(DEFAULT_WINDOWS_WIDTH, DEFAULT_WINDOWS_HEIGHT);
                }
            });

            m_qspQuickWnd->show();
        } else {
            m_qspQuickWnd->setWindowState(Qt::WindowActive);
            m_qspQuickWnd->activateWindow(); // Reactive main window
        }

        QMetaObject::invokeMethod(m_qspQuickWnd.get(), "fileSelectedInSys", Qt::QueuedConnection,
                                  Q_ARG(QStringList, m_selectedFiles));

    } else {
        qDebug() << "Active quick install window to install file:" << m_selectedFiles;
        //Hide quick window in normal mode
        if (nullptr != m_qspQuickWnd.get()) {
            m_qspQuickWnd->hide();
        }

        //Init Normal window at first time
        if (nullptr == m_qspMainWnd.get()) {
            m_qspMainWnd.reset(new DFontMgrMainWindow());
            int windowWidth = reinterpret_cast<DFontMgrMainWindow *>(
                                  m_qspMainWnd.get())->m_winWidth;
            int windowHeight = reinterpret_cast<DFontMgrMainWindow *>(
                                   m_qspMainWnd.get())->m_winHight;
            //.toInt(&hWinDataStatus);
            m_qspMainWnd->setMinimumSize(DEFAULT_WINDOWS_WIDTH, DEFAULT_WINDOWS_HEIGHT);
            if (DEFAULT_WINDOWS_WIDTH <= windowWidth && DEFAULT_WINDOWS_HEIGHT <= windowHeight) {
                m_qspMainWnd->resize(windowWidth, windowHeight);
            }

            Dtk::Widget::moveToCenter(m_qspMainWnd.get());

            m_qspMainWnd->show();
        } else {
            m_qspMainWnd->setWindowState(Qt::WindowActive);
            m_qspMainWnd->activateWindow(); // Reactive main window
            //m_qspMainWnd->resize(DEFAULT_WINDOWS_WIDTH, DEFAULT_WINDOWS_HEIGHT);
        }
        //For: Drag files on task bar app icon
        //need start installtion flow
        if (m_selectedFiles.size() > 0) {
            QMetaObject::invokeMethod(m_qspMainWnd.get(), "fileSelectedInSys", Qt::QueuedConnection,
                                      Q_ARG(QStringList, m_selectedFiles));
        }
    }
}

void SingleFontApplication::slotBatchInstallFonts()
{
    reinterpret_cast<DFontMgrMainWindow *>(m_qspMainWnd.get())->waitForInstallSpinner->stop();
    reinterpret_cast<DFontMgrMainWindow *>(m_qspMainWnd.get())->waitForInstallSpinner->hide();
    activateWindow();
    m_selectedFiles.clear();
}

void SingleFontApplication::initWaitForInstallTimer()
{
    waitForNextFontTimer = new QTimer();
    waitForNextFontTimer->setSingleShot(true);
    connect(waitForNextFontTimer, &QTimer::timeout, this, &SingleFontApplication::slotBatchInstallFonts);
}

void SingleFontApplication::installFonts(QStringList fontPathList)
{
    /* 临时方案，等文管那边改好了右键-》打开的方式，可以把这个删除 UT000591 */
    reinterpret_cast<DFontMgrMainWindow *>(m_qspMainWnd.get())->waitForInstallSpinner->start();
    reinterpret_cast<DFontMgrMainWindow *>(m_qspMainWnd.get())->waitForInstallSpinner->show();
    /* 选中多个字体，右键->打开，目前后台每次只发送一个字体路径，启用定时器等待接收全部字体后，批量安装 UTOOO591 */
    waitForNextFontTimer->stop();
    for (QString fontPath : fontPathList) {
        if (Utils::isFontMimeType(fontPath)) {
            m_selectedFiles.append(fontPath);
        }
    }
    waitForNextFontTimer->start(1000);
}

/* Deprecated ut000591 */
void SingleFontApplication::onNewProcessInstance(qint64 pid, const QStringList &arguments)
{
    Q_UNUSED(pid);

    //clear old file list
    m_selectedFiles.clear();

    //<app_excename> <file list>
    //1.Skip app-exce name p=0
    //2.Check font file MIME,ignore invalid file.
    for (int p = 1; p < arguments.size(); p++) {
        if (Utils::isFontMimeType(arguments[p])) {
            m_selectedFiles.append(arguments[p]);
        }
    }
    activateWindow();
}

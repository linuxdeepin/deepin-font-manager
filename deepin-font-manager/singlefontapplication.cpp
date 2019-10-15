#include "singlefontapplication.h"
#include "views/dfontmgrmainwindow.h"
#include "globaldef.h"

#include <QFileInfo>
#include <QLocalServer>
#include <QLocalSocket>
#include <QCommandLineParser>

#include <DWidgetUtil>

#include <sys/types.h>
#include <unistd.h>

DCORE_USE_NAMESPACE

#define TIME_OUT                (500)    // 500ms

SingleFontApplication::SingleFontApplication(int &argc, char **argv)
    :DApplication (argc, argv)
    ,m_qspMainWnd(nullptr)
    ,m_qspLocalServer(nullptr)
{
    m_serverName = QFileInfo(QCoreApplication::applicationFilePath()).fileName();

    initLocalConnection();
}

SingleFontApplication::~SingleFontApplication()
{
}

bool SingleFontApplication::isRunning() {
    return m_isRunning;
}

void SingleFontApplication::setMainWindow(DMainWindow *mainWindow)
{
    m_qspMainWnd.reset(mainWindow);
    m_qspMainWnd->setMinimumSize(DEFAULT_WINDOWS_WIDTH, DEFAULT_WINDOWS_HEIGHT);
    m_qspMainWnd->setWindowIcon(QIcon::fromTheme(DEEPIN_FONT_MANAGER));
}

void SingleFontApplication::parseCmdLine(){
    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin Font Manager.");
    parser.addHelpOption();
    parser.addVersionOption();


    parser.process(*this);

    //Clear old parameter
    if (!m_selectedFiles.isEmpty()) {
        m_selectedFiles.clear();
    }

    m_selectedFiles = parser.positionalArguments();

    qDebug() << __FUNCTION__ << m_selectedFiles;
}

void SingleFontApplication::newLocalConnection() {
    QScopedPointer<QLocalSocket> socket(m_qspLocalServer->nextPendingConnection());

    if(socket) {
        socket->waitForReadyRead(2*TIME_OUT);

        //Read the parameter from other instance
        QTextStream stream(socket.get());

        QString selectFile = stream.readAll();

        qDebug() << "Exist instance (" << getpid() << ")\n"
                 << "New instance parameter:" << selectFile
                 << "Old instance parameter:" << m_selectedFiles;

        m_selectedFiles.clear();

        if (!selectFile.isEmpty()) {
            m_selectedFiles.append(selectFile);
        }

        activateWindow();
    }
}

void SingleFontApplication::initLocalConnection() {
    m_isRunning = false;

    QLocalSocket socket;
    socket.connectToServer(m_serverName);
    if(socket.waitForConnected(TIME_OUT)) {
        qDebug() << m_serverName + "already running.\n";

        m_isRunning = true;

        //Parse the command line parametes and send it
        // to the exist instance.
        parseCmdLine();

        //send empty if no file parameter
        QString selectedFile("");
        if (m_selectedFiles.size() > 0) {
            selectedFile = m_selectedFiles.at(0);
        }

        qDebug() << "New instance (" << getpid() << ") : parameter:" << selectedFile;

        socket.write(selectedFile.toUtf8());
        socket.flush();

        return;
    }

    //Server not exist,create server
    newLocalServer();

    //First time start,get command line parameter
    parseCmdLine();
}

void SingleFontApplication::newLocalServer() {
    m_qspLocalServer.reset(new QLocalServer(this));

    connect(m_qspLocalServer.get(), &QLocalServer::newConnection,
            this, &SingleFontApplication::newLocalConnection);

    if(!m_qspLocalServer->listen(m_serverName)) {
        // Try to listen again, app crash may lead server don't be cleard
        //so remove it directly
        if(m_qspLocalServer->serverError() == QAbstractSocket::AddressInUseError) {
            QLocalServer::removeServer(m_serverName);
            m_qspLocalServer->listen(m_serverName);
        }
    }
}

void SingleFontApplication::activateWindow() {
    if(m_qspMainWnd) {
        m_qspMainWnd->show();
        m_qspMainWnd->raise();
        m_qspMainWnd->activateWindow(); // Reactive main window
        m_qspMainWnd->resize(DEFAULT_WINDOWS_WIDTH, DEFAULT_WINDOWS_HEIGHT);

        Dtk::Widget::moveToCenter(m_qspMainWnd.get());

        DFontMgrMainWindow* pMainWnd = reinterpret_cast<DFontMgrMainWindow*>(m_qspMainWnd.get());

        Q_ASSERT(pMainWnd != nullptr);

        if (m_selectedFiles.size() > 0 && (!m_selectedFiles.at(0).isEmpty())) {
            qDebug() << "File:" << m_selectedFiles << " to quick install.";

            m_qspMainWnd->setVisible(false);
            pMainWnd->setQuickInstallMode(true);
            QMetaObject::invokeMethod(m_qspMainWnd.get(), "quickModeInstall", Qt::QueuedConnection,
                                      Q_ARG(QStringList, m_selectedFiles));
        } else {
            //If no parameter, that is normal mode.
            //Need hide quick window, which may be showed before.
            pMainWnd->setQuickInstallMode(false);
            pMainWnd->hideQucikInstallWindow();
        }
    }
}

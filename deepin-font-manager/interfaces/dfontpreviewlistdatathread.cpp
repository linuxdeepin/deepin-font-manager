#include "dfontpreviewlistdatathread.h"
#include "dfmxmlwrapper.h"
#include "dfontpreviewlistview.h"
#include "dcopyfilesmanager.h"
#include "dcomworker.h"
#include "dfmdbmanager.h"
#include "dfontinfomanager.h"

#include <QFontDatabase>
#include <QApplication>

static DFontPreviewListDataThread *INSTANCE = nullptr;

QList<DFontPreviewItemData> DFontPreviewListDataThread::m_fontModelList = QList<DFontPreviewItemData>();
//系统字体路径
const QString FONTS_DIR = QDir::homePath() + "/.local/share/fonts/";

const QString FONTS_UP_DIR = QDir::homePath() + "/.local/share/";

DFontPreviewListDataThread *DFontPreviewListDataThread::instance(DFontPreviewListView *view)
{
    if (!INSTANCE) {
        INSTANCE = new DFontPreviewListDataThread(view);
    }

    return INSTANCE;
}

DFontPreviewListDataThread *DFontPreviewListDataThread::instance()
{
    return INSTANCE;
}

/*************************************************************************
 <Function>      DFontPreviewListDataThread
 <Description>    构造函数
 <Author>        null
 <Input>
    <param1>     view            Description:父指针
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFontPreviewListDataThread::DFontPreviewListDataThread(DFontPreviewListView *view)
    : m_view(view)
    , m_fsWatcher(nullptr)
    , m_mutex(nullptr)
    , m_fontInfoManager(DFontInfoManager::instance())
    , m_fontManager(DFontManager::instance())
{
    if (view != nullptr)
        m_mutex = view->getMutex();

    m_dbManager = DFMDBManager::instance();
    moveToThread(&mThread);
    QObject::connect(&mThread, SIGNAL(started()), this, SLOT(doWork()));
    connect(this, &DFontPreviewListDataThread::requestDeleted, this, &DFontPreviewListDataThread::onFileDeleted/*, Qt::QueuedConnection*/);
    connect(this, &DFontPreviewListDataThread::requestAdded, this, &DFontPreviewListDataThread::onFileAdded/*, Qt::QueuedConnection*/);
    connect(this, &DFontPreviewListDataThread::requestForceDeleteFiles, this, &DFontPreviewListDataThread::forceDeleteFiles);
    connect(this, &DFontPreviewListDataThread::requestRemoveFileWatchers, this, &DFontPreviewListDataThread::onRemoveFileWatchers, Qt::BlockingQueuedConnection);
    connect(this, &DFontPreviewListDataThread::requestAutoDirWatchers, this, &DFontPreviewListDataThread::onAutoDirWatchers, Qt::BlockingQueuedConnection);
    connect(this, &DFontPreviewListDataThread::requestExportFont, this, &DFontPreviewListDataThread::onExportFont);

    connect(m_fontManager, &DFontManager::installFinished, this, &DFontPreviewListDataThread::onInstallFinished);

    connect(m_fontManager, &DFontManager::reInstallFinished, this, &DFontPreviewListDataThread::onReInstallFinished);

    connect(this, &DFontPreviewListDataThread::requestBatchInstall, this, &DFontPreviewListDataThread::onBatchInstall);
    connect(this, &DFontPreviewListDataThread::requestBatchReInstall, this, &DFontPreviewListDataThread::onBatchReInstall);
    connect(this, &DFontPreviewListDataThread::requestBatchReInstallContinue,
            this, &DFontPreviewListDataThread::batchReInstallContinue);
    connect(this, &DFontPreviewListDataThread::requestCancelReinstall, this, &DFontPreviewListDataThread::onCancelReinstall);

    QTimer::singleShot(3, [this] {
        mThread.start();
    });
}

DFontPreviewListDataThread::~DFontPreviewListDataThread()
{
}

/*************************************************************************
 <Function>      doWork
 <Description>   线程函数
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::doWork()
{
    QMutexLocker locker(m_mutex);
    initFileSystemWatcher();
    m_fontModelList.clear();

    qDebug() << "doWork thread id = " << QThread::currentThreadId();

    QStringList disableFontList = DFMXmlWrapper::getFontConfigDisableFontPathList();

    FontManager::getFontList(true);
    if (!m_fontModelList.isEmpty()) {
        //从fontconfig配置文件同步字体启用/禁用状态数据
        syncFontEnableDisableStatusData(disableFontList);
        refreshFontListData(true, disableFontList);

        m_view->onFinishedDataLoad();
        return;
    }

    qDebug() << "strAllFontList.size()" << m_allFontPathList.size() << endl;
    int index = 0;
    for (QString &filePath : m_allFontPathList) {
        if (filePath.length() > 0) {
            index = insertFontItemData(filePath, index, m_chineseFontPathList, m_monoSpaceFontPathList, true);
        }
    }

    for (QString &filePath : disableFontList) {
        index =  insertFontItemData(filePath, index, m_chineseFontPathList, m_monoSpaceFontPathList, true, false);
    }

    m_dbManager->commitAddFontInfo();
    m_view->onFinishedDataLoad();

    Q_EMIT m_view->multiItemsAdded(m_fontModelList, DFontSpinnerWidget::StartupLoad);
}

/*************************************************************************
 <Function>      initFileSystemWatcher
 <Description>   初始化系统文件监视器
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::initFileSystemWatcher()
{
    if (m_fsWatcher == nullptr)
        m_fsWatcher = new QFileSystemWatcher(this);
    QDir dir(FONTS_DIR);
    if (!dir.exists())
        dir.mkpath(FONTS_DIR);

    m_fsWatcher->addPath(FONTS_DIR);
    m_fsWatcher->addPath(FONTS_UP_DIR);
    connect(m_fsWatcher, &QFileSystemWatcher::fileChanged,
    this, [ = ](const QString & path) {
        qDebug() << "fileChanged" << path;
        updateChangedFile(path);
    });

    connect(m_fsWatcher, &QFileSystemWatcher::directoryChanged,
    this, [ = ](const QString & path) {
        QFileInfo f(path);
        if (!f.isDir())
            return ;
//        qDebug() << "directoryChanged" << path;
        updateChangedDir();

        if (!dir.exists()) {
            m_fsWatcher->removePath(FONTS_DIR);
        }
    });
}

/*************************************************************************
 <Function>      updateChangedFile
 <Description>   文件修改后触发函数
 <Author>        null
 <Input>
    <param1>     path            Description:路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::updateChangedFile(const QString &path)
{
    m_view->updateChangedFile(path);
}

/*************************************************************************
 <Function>      updateChangedDir
 <Description>   文件夹修改后触发函数
 <Author>        null
 <Input>
    <param1>     path            Description:路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::updateChangedDir()
{
    QMutexLocker locker(m_mutex);
    m_view->updateChangedDir();
}

/*************************************************************************
 <Function>      addPathWatcher
 <Description>   添加文件监视器
 <Author>        null
 <Input>
    <param1>     path            Description:需要添加的路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::addPathWatcher(const QString &path)
{
    if (m_fsWatcher == nullptr)
        return;

    if (!QFileInfo(path).exists())
        return;

    if (m_fsWatcher->addPath(path)) {
        if (!m_fsWatcher->directories().contains(FONTS_DIR))
            m_fsWatcher->addPath(FONTS_DIR);

        if (!m_fsWatcher->directories().contains(FONTS_UP_DIR))
            m_fsWatcher->addPath(FONTS_UP_DIR);
    }
}

/*************************************************************************
 <Function>      removePathWatcher
 <Description>   移除特定路径的文件监视器
 <Author>        null
 <Input>
    <param1>     path            Description:需要移除的路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::removePathWatcher(const QString &path)
{
    if (m_fsWatcher == nullptr)
        return;
    m_fsWatcher->removePath(path);
}

/*************************************************************************
 <Function>      onFileDeleted
 <Description>   文件删除响应函数
 <Author>        null
 <Input>
    <param1>     files            Description:删除的文件路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::onFileDeleted(QStringList &files)
{
    qDebug() << __FUNCTION__ << files.size();
    if (m_mutex != nullptr)
        QMutexLocker locker(m_mutex);
    m_view->deleteCurFonts(files, false);
}

/*************************************************************************
 <Function>      onFileAdded
 <Description>   增加文件响应函数
 <Author>        null
 <Input>
    <param1>     files            Description:增加的文件路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::onFileAdded(const QStringList &files)
{
    if (files.isEmpty()) {
        return;
    }

    if (m_mutex != nullptr)
        QMutexLocker locker(m_mutex);
    qDebug() << __func__ << "S" << QThread::currentThreadId() << endl;
    m_installFiles.clear();
    m_installedFiles.clear();
    m_installedFontsFamilyname.clear();
    m_halfInstalledFiles.clear();
    m_errorList.clear();
    refreshFontListData(false, files);
}

/*************************************************************************
 <Function>      getFontModelList
 <Description>   获取当前列表所有项的数据链表
 <Author>        null
 <Input>
    <param1>     null                         Description:null
 <Return>        QList<DFontPreviewItemData>  Description:列表所有项的数据链表
 <Note>          null
*************************************************************************/
QList<DFontPreviewItemData> DFontPreviewListDataThread::getFontModelList()
{
    if (m_view->isListDataLoadFinished())
        return m_fontModelList;
    return QList<DFontPreviewItemData>();
}

//更新itemDataList的itemData状态

/*************************************************************************
 <Function>      updateItemStatus
 <Description>   更新itemDataList的itemData状态
 <Author>        null
 <Input>
    <param1>     index               Description:需要更新的位置
    <param2>     itemData            Description:需要更新的数据
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::updateItemStatus(int index, const DFontPreviewItemData &itemData)
{
    m_fontModelList.replace(index, itemData);
}

/*************************************************************************
 <Function>      forceDeleteFiles
 <Description>   强制删除文件
 <Author>        null
 <Input>
    <param1>     files            Description:需要删除的文件路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::forceDeleteFiles(QStringList &files)
{
    qDebug() << __FUNCTION__ << files << m_mutex;
    if (m_mutex != nullptr)
        QMutexLocker locker(m_mutex);
    m_view->deleteFontFiles(files, true);

    Q_EMIT requestBatchReInstallContinue();
    qDebug() << __FUNCTION__ << files << " end ";
}

/*************************************************************************
 <Function>      onRemoveFileWatchers
 <Description>   批量移除文件监视器
 <Author>        null
 <Input>
    <param1>     files            Description:需要移除的路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::onRemoveFileWatchers(const QStringList &files)
{
    qDebug() << __FUNCTION__ << files.size() << m_fsWatcher->removePaths(files);
    Q_UNUSED(files)
    m_fsWatcher->removePath(FONTS_DIR);
    m_fsWatcher->removePath(FONTS_UP_DIR);
}

/*************************************************************************
 <Function>      onAutoDirWatchers
 <Description>   添加特定文件夹的文件监视器
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::onAutoDirWatchers()
{
    m_fsWatcher->addPath(FONTS_DIR);
    m_fsWatcher->addPath(FONTS_UP_DIR);
}

/**
* @brief DFontPreviewListDataThread::onExportFont 导出文件列表槽函数
* @param fontList 要导出的文件列表
* @return void
*/
void DFontPreviewListDataThread::onExportFont(QStringList &fontList)
{
    qDebug() << __FUNCTION__ << fontList.size();

    QDir dir(FONTS_DESKTOP_DIR);
    if (!dir.exists())
        dir.mkpath(FONTS_DESKTOP_DIR);

    DCopyFilesManager::instance()->copyFiles(CopyFontThread::EXPORT, fontList);
    Q_EMIT exportFontFinished(fontList.size());
}

/**
* @brief DFontPreviewListDataThread::onInstallFinished 字体安装后的槽函数
* @param state 完成状态
* @param fileList 安装字体文件列表
* @return void
*/
void DFontPreviewListDataThread::onInstallFinished(int state, const QStringList &fileList)
{
    // ToDo:
    //   May send signal to mainwindow refresh new installed font
    // QMIT notfiyRefresh;
    if (0 == state || 1 == state) {
        m_installFiles.clear();
        m_installState = DFontManager::Install;

        // Update the installtion file list showed in exception dialog
        m_installFiles << m_installedFiles;

        m_installFiles << m_damagedFiles;

        updateInstalledFontList(fileList);
    }

    Q_EMIT installFinished();

    if (ifNeedShowExceptionWindow(false)) {
        qDebug() << __FUNCTION__ << "need reinstall";
    } else {
        qDebug() << __FUNCTION__ << " no need reinstall";
        onFileAdded(m_outfileList);
    }
}

/**
* @brief DFontPreviewListDataThread::onReInstallFinished 字体重新安装后的槽函数
* @param state 完成状态
* @param fileList 重装字体文件列表
* @return void
*/
void DFontPreviewListDataThread::onReInstallFinished(int state, const QStringList &fileList)
{
    qDebug() << __FUNCTION__  << fileList.size();
    if (0 == state) {
        m_installFiles.clear();
        m_installState = DFontManager::ReInstall;
    }

    updateInstalledFontList(fileList);

    Q_EMIT reInstallFinished();

    onFileAdded(m_outfileList);

    qDebug() << __FUNCTION__ << m_outfileList.size();
}

/**
* @brief DFontPreviewListDataThread::verifyFontFiles 字体文件过滤器，过滤后得到需要新安装的字体，重复安装字体，损毁字体，系统字体
* @param void
* @return void
*/
void DFontPreviewListDataThread::verifyFontFiles()
{
    QStringList fontInfos;
    QStringList instFontInfos;

    m_damagedFiles.clear();
    m_installedFiles.clear();
    m_newInstallFiles.clear();
    m_systemFiles.clear();
    m_errorList.clear();
    m_newHalfInstalledFiles.clear();
    m_oldHalfInstalledFiles.clear();

    for (auto &it : m_installFiles) {
        DFontInfo fontInfo = m_fontInfoManager->getFontInfo(it);
        if (Q_UNLIKELY(fontInfo.isError)) {
            m_damagedFiles.append(it);

#ifdef QT_QML_DEBUG
//            qDebug() << __FUNCTION__ << " (" << it << " :Damaged file)";
#endif
        } else if (fontInfo.isInstalled && !isSystemFont(fontInfo)) {
            QString familyStyleName = getFamilyStyleName(fontInfo);
            if (!instFontInfos.contains(familyStyleName)) {
                instFontInfos.append(familyStyleName);
                m_installedFiles.append(it);
            }

#ifdef QT_QML_DEBUG
//            qDebug() << __FUNCTION__ << " (" << it << " :Installed file)";
#endif
        } else if (isSystemFont(fontInfo)) {
            m_systemFiles.append(it);

#ifdef QT_QML_DEBUG
//            qDebug() << __FUNCTION__ << " (" << it << " :System file)";
#endif
        } else {
            QString familyStyleName = getFamilyStyleName(fontInfo);
            if (!fontInfos.contains(familyStyleName)) {
                fontInfos.append(familyStyleName);
                /*如果是字体验证框弹出时再进行安装的话,因为这一步骤安装的字体没有插入数据库,所以需要再判断下是否为安装过的.*/
                if (m_installedFontsFamilyname.contains(familyStyleName)) {
                    /*这里获取需要新添加到验证框中的字体m_newHalfInstalledFiles和之前出现过的字体m_oldHalfInstalledFiles
                    ,用于之后listview滚动和设置选中状态使用*/
                    if (!m_halfInstalledFiles.contains(it)) {
                        m_newHalfInstalledFiles.append(it);
                    } else {
                        m_oldHalfInstalledFiles.append(it);
                    }
                } else {
                    m_newInstallFiles.append(it);
                }
            }

#ifdef QT_QML_DEBUG
//            qDebug() << __FUNCTION__ << " (" << it << " :New file)";
#endif
        }
    }
    m_errorList = m_damagedFiles + m_installedFiles + m_systemFiles;
    m_damagedFiles.clear();
    m_installFiles.clear();
    m_systemFiles.clear();
}

/**
* @brief DFontPreviewListDataThread::ifNeedShowExceptionWindow 检测是否要弹出字体验证框，存在重复安装字体，系统字体时，损坏字体时弹出字体验证框
* @param void
* @return void
*/
bool DFontPreviewListDataThread::ifNeedShowExceptionWindow(bool needSkipException)
{
    // Skip Exception dialog
    if (needSkipException) {
        return false;
    }

    if (m_isReinstall)
        return true;
    // If have new install file,install first,then check the exception list
    if (DFontManager::Install == m_installState && m_newInstallFiles.size() > 0) {
        return false;
    }

    // For all selected files is installed & damage
    if (((DFontManager::Install == m_installState)
            && (m_errorList.size() > 0))
            || ((DFontManager::ReInstall == m_installState) && (m_errorList.size() > 0))) {
        m_isReinstall = true;
        return true;
    }

    return false;
}

/**
* @brief DFontPreviewListDataThread::updateInstalledFontList 更新安装成功的字体列表
* @param filesList 字体文件列表
* @return void
*/
void DFontPreviewListDataThread::updateInstalledFontList(const QStringList &filesList)
{
    if (filesList.isEmpty())
        return;

    for (const QString &filePath : filesList) {
        DFontInfo fontInfo = m_fontInfoManager->getFontInfo(filePath);
        QString familyStyleName = getFamilyStyleName(fontInfo);

        if (!m_installedFontsFamilyname.contains(familyStyleName)) {
            m_installedFontsFamilyname.append(familyStyleName);
            m_outfileList.append(fontInfo.filePath);
        }
    }
    m_installFontCnt = m_outfileList.size();
}

/**
* @brief DFontPreviewListDataThread::isSystemFont 判断当前字体是否为系统字体
* @param f 传入当前字体
* @return bool true 表示当前字体为系统字体；false 表示当前字体不是系统字体
*/
bool DFontPreviewListDataThread::isSystemFont(const DFontInfo &f)
{
    QString fontFullname = QString("%1%2").arg(f.familyName).arg(f.styleName);
    return (m_SystemFontsList.contains(fontFullname));
}

/**
* @brief DFontPreviewListDataThread::batchInstall 批量安装处理函数
* @param void
* @return void
*/
void DFontPreviewListDataThread::batchInstall()
{
    // Check&Sort uninstalled ,installed & damaged font file here
    if (m_newInstallFiles.isEmpty()) {
        qDebug() << __FUNCTION__ << " no new fonts";
        Q_EMIT installFinished();
        return;
    }

    QStringList installListWithFamliyName;
    for (const QString &filePath : m_newInstallFiles) {
        DFontInfo fontInfo = m_fontInfoManager->getFontInfo(filePath);
        QString familyName = getFamilyName(fontInfo);
        installListWithFamliyName.append(filePath + "|" + familyName);

//        qDebug() << " Prepare install file: " << it + "|" + familyName;
    }

//    qDebug() << installListWithFamliyName << endl;
    m_newInstallFiles.clear();

    m_fontManager->setType(DFontManager::Install);
    m_fontManager->setInstallFileList(installListWithFamliyName);
    m_fontManager->start();
}

/**
* @brief DFontPreviewListDataThread::onBatchReInstall 安装重复字体槽函数
* @param void
* @return void
*/
void DFontPreviewListDataThread::onBatchReInstall(const QStringList &fontList)
{
    qDebug() << __FUNCTION__ << " need reinstall " << fontList.size();
    if (fontList.isEmpty())
        return;

    // Reinstall the user selected files
    m_installState = DFontManager::ReInstall;
    m_installFiles = fontList;

#ifdef QT_QML_DEBUG
//        qDebug() << __FUNCTION__ << " [reinstallFiles=" << m_installFiles << "]";
#endif

    QStringList filesInstalled;
    QStringList installListWithFamliyName;
    for (auto &it : fontList) {
        //delete the font file first
        DFontInfo fi = m_fontInfoManager->getFontInfo(it);
        QString filePath = DFMDBManager::instance()->isFontInfoExist(fi);
        QString familyName = getFamilyName(fi);
        installListWithFamliyName.append(it + "|" + familyName);
//        qDebug() << " Prepare install file: " << it + m_loadingSpinner"|" + familyName;
        if (QFileInfo(filePath).fileName() == QFileInfo(it).fileName()) {
//                    qDebug() << __FUNCTION__ << "same file " << it << " will be overrided ";
            continue;
        }
        filesInstalled << filePath;
    }
    //force delete fonts installed
    if (!filesInstalled.empty()) {
        Q_EMIT requestForceDeleteFiles(filesInstalled);
        return;
    }

    m_installedFiles.clear();

    m_fontManager->setType(DFontManager::ReInstall);
    m_fontManager->setInstallFileList(installListWithFamliyName);
    m_fontManager->start();
}

/**
* @brief DFontPreviewListDataThread::onBatchReInstall 安装重复字体
* @param void
* @return void
*/
void DFontPreviewListDataThread::onCancelReinstall()
{
    qDebug() << __FUNCTION__ << QThread::currentThreadId();
    onFileAdded(m_outfileList);
}

/**
* @brief DFontPreviewListDataThread::onBatchInstall 安装字体槽函数
* @param fontList 待安装字体列表
* @return void
*/
void DFontPreviewListDataThread::onBatchInstall(const QStringList &fontList)
{
    qDebug() << __FUNCTION__ << fontList.size();
    m_installState = DFontManager::Install;
    m_installFiles.clear();
    m_installedFontsFamilyname.clear();
    m_halfInstalledFiles.clear();
    m_outfileList.clear();
    m_installFiles = fontList;
    m_installFontCnt = 0;
    m_isReinstall = false;
    getAllSysFonts();
    // Check installed & damaged font file here
    verifyFontFiles();

    // Install the font list ,which may be changed in exception window
    batchInstall();
}

/**
* @brief DFontPreviewListDataThread::batchHalfwayInstall 字体验证框弹出时在文件管理器进行安装
* @param filelist 并行安装新增文件列表
* @return void
*/
void DFontPreviewListDataThread::batchHalfwayInstall(const QStringList &filelist)
{
    m_installFiles = filelist;
    verifyFontFiles();

    qDebug() << m_newHalfInstalledFiles.count() << "*" << m_oldHalfInstalledFiles.count() << endl;
    m_halfInstalledFiles.append(m_newHalfInstalledFiles);

    //当安装的字体是需要新添加到字体验证框时或者已经添加到字体验证框时,刷新listview.
    if (m_errorList.count() + m_newHalfInstalledFiles.count() + m_oldHalfInstalledFiles.count() > 0) {
        Q_EMIT SignalManager::instance()->updateInstallErrorListview(m_errorList, m_halfInstalledFiles, m_newHalfInstalledFiles, m_oldHalfInstalledFiles);
    }

    QStringList installListWithFamliyName;
    for (auto &it : m_newInstallFiles) {
        DFontInfo fontInfo = m_fontInfoManager->getFontInfo(it);
        QString familyName = getFamilyName(fontInfo);
        installListWithFamliyName.append(it + "|" + familyName);
//        qDebug() << " Prepare install file: " << it + "|" + familyName;
    }

    m_fontManager->setType(DFontManager::HalfwayInstall);
    m_fontManager->setInstallFileList(installListWithFamliyName);
    m_fontManager->start();
}

/**
* @brief DFontPreviewListDataThread::batchReInstallContinue 重装验证页面，继续按钮处理函数-继续批量安装
* @param void
* @return void
*/
void DFontPreviewListDataThread::batchReInstallContinue()
{
    if (m_installState == DFontManager::ReInstall) {
        m_installedFiles.clear();
    }

    QStringList installListWithFamliyName;
    for (auto &it : m_installFiles) {
        DFontInfo fontInfo = m_fontInfoManager->getFontInfo(it);
        QString familyName = getFamilyName(fontInfo);
        installListWithFamliyName.append(it + "|" + familyName);

//        qDebug() << " Prepare install file: " << it + "|" + familyName;
    }

    m_fontManager->setType(DFontManager::ReInstall);
    m_fontManager->setInstallFileList(installListWithFamliyName);
    m_fontManager->start();
}

/*************************************************************************
 <Function>      insertFontItemData
 <Description>   将需要添加项的字体数据收集放人list中.
 <Author>        null
 <Input>
    <param1>     filepath            Description:文件路径
    <param2>     index               Description:null添加的位置
    <param3>     chineseFontPathList           Description:中文字体列表
    <param4>     monoSpaceFontPathList           Description:等宽字体列表
    <param5>     isStartup           Description:是否为第一次启动
    <param6>     isEnabled           Description:是否可以换启用

 <Return>        int                 Description:null
 <Note>          null
*************************************************************************/
int DFontPreviewListDataThread::insertFontItemData(const QString &filePath,
                                                   int index,
                                                   const QStringList &chineseFontPathList,
                                                   const QStringList &monoSpaceFontPathList,
                                                   bool isStartup, bool isEnabled)
{
    DFontInfoManager *fontInfoMgr = DFontInfoManager::instance();
    DFontPreviewItemData itemData;
    itemData.fontInfo = fontInfoMgr->getFontInfo(filePath, true);

    QString familyName;
    if (itemData.fontInfo.sp3FamilyName.isEmpty() || itemData.fontInfo.sp3FamilyName.contains(QChar('?'))) {
        int appFontId = QFontDatabase::addApplicationFont(itemData.fontInfo.filePath);

        itemData.appFontId = appFontId;
        QStringList fontFamilyList = QFontDatabase::applicationFontFamilies(appFontId);
        for (QString &family : fontFamilyList) {
            if (family.contains(QChar('?')))
                continue;
            familyName = family;
        }
        if (familyName.isEmpty()) {
            familyName = itemData.fontInfo.fullname;
        } else {
            itemData.fontInfo.sp3FamilyName = familyName;
        }
    } else if (itemData.fontInfo.isSystemFont) {
        familyName = itemData.fontInfo.sp3FamilyName;
    } else {
        familyName = itemData.fontInfo.fullname;
    }

    if (itemData.fontInfo.styleName.length() > 0 && !familyName.endsWith(itemData.fontInfo.styleName)) {
        itemData.fontData.strFontName =
            QString("%1-%2").arg(familyName).arg(itemData.fontInfo.styleName);
    } else {
        itemData.fontData.strFontName = familyName;
    }

    itemData.strFontId = QString::number(index);
    itemData.fontData.setEnabled(isEnabled);
    itemData.fontData.setCollected(false);
    itemData.fontData.setChinese(chineseFontPathList.contains(filePath) && (itemData.fontInfo.previewLang & FONT_LANG_CHINESE));
    itemData.fontData.setMonoSpace(monoSpaceFontPathList.contains(filePath));
    itemData.fontData.setFontType(itemData.fontInfo.type);
    itemData.fontData.isSystemFont = m_dbManager->isSystemFont(filePath);

    itemData.fontInfo.isInstalled = true;

    //中文字体
    if (itemData.fontInfo.isSystemFont && itemData.fontData.isChinese()) {
        /* Bug#16821 UT000591  添加字体后需要加入到Qt的字体数据库中，否则无法使用*/
//        qDebug() << "addApplicationFont s"  << endl;
        int appFontId = QFontDatabase::addApplicationFont(itemData.fontInfo.filePath);
//        qDebug() << "addApplicationFont e"  << appFontId << itemData.fontData.strFontName << endl;
        itemData.appFontId = appFontId;

        QStringList fontFamilyList = QFontDatabase::applicationFontFamilies(appFontId);
        if (fontFamilyList.size() > 1) {
            ++index;
            for (QString &fontFamily : fontFamilyList) {
                itemData.strFontId = QString::number(index);
                itemData.fontInfo.familyName = fontFamily;
                itemData.fontInfo.sp3FamilyName = fontFamily;
                if (itemData.fontInfo.styleName.length() > 0) {
                    itemData.fontData.strFontName =
                        QString("%1-%2").arg(itemData.fontInfo.familyName).arg(itemData.fontInfo.styleName);
                } else {
                    itemData.fontData.strFontName = itemData.fontInfo.familyName;
                }
                m_dbManager->addFontInfo(itemData);
                m_fontModelList.append(itemData);
            }
        } else {
            m_dbManager->addFontInfo(itemData);
            m_fontModelList.append(itemData);
        }
    } else {
        m_dbManager->addFontInfo(itemData);
        m_fontModelList.append(itemData);
    }

//    Q_EMIT m_view->itemAdded(itemData);
    addPathWatcher(filePath);

    if (!isStartup) {
        m_diffFontModelList.append(itemData);
    }
    return (index + 1);
}

/*************************************************************************
 <Function>      refreshFontListData
 <Description>    刷新字体列表的数据
 <Author>        null
 <Input>
    <param1>     isStartup              Description:是否为第一次启动
    <param2>     installFont            Description:安装的字体文件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::refreshFontListData(bool isStartup, const QStringList &installFont)
{
    qDebug() << __FUNCTION__ << " begin" << isStartup << installFont.size() << QThread::currentThreadId() << m_fontModelList.size();

    if (isStartup) {
        for (const QString &filePath : installFont) {
            if (!m_allFontPathList.contains(filePath))
                m_allFontPathList << filePath;
        }
    } else {
        FontManager::getFontListInSequence(isStartup);
    }

    QSet<QString> dbFilePathSet;
    for (DFontPreviewItemData &itemData : m_fontModelList) {
        QString filePath = itemData.fontInfo.filePath.trimmed();

        dbFilePathSet.insert(filePath);
        if (isStartup)
            addPathWatcher(filePath);
    }

    if (isStartup) {
        for (DFontPreviewItemData &itemData : m_delFontInfoList) {
            //如果字体文件已经不存在，则从t_manager表中删除
            //删除字体之前启用字体，防止下次重新安装后就被禁用
            m_view->enableFont(itemData.fontInfo.filePath);
            DFMDBManager::instance()->deleteFontInfo(itemData);
        }
    }

    DFMDBManager::instance()->commitDeleteFontInfo();
    m_view->enableFonts();

    m_delFontInfoList.clear();
    m_diffFontModelList.clear();

    //根据文件路径比较出不同的字体文件
    QSet<QString> allFontListSet = m_allFontPathList.toSet();
    QSet<QString> diffSet = allFontListSet.subtract(dbFilePathSet);
    qDebug() << "diffSet count:" << diffSet.count();
    if (diffSet.count() > 0) {
        int maxFontId = m_dbManager->getCurrMaxFontId();
        QList<QString> diffFilePathList = diffSet.toList();
        int index = maxFontId + 1;
        for (QString &filePath : diffFilePathList) {
            if (m_dbManager->isSystemFont(filePath) || installFont.contains(filePath)) {
                bool isEnabled = (isStartup && installFont.contains(filePath)) ? false : true;
                index = insertFontItemData(filePath, index, m_chineseFontPathList, m_monoSpaceFontPathList, isStartup, isEnabled);
            }
        }
        m_dbManager->commitAddFontInfo();
    }
    qDebug() << __FUNCTION__ << " end " << QThread::currentThreadId() << m_diffFontModelList.size() << m_fontModelList.size();

    if (!isStartup && !installFont.isEmpty()) {
        Q_EMIT m_view->multiItemsAdded(m_diffFontModelList, DFontSpinnerWidget::NoLabel);
        Q_EMIT m_view->itemsSelected(installFont);
    } else if (isStartup) {
        Q_EMIT m_view->multiItemsAdded(m_fontModelList, DFontSpinnerWidget::StartupLoad);
    } else {
        Q_EMIT m_view->multiItemsAdded(m_fontModelList, DFontSpinnerWidget::Load);
    }
}

/*************************************************************************
 <Function>      syncFontEnableDisableStatusData
 <Description>   更新从配置文件中读到的禁用字体的信息
 <Author>        null
 <Input>
    <param1>     null            Description:null
    <param2>     null            Description:null
    <param3>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::syncFontEnableDisableStatusData(const QStringList &disableFontPathList)
{
    //disableFontPathList为被禁用的字体路径列表
//    if (disableFontPathList.size() == 0) {
//        return;
//    }

    QMap<QString, bool> disableFontMap;
    for (const QString &disableFontPath : disableFontPathList) {
        disableFontMap.insert(disableFontPath, true);
    }

    for (DFontPreviewItemData &fontItemData : m_fontModelList) {
        QString keyFilePath = fontItemData.fontInfo.filePath;

        if (fontItemData.fontData.isEnabled() != disableFontMap.value(keyFilePath))
            continue;

        //disableFontMap为被禁用的字体map
        if (disableFontMap.value(keyFilePath)) {
            fontItemData.fontData.setEnabled(false);
        } else {
            fontItemData.fontData.setEnabled(true);
        }

        m_dbManager->updateFontInfo(fontItemData, "isEnabled");
    }

    m_dbManager->commitUpdateFontInfo();
}


/*************************************************************************
 <Function>      updateFontId
 <Description>   更新字体信息中的fontid
 <Author>        null
 <Input>
    <param1>     null            Description:null
    <param2>     null            Description:null
    <param3>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListDataThread::updateFontId(const DFontPreviewItemData &itemData, int id)
{
    if (id < 0)
        return;
    int index = m_fontModelList.indexOf(itemData);
    if (index > -1) {
        m_fontModelList[index].appFontId = id;
        if (m_fontModelList[index].fontInfo.sp3FamilyName != itemData.fontInfo.sp3FamilyName)
            m_fontModelList[index].fontInfo.sp3FamilyName = itemData.fontInfo.sp3FamilyName;
    }
}

/**
* @brief DFontPreviewListDataThread::updateSysFonts 更新系统字体列表
* @param void
* @return void
*/
void DFontPreviewListDataThread::getAllSysFonts()
{
    m_SystemFontsList.clear();
    for (const DFontPreviewItemData &font : m_fontModelList) {
        if (!font.fontInfo.isSystemFont)
            continue;

        QString systemFileName = QString("%1%2").arg(font.fontInfo.familyName).arg(font.fontInfo.styleName);
        m_SystemFontsList.append(systemFileName);
    }
}

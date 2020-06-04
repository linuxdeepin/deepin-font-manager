#include "dfontpreviewlistdatathread.h"
#include "dfmxmlwrapper.h"
#include "dfontpreviewlistview.h"

#include <QFontDatabase>

static DFontPreviewListDataThread *INSTANCE = nullptr;
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

DFontPreviewListDataThread::DFontPreviewListDataThread(DFontPreviewListView *view)
    : m_view(view)
    , m_fsWatcher(nullptr)
    , m_mutex(nullptr)
{
    if (view != nullptr)
        m_mutex = view->getMutex();

//    QTimer::singleShot(50, this, [this]() {
    m_dbManager = DFMDBManager::instance();
    moveToThread(&mThread);
    QObject::connect(&mThread, SIGNAL(started()), this, SLOT(doWork()));
    connect(m_view, &DFontPreviewListView::requestDeleted, this, &DFontPreviewListDataThread::onFileDeleted, Qt::QueuedConnection);
    connect(m_view, &DFontPreviewListView::requestAdded, this, &DFontPreviewListDataThread::onFileAdded/*, Qt::QueuedConnection*/);
    connect(this, &DFontPreviewListDataThread::requestForceDeleteFiles, this, &DFontPreviewListDataThread::forceDeleteFiles);
    connect(this, &DFontPreviewListDataThread::requestRemoveFileWatchers, this, &DFontPreviewListDataThread::onRemoveFileWatchers, Qt::BlockingQueuedConnection);
    mThread.start();
//    });
}

DFontPreviewListDataThread::~DFontPreviewListDataThread()
{
}

void DFontPreviewListDataThread::doWork()
{
    QMutexLocker locker(m_mutex);
    initFileSystemWatcher();
    m_fontModelList.clear();

    qDebug() << "doWork thread id = " << QThread::currentThreadId();
    DFontInfoManager *fontInfoMgr = DFontInfoManager::instance();

    //Call refreshList out side DFontInfoManager constructor
    //instead of in constructor
    fontInfoMgr->refreshList();

    QStringList disableFontList = DFMXmlWrapper::getFontConfigDisableFontPathList();

    int recordCount = m_dbManager->getRecordCount();
    if (recordCount > 0) {

        //从fontconfig配置文件同步字体启用/禁用状态数据
        syncFontEnableDisableStatusData(disableFontList);
        qDebug() << __func__ << "S" << endl;
        refreshFontListData(true, disableFontList);

        m_view->onFinishedDataLoad();
        return;
    }

    QStringList chineseFontPathList = fontInfoMgr->getAllChineseFontPath();
    QStringList monoSpaceFontPathList = fontInfoMgr->getAllMonoSpaceFontPath();
    QStringList strAllFontList = fontInfoMgr->getAllFontPath();
    qDebug() << "strAllFontList.size()" << strAllFontList.size() << endl;
    int index = 0;
    for (QString &filePath : strAllFontList) {
        if (filePath.length() > 0) {
            index = insertFontItemData(filePath, index, chineseFontPathList, monoSpaceFontPathList, true);
        }
    }

    for (QString &filePath : disableFontList) {
        index =  insertFontItemData(filePath, index, chineseFontPathList, monoSpaceFontPathList, true, false);
    }

    Q_EMIT m_view->multiItemsAdded(m_fontModelList);

    m_dbManager->commitAddFontInfo();

    m_view->onFinishedDataLoad();
}

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
//        qDebug() << "directoryChanged" << path;
        updateChangedDir(path);

        if (!dir.exists()) {
            m_fsWatcher->removePath(FONTS_DIR);
        }
    });
}

void DFontPreviewListDataThread::updateChangedFile(const QString &path)
{
    m_view->updateChangedFile(path);
}

void DFontPreviewListDataThread::updateChangedDir(const QString &path)
{
    m_view->updateChangedDir(path);
}

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

void DFontPreviewListDataThread::removePathWatcher(const QString &path)
{
    if (m_fsWatcher == nullptr)
        return;
    m_fsWatcher->removePath(path);
}

void DFontPreviewListDataThread::onFileDeleted(const QStringList &files)
{
    qDebug() << __FUNCTION__ << files.size() << m_mutex;
    if (m_mutex != nullptr)
        QMutexLocker locker(m_mutex);
    m_view->deleteCurFonts(files);
//    qDebug() << __FUNCTION__ << files.size() << " end ";
}

void DFontPreviewListDataThread::onFileAdded(const QStringList &files)
{
    if (files.isEmpty()) {
//        if (isFirstInstall) {
//            Q_EMIT SignalManager::instance()->showInstallErrorDialog();
//        }
        return;
    }


    if (m_mutex != nullptr)
        QMutexLocker locker(m_mutex);
    qDebug() << __func__ << "S" << endl;
    refreshFontListData(false, files);
}

QList<DFontPreviewItemData> DFontPreviewListDataThread::getFontModelList()
{
    if (m_view->isListDataLoadFinished())
        return m_fontModelList;
    return QList<DFontPreviewItemData>();
}

QList<DFontPreviewItemData> DFontPreviewListDataThread::getDiffFontModelList() const
{
    return m_diffFontModelList;
}

void DFontPreviewListDataThread::setMutex(QMutex *mutex)
{
    m_mutex = mutex;
}

void DFontPreviewListDataThread::forceDeleteFiles(const QStringList &files)
{
    qDebug() << __FUNCTION__ << files << m_mutex;
    if (m_mutex != nullptr)
        QMutexLocker locker(m_mutex);
    m_view->deleteFontFiles(files, true);

    Q_EMIT requestBatchReInstallContinue();
    qDebug() << __FUNCTION__ << files << " end ";
}

void DFontPreviewListDataThread::onRemoveFileWatchers(const QStringList &files)
{
    m_fsWatcher->removePaths(files);
}

int DFontPreviewListDataThread::insertFontItemData(const QString &filePath,
                                                   int index,
                                                   const QStringList &chineseFontPathList,
                                                   const QStringList &monoSpaceFontPathList,
                                                   bool isStartup, bool isEnabled)
{
    DFontInfoManager *fontInfoMgr = DFontInfoManager::instance();
    DFontPreviewItemData itemData;
    QFileInfo filePathInfo(filePath);
    if (isStartup) {
        itemData.fontInfo = fontInfoMgr->getFontInfo(filePath, true);
    } else {
        itemData.fontInfo = fontInfoMgr->getFontInfo(filePath);
    }

    checkStyleName(itemData.fontInfo);


    if (itemData.fontInfo.styleName.length() > 0) {
        itemData.strFontName =
            QString("%1-%2").arg(itemData.fontInfo.familyName).arg(itemData.fontInfo.styleName);
    } else {
        itemData.strFontName = itemData.fontInfo.familyName;
    }

    itemData.strFontId = QString::number(index);
    itemData.strFontFileName = filePathInfo.baseName();
//    itemData.strFontPreview = m_view->getPreviewTextWithSize(&itemData.iFontSize);
    itemData.isEnabled = isEnabled;
    itemData.isPreviewEnabled = true;
    itemData.isCollected = false;
    itemData.isChineseFont = chineseFontPathList.contains(filePath);
    itemData.isMonoSpace = monoSpaceFontPathList.contains(filePath);

    itemData.fontInfo.isInstalled = true;

    //中文字体
    if (itemData.fontInfo.isSystemFont && itemData.isChineseFont) {
        /* Bug#16821 UT000591  添加字体后需要加入到Qt的字体数据库中，否则无法使用*/
        qDebug() << "addApplicationFont s"  << endl;
        int appFontId = QFontDatabase::addApplicationFont(itemData.fontInfo.filePath);
        qDebug() << "addApplicationFont e"  << appFontId << itemData.strFontName << endl;

        QStringList fontFamilyList = QFontDatabase::applicationFontFamilies(appFontId);
        if (fontFamilyList.size() > 1) {
            ++index;
            for (QString &fontFamily : fontFamilyList) {
                itemData.strFontId = QString::number(index);
                itemData.fontInfo.familyName = fontFamily;
                if (itemData.fontInfo.styleName.length() > 0) {
                    itemData.strFontName =
                        QString("%1-%2").arg(itemData.fontInfo.familyName).arg(itemData.fontInfo.styleName);
                } else {
                    itemData.strFontName = itemData.fontInfo.familyName;
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

void DFontPreviewListDataThread::refreshFontListData(bool isStartup, const QStringList &installFont)
{
    qDebug() << __FUNCTION__ << " begin";
    DFontInfoManager *fontInfoMgr = DFontInfoManager::instance();
    QStringList strAllFontList = fontInfoMgr->getAllFontPath();

    QList<DFontPreviewItemData> fontInfoList;
    if (isStartup) {
        fontInfoList = m_dbManager->getAllFontInfo();
        for (const QString &filePath : installFont) {
            if (!strAllFontList.contains(filePath))
                strAllFontList << filePath;
        }
        qDebug() << installFont;

    } else {
        fontInfoList = m_fontModelList;
    }
    QStringList chineseFontPathList = fontInfoMgr->getAllChineseFontPath();
    QStringList monoSpaceFontPathList = fontInfoMgr->getAllMonoSpaceFontPath();

    QSet<QString> dbFilePathSet;
    for (DFontPreviewItemData &itemData : fontInfoList) {
        if (itemData.fontInfo.isSystemFont) {
            if (itemData.fontInfo.familyName.startsWith("CESI") || itemData.fontInfo.familyName.contains("Mono")) {
                itemData.isCanDisable = false;
            }
        }
        if (isStartup) {
            QString filePath = itemData.fontInfo.filePath.trimmed();
            QFileInfo filePathInfo(filePath);

            if (filePathInfo.exists()) {
                m_fontModelList.append(itemData);
                dbFilePathSet.insert(filePath);
                addPathWatcher(filePath);
            } else {
                //如果字体文件已经不存在，则从t_manager表中删除
                if (!filePathInfo.exists()) {
                    //删除字体之前启用字体，防止下次重新安装后就被禁用
                    m_view->enableFont(itemData.fontInfo.filePath);
                    DFMDBManager::instance()->deleteFontInfo(itemData);
                }
            }
        } else {
            QString filePath = itemData.fontInfo.filePath;
            dbFilePathSet.insert(filePath);
            addPathWatcher(filePath);
        }
    }
    DFMDBManager::instance()->commitDeleteFontInfo();
    m_view->enableFonts();

    m_diffFontModelList.clear();

    //根据文件路径比较出不同的字体文件
    QSet<QString> allFontListSet = strAllFontList.toSet();
    QSet<QString> diffSet = allFontListSet.subtract(dbFilePathSet);
    qDebug() << "diffSet count:" << diffSet.count();
    if (diffSet.count() > 0) {
        int maxFontId = m_dbManager->getCurrMaxFontId();
        QList<QString> diffFilePathList = diffSet.toList();
        int index = maxFontId + 1;
        qDebug() << "insertFontItemData" << "s" << endl;
        foreach (QString filePath, diffFilePathList) {
            if (m_dbManager->isSystemFont(filePath) || installFont.contains(filePath)) {
                bool isEnabled = (isStartup && installFont.contains(filePath)) ? false : true;
                index = insertFontItemData(filePath, index, chineseFontPathList, monoSpaceFontPathList, isStartup, isEnabled);
            }
        }
        qDebug() << "insertFontItemData" << "s" << endl;
        m_dbManager->commitAddFontInfo();
    }
    if (!isStartup && !installFont.isEmpty()) {
        Q_EMIT m_view->multiItemsAdded(m_diffFontModelList);
        Q_EMIT m_view->itemsSelected(installFont);
    } else {
        Q_EMIT m_view->multiItemsAdded(m_fontModelList);
    }
}

void DFontPreviewListDataThread::removeFontData(const DFontPreviewItemData &removeItemData)
{
    m_diffFontModelList.clear();

    for (DFontPreviewItemData &itemData : m_fontModelList) {
        if (itemData.fontInfo.filePath == removeItemData.fontInfo.filePath) {
            m_fontModelList.removeOne(itemData);
            return;
        }
    }
}

void DFontPreviewListDataThread::checkStyleName(DFontInfo &f)
{
    QStringList str;
    str << "Regular" << "Bold" << "Light" << "Thin" << "ExtraLight" << "ExtraBold" << "Medium" << "DemiBold" << "Black";
//有些字体文件因为不规范导致的stylename为空，通过psname来判断该字体的stylename。这种情况下Psname也为空的情况极其罕见所以没有进行处理
    if (!str.contains(f.styleName)) {
        if (f.psname != "") {
            if (f.psname.contains("Regular")) {
                f.styleName = "Regular";
                return;
            } else if (f.psname.contains("Bold")) {
                f.styleName = "Bold";
                return;
            } else if (f.psname.contains("Light")) {
                f.styleName = "Light";
                return;
            } else if (f.psname.contains("Thin")) {
                f.styleName = "Thin";
                return;
            } else if (f.psname.contains("Thin")) {
                f.styleName = "Thin";
                return;
            } else if (f.psname.contains("ExtraLight")) {
                f.styleName = "ExtraLight";
                return;
            } else if (f.psname.contains("ExtraBold")) {
                f.styleName = "ExtraBold";
                return;
            } else if (f.psname.contains("Medium")) {
                f.styleName = "Medium";
                return;
            } else if (f.psname.contains("DemiBold")) {
                f.styleName = "DemiBold";
                return;
            } else if (f.psname.contains("Black")) {
                f.styleName = "Black";
                return;
            }
        }
    }
}

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

    QList<DFontPreviewItemData> fontInfoList = m_dbManager->getAllFontInfo();

    for (DFontPreviewItemData &fontItemData : fontInfoList) {
        QString keyFilePath = fontItemData.fontInfo.filePath;

        if (fontItemData.isEnabled != disableFontMap.value(keyFilePath))
            continue;

        //disableFontMap为被禁用的字体map
        if (disableFontMap.value(keyFilePath)) {
            fontItemData.isEnabled = false;
            fontItemData.isPreviewEnabled = false;
        } else {
            fontItemData.isEnabled = true;
            fontItemData.isPreviewEnabled = true;
        }

        m_dbManager->updateFontInfo(fontItemData, "isEnabled");
    }

    m_dbManager->commitUpdateFontInfo();
}

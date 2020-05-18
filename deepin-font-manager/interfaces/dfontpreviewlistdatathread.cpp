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
    connect(m_view, &DFontPreviewListView::requestAdded, this, &DFontPreviewListDataThread::onFileAdded, Qt::QueuedConnection);
    connect(this, &DFontPreviewListDataThread::requestForceDeleteFiles, this, &DFontPreviewListDataThread::forceDeleteFiles);
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

        refreshFontListData(true, false, disableFontList);

        m_view->onFinishedDataLoad();
        return;
    }

    QStringList chineseFontPathList = fontInfoMgr->getAllChineseFontPath();
    QStringList monoSpaceFontPathList = fontInfoMgr->getAllMonoSpaceFontPath();
    QStringList strAllFontList = fontInfoMgr->getAllFontPath();
    qDebug() << "strAllFontList.size()" << strAllFontList.size() << endl;
    int index = 0;
    for (int i = 0; i < strAllFontList.size(); ++i) {
        QString filePath = strAllFontList.at(i);
        if (filePath.length() > 0) {
            index = insertFontItemData(filePath, index, chineseFontPathList, monoSpaceFontPathList, true);
        }
    }

    for (QString filePath : disableFontList) {
        index = insertFontItemData(filePath, index, chineseFontPathList, monoSpaceFontPathList, true, false);
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

void DFontPreviewListDataThread::onFileAdded(const QStringList &files, bool isFirstInstall)
{
    if (files.isEmpty()) {
        if (isFirstInstall) {
            Q_EMIT SignalManager::instance()->showInstallErrorDialog();
        }
        return;
    }


    if (m_mutex != nullptr)
        QMutexLocker locker(m_mutex);

    refreshFontListData(false, isFirstInstall, files);
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

QStringList DFontPreviewListDataThread::getDiffFontList() const
{
    QStringList fontList;
    for (DFontPreviewItemData item : m_diffFontModelList) {
        fontList << item.fontInfo.filePath;
    }
    return fontList;
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

    if (itemData.fontInfo.styleName.length() > 0) {
        itemData.strFontName =
            QString("%1-%2").arg(itemData.fontInfo.familyName).arg(itemData.fontInfo.styleName);
    } else {
        itemData.strFontName = itemData.fontInfo.familyName;
    }

    itemData.strFontId = QString::number(index);
    itemData.strFontFileName = filePathInfo.baseName();
    itemData.strFontPreview = m_view->getPreviewTextWithSize(&itemData.iFontSize);
    itemData.isEnabled = isEnabled;
    itemData.isPreviewEnabled = true;
    itemData.isCollected = false;
    itemData.isChineseFont = chineseFontPathList.contains(filePath);
    itemData.isMonoSpace = monoSpaceFontPathList.contains(filePath);

    itemData.fontInfo.isInstalled = true;

    /* Bug#16821 UT000591  添加字体后需要加入到Qt的字体数据库中，否则无法使用*/
    int appFontId = QFontDatabase::addApplicationFont(itemData.fontInfo.filePath);
    //中文字体
    if (itemData.fontInfo.isSystemFont && itemData.isChineseFont) {
        QStringList fontFamilyList = QFontDatabase::applicationFontFamilies(appFontId);
        if (fontFamilyList.size() > 1) {
            ++index;
            for (QString fontFamily : fontFamilyList) {
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

void DFontPreviewListDataThread::refreshFontListData(bool isStartup,  bool isFirstInstall, const QStringList &installFont)
{
    qDebug() << __FUNCTION__ << " begin";
    DFontInfoManager *fontInfoMgr = DFontInfoManager::instance();
    QStringList strAllFontList = fontInfoMgr->getAllFontPath();

    QList<DFontPreviewItemData> fontInfoList;
    if (isStartup) {
        fontInfoList = m_dbManager->getAllFontInfo();
        for (QString filePath : installFont) {
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
    for (int i = 0; i < fontInfoList.size(); ++i) {
        DFontPreviewItemData itemData = fontInfoList.at(i);

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
    isStartup = false;
    if (!isStartup) {
        //根据文件路径比较出不同的字体文件
        QSet<QString> allFontListSet = strAllFontList.toSet();
        QSet<QString> diffSet = allFontListSet.subtract(dbFilePathSet);
        qDebug() << "diffSet count:" << diffSet.count();
        if (diffSet.count() > 0) {
            int maxFontId = m_dbManager->getCurrMaxFontId();
            QList<QString> diffFilePathList = diffSet.toList();
            for (int i = 0; i < diffFilePathList.size(); ++i) {
                QString filePath = diffFilePathList.at(i);
                if (m_dbManager->isSystemFont(filePath) || installFont.contains(filePath)) {
                    bool isEnabled = (isStartup && installFont.contains(filePath)) ? false : true;
                    insertFontItemData(filePath, maxFontId + i + 1, chineseFontPathList, monoSpaceFontPathList, isStartup, isEnabled);
                }
            }
        }

        m_dbManager->commitAddFontInfo();
    }
    if (!isStartup && !installFont.isEmpty()) {
        Q_EMIT m_view->multiItemsAdded(m_diffFontModelList);
        Q_EMIT m_view->itemsSelected(installFont, isFirstInstall);
    } else {
        Q_EMIT m_view->multiItemsAdded(m_fontModelList);
    }
    qDebug() << __FUNCTION__ << " end";
}

void DFontPreviewListDataThread::removeFontData(const DFontPreviewItemData &removeItemData)
{
    m_diffFontModelList.clear();

    int removeIndex = -1;
    for (int i = 0; i < m_fontModelList.size(); i++) {
        if (m_fontModelList.at(i).fontInfo.filePath == removeItemData.fontInfo.filePath) {
            removeIndex = i;
            break;
        }
    }

    if (removeIndex != -1) {
        m_fontModelList.removeAt(removeIndex);
    }
}

void DFontPreviewListDataThread::syncFontEnableDisableStatusData(const QStringList &disableFontPathList)
{
    //disableFontPathList为被禁用的字体路径列表
//    if (disableFontPathList.size() == 0) {
//        return;
//    }

    QMap<QString, bool> disableFontMap;
    for (int i = 0; i < disableFontPathList.size(); i++) {
        QString disableFontPath = disableFontPathList.at(i);
        disableFontMap.insert(disableFontPath, true);
    }

    QList<DFontPreviewItemData> fontInfoList = m_dbManager->getAllFontInfo();
    QStringList fontList;

    for (int i = 0; i < fontInfoList.size(); i++) {
        DFontPreviewItemData fontItemData = fontInfoList.at(i);
        QString keyFilePath = fontItemData.fontInfo.filePath;
        fontList << keyFilePath;

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

//    for (QString disableFont : disableFontPathList) {
//        if (!fontList.contains(disableFont) && m_dbManager->isUserFont(disableFont)) {
//            m_view->enableFont(disableFont);
//        }
//    }
//    m_view->enableFonts();
}

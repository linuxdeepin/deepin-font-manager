#include "dfontpreviewlistdatathread.h"
#include "dfmxmlwrapper.h"

static DFontPreviewListDataThread *INSTANCE = nullptr;

DFontPreviewListDataThread *DFontPreviewListDataThread::instance()
{
    if (!INSTANCE) {
        INSTANCE = new DFontPreviewListDataThread;
    }

    return INSTANCE;
}

DFontPreviewListDataThread::DFontPreviewListDataThread()
    : m_dbManager(DFMDBManager::instance())
{
    mThread = new QThread();
    this->moveToThread(mThread);
    QObject::connect(mThread, SIGNAL(started()), this, SLOT(doWork()));
    connect(mThread, SIGNAL(finished()), mThread, SLOT(deleteLater()));
    mThread->start();
}

DFontPreviewListDataThread::~DFontPreviewListDataThread() {
}

void DFontPreviewListDataThread::doWork()
{
    m_fontModelList.clear();

    qDebug() << "doWork thread id = " << QThread::currentThreadId();
    QStringList fontNameList;
    DFontInfoManager *fontInfoMgr = DFontInfoManager::instance();
    QStringList disableFontList = DFMXmlWrapper::getFontConfigDisableFontPathList();

    int recordCount = m_dbManager->getRecordCount();
    if (recordCount > 0) {

        //从fontconfig配置文件同步字体启用/禁用状态数据
        syncFontEnableDisableStatusData(disableFontList);

        refreshFontListData(true);

        emit resultReady();

        return;
    }

    //开启事务
    m_dbManager->beginTransaction();

    QStringList chineseFontPathList = fontInfoMgr->getAllChineseFontPath();
    QStringList monoSpaceFontPathList = fontInfoMgr->getAllMonoSpaceFontPath();
    QStringList strAllFontList = fontInfoMgr->getAllFontPath();
    qDebug() << "strAllFontList.size()" << strAllFontList.size() << endl;
    for (int i = 0; i < strAllFontList.size(); ++i) {
        QString filePath = strAllFontList.at(i);
        if (filePath.length() > 0) {
            insertFontItemData(filePath, i + 1, chineseFontPathList, monoSpaceFontPathList, true);
        }
    }

    m_dbManager->endTransaction();

    emit resultReady();
}

QList<DFontPreviewItemData> DFontPreviewListDataThread::getFontModelList() const
{
    return m_fontModelList;
}

QList<DFontPreviewItemData> DFontPreviewListDataThread::getDiffFontModelList() const
{
    return m_diffFontModelList;
}

void DFontPreviewListDataThread::insertFontItemData(QString filePath,
                                                    int index,
                                                    QStringList chineseFontPathList,
                                                    QStringList monoSpaceFontPathList,
                                                    bool isStartup)
{
    DFontInfoManager *fontInfoMgr = DFontInfoManager::instance();
    DFontPreviewItemData itemData;
    QFileInfo filePathInfo(filePath);
    itemData.fontInfo = fontInfoMgr->getFontInfo(filePath);

    if (itemData.fontInfo.styleName.length() > 0) {
        itemData.strFontName =
            QString("%1-%2").arg(itemData.fontInfo.familyName).arg(itemData.fontInfo.styleName);
    } else {
        itemData.strFontName = itemData.fontInfo.familyName;
    }

    itemData.strFontId = QString::number(index);
    itemData.strFontFileName = filePathInfo.baseName();
    itemData.strFontPreview = FTM_DEFAULT_PREVIEW_TEXT;
    itemData.iFontSize = FTM_DEFAULT_PREVIEW_FONTSIZE;
    itemData.isEnabled = true;
    itemData.isPreviewEnabled = true;
    itemData.isCollected = false;
    itemData.isChineseFont = chineseFontPathList.contains(filePath);
    itemData.isMonoSpace = monoSpaceFontPathList.contains(filePath);

    itemData.fontInfo.isInstalled = true;

    m_dbManager->addFontInfo(itemData);

    if (!isStartup) {
        m_diffFontModelList.append(itemData);
    }

    m_fontModelList.append(itemData);
}

void DFontPreviewListDataThread::refreshFontListData(bool isStartup)
{
    DFontInfoManager *fontInfoMgr = DFontInfoManager::instance();
    QStringList strAllFontList = fontInfoMgr->getAllFontPath();

    QList<DFontPreviewItemData> fontInfoList = m_dbManager->getAllFontInfo();
    QStringList chineseFontPathList = fontInfoMgr->getAllChineseFontPath();
    QStringList monoSpaceFontPathList = fontInfoMgr->getAllMonoSpaceFontPath();

    QSet<QString> dbFilePathSet;
    for (int i = 0; i < fontInfoList.size(); ++i) {

        DFontPreviewItemData itemData = fontInfoList.at(i);

        if (isStartup) {
            m_fontModelList.append(itemData);
        }
        else {
            QString filePath = itemData.fontInfo.filePath;
            QFileInfo filePathInfo(filePath);
            dbFilePathSet.insert(filePath);
        }
    }

    m_diffFontModelList.clear();
    if (!isStartup) {
        //开启事务
        m_dbManager->beginTransaction();

        //根据文件路径比较出不同的字体文件
        QSet<QString> allFontListSet = strAllFontList.toSet();
        QSet<QString> diffSet = allFontListSet.subtract(dbFilePathSet);
        qDebug() << "diffSet count:" << diffSet.count();
        if (diffSet.count() > 0) {
            int maxFontId = m_dbManager->getCurrMaxFontId();
            QList<QString> diffFilePathList = diffSet.toList();
            for (int i = 0; i < diffFilePathList.size(); ++i) {
                QString filePath = diffFilePathList.at(i);
                if (filePath.length() > 0) {
                    insertFontItemData(filePath, maxFontId + i + 1, chineseFontPathList, monoSpaceFontPathList, isStartup);
                }
            }
        }
        m_dbManager->endTransaction();
    }
}

void DFontPreviewListDataThread::removeFontData(DFontPreviewItemData removeItemData)
{
    m_diffFontModelList.clear();

    int removeIndex = -1;
    for(int i=0; i<m_fontModelList.size(); i++) {
        DFontPreviewItemData itemData = m_fontModelList.at(i);
        if (itemData.strFontId == removeItemData.strFontId) {
            removeIndex = i;
        }
    }

    if (removeIndex != -1) {
        m_fontModelList.removeAt(removeIndex);
    }
}

void DFontPreviewListDataThread::syncFontEnableDisableStatusData(QStringList disableFontPathList)
{
    //disableFontPathList为被禁用的字体路径列表
    if (disableFontPathList.size() == 0) {
        return;
    }

    QMap<QString, bool> disableFontMap;
    for(int i=0; i<disableFontPathList.size(); i++) {
        QString disableFontPath = disableFontPathList.at(i);
        disableFontMap.insert(disableFontPath, true);
    }

    //开启事务
    m_dbManager->beginTransaction();

    QList<DFontPreviewItemData> fontInfoList = m_dbManager->getAllFontInfo();

    for(int i=0; i<fontInfoList.size(); i++) {
        DFontPreviewItemData fontItemData = fontInfoList.at(i);
        QString keyFilePath = fontItemData.fontInfo.filePath;

        //disableFontMap为被禁用的字体map
        if (disableFontMap.value(keyFilePath)) {
            fontItemData.isEnabled = false;
            fontItemData.isPreviewEnabled = false;
        }
        else {
            fontItemData.isEnabled = true;
            fontItemData.isPreviewEnabled = true;
        }

        m_dbManager->updateFontInfoByFontFilePath(keyFilePath, "isEnabled", QString::number(fontItemData.isEnabled));
    }

    m_dbManager->endTransaction();
}

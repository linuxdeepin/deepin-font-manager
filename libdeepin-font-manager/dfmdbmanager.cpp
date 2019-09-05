#include "dfmdbmanager.h"
#include "dfontinfomanager.h"

#include <QDir>

#include <DLog>

static DFMDBManager *INSTANCE = nullptr;

DFMDBManager::DFMDBManager(QObject *parent)
    : QObject(parent)
    , m_sqlUtil(new DSqliteUtil(QDir::homePath() + "/.deepin-font-manager/.font_manager.db"))
{
    QDir dbdir(QDir::homePath() + "/.deepin-font-manager/");
    if (!dbdir.exists()) {
        dbdir.mkdir(QDir::homePath() + "/.deepin-font-manager/");
        qDebug() << __FUNCTION__ << QDir::homePath() + "/.deepin-font-manager/";
    }
}

DFMDBManager::~DFMDBManager()
{
    delete m_sqlUtil;
}

DFMDBManager *DFMDBManager::instance()
{
    if (!INSTANCE) {
        INSTANCE = new DFMDBManager;
    }

    return INSTANCE;
}

inline bool DFMDBManager::isSystemFont(QString filePath)
{
    return filePath.contains("/usr/share/fonts/deepin-font-install") ? false : true;
}

DFontInfo *DFMDBManager::getDFontInfo(const QMap<QString, QString> &record)
{
    DFontInfo *pFontInfo = new DFontInfo;

    pFontInfo->filePath = record.value("filePath");
    pFontInfo->familyName = record.value("familyName");
    pFontInfo->styleName = record.value("styleName");
    pFontInfo->type = record.value("type");
    pFontInfo->version = record.value("version");
    pFontInfo->copyright = record.value("copyright");
    pFontInfo->description = record.value("description");
    pFontInfo->sysVersion = record.value("sysVersion");
    pFontInfo->isInstalled = record.value("isInstalled").toInt();
    pFontInfo->isError = record.value("isError").toInt();
    pFontInfo->isSystemFont = isSystemFont(pFontInfo->filePath);

    return pFontInfo;
}

QList<DFontPreviewItemData> DFMDBManager::getAllFontInfo()
{
    QList<DFontPreviewItemData> fontItemDataList;

    QList<QMap<QString, QString>> recordList;

    QList<QString> keyList;
    keyList.append("fontId");
    keyList.append("fontName");
    keyList.append("isEnabled");
    keyList.append("isCollected");
    keyList.append("filePath");
    keyList.append("familyName");
    keyList.append("styleName");
    keyList.append("type");
    keyList.append("version");
    keyList.append("copyright");
    keyList.append("description");
    keyList.append("sysVersion");
    keyList.append("isInstalled");
    keyList.append("isError");
    keyList.append("appFontId");

    m_sqlUtil->findRecords(keyList, &recordList);
    for (int i = 0; i < recordList.size(); ++i) {
        QMap<QString, QString> record = recordList.at(i);
        if (record.size() > 0) {
            DFontPreviewItemData itemData;
            itemData.strFontId = record.value("fontId");
            QString filePath = record.value("filePath");
            itemData.strFontName = record.value("fontName");
            QFileInfo filePathInfo(filePath);
            itemData.strFontFileName = filePathInfo.baseName();
            itemData.strFontPreview = FTM_DEFAULT_PREVIEW_TEXT;
            itemData.iFontSize = FTM_DEFAULT_PREVIEW_FONTSIZE;
            itemData.isEnabled = record.value("isEnabled").toInt();
            itemData.isCollected = record.value("isCollected").toInt();
            itemData.appFontId = record.value("appFontId").toInt();
            itemData.pFontInfo = getDFontInfo(record);

            fontItemDataList.push_back(itemData);
        }
    }

    return fontItemDataList;
}

int DFMDBManager::getRecordCount()
{
    return m_sqlUtil->getRecordCount();
}

QList<DFontPreviewItemData> DFMDBManager::findFontInfosByCondition(QMap<QString, QString> whereMap)
{
    QList<DFontPreviewItemData> fontItemDataList;

    QList<QMap<QString, QString>> recordList;

    QList<QString> keyList;
    keyList.append("fontId");
    keyList.append("fontName");
    keyList.append("isEnabled");
    keyList.append("isCollected");
    keyList.append("filePath");
    keyList.append("familyName");
    keyList.append("styleName");
    keyList.append("type");
    keyList.append("version");
    keyList.append("copyright");
    keyList.append("description");
    keyList.append("sysVersion");
    keyList.append("isInstalled");
    keyList.append("isError");
    keyList.append("appFontId");

    m_sqlUtil->findRecords(keyList, whereMap, &recordList);

    for (int i = 0; i < recordList.size(); ++i) {
        QMap<QString, QString> record = recordList.at(i);
        if (record.size() > 0) {
            DFontPreviewItemData itemData;
            itemData.strFontId = record.value("fontId");
            QString filePath = record.value("filePath");
            itemData.strFontName = record.value("fontName");
            QFileInfo filePathInfo(filePath);
            itemData.strFontFileName = filePathInfo.baseName();
            itemData.strFontPreview = FTM_DEFAULT_PREVIEW_TEXT;
            itemData.iFontSize = FTM_DEFAULT_PREVIEW_FONTSIZE;
            itemData.isEnabled = record.value("isEnabled").toInt();
            itemData.isCollected = record.value("isCollected").toInt();
            itemData.appFontId = record.value("appFontId").toInt();
            itemData.pFontInfo = getDFontInfo(record);

            fontItemDataList.push_back(itemData);
        }
    }

    return fontItemDataList;
}

DFontPreviewItemData DFMDBManager::findFontInfoByFontId(QString strFontId)
{
    QList<DFontPreviewItemData> fontItemDataList;

    QList<QMap<QString, QString>> recordList;

    QList<QString> keyList;
    keyList.append("fontId");
    keyList.append("fontName");
    keyList.append("isEnabled");
    keyList.append("isCollected");
    keyList.append("filePath");
    keyList.append("familyName");
    keyList.append("styleName");
    keyList.append("type");
    keyList.append("version");
    keyList.append("copyright");
    keyList.append("description");
    keyList.append("sysVersion");
    keyList.append("isInstalled");
    keyList.append("isError");
    keyList.append("appFontId");

    QMap<QString, QString> whereMap;
    whereMap.insert("fontId", strFontId);
    m_sqlUtil->findRecords(keyList, whereMap, &recordList);

    for (int i = 0; i < recordList.size(); ++i) {
        QMap<QString, QString> record = recordList.at(i);
        if (record.size() > 0) {
            DFontPreviewItemData itemData;
            itemData.strFontId = record.value("fontId");
            QString filePath = record.value("filePath");
            itemData.strFontName = record.value("fontName");
            QFileInfo filePathInfo(filePath);
            itemData.strFontFileName = filePathInfo.baseName();
            itemData.strFontPreview = FTM_DEFAULT_PREVIEW_TEXT;
            itemData.iFontSize = FTM_DEFAULT_PREVIEW_FONTSIZE;
            itemData.isEnabled = record.value("isEnabled").toInt();
            itemData.isCollected = record.value("isCollected").toInt();
            itemData.appFontId = record.value("appFontId").toInt();
            itemData.pFontInfo = getDFontInfo(record);

            fontItemDataList.push_back(itemData);
        }
    }

    DFontPreviewItemData itemData;
    itemData.strFontId = "-1";
    itemData.pFontInfo = nullptr;
    if (recordList.size() > 0) {
        itemData = fontItemDataList.first();
    }

    return itemData;
}

DFontPreviewItemData DFMDBManager::findFontInfoByFilePath(QString strFontFilePath)
{
    QList<DFontPreviewItemData> fontItemDataList;

    QList<QMap<QString, QString>> recordList;

    QList<QString> keyList;
    keyList.append("fontId");
    keyList.append("fontName");
    keyList.append("isEnabled");
    keyList.append("isCollected");
    keyList.append("filePath");
    keyList.append("familyName");
    keyList.append("styleName");
    keyList.append("type");
    keyList.append("version");
    keyList.append("copyright");
    keyList.append("description");
    keyList.append("sysVersion");
    keyList.append("isInstalled");
    keyList.append("isError");
    keyList.append("appFontId");

    QMap<QString, QString> whereMap;
    whereMap.insert("filePath", strFontFilePath);
    m_sqlUtil->findRecords(keyList, whereMap, &recordList);

    for (int i = 0; i < recordList.size(); ++i) {
        QMap<QString, QString> record = recordList.at(i);
        if (record.size() > 0) {
            DFontPreviewItemData itemData;
            itemData.strFontId = record.value("fontId");
            QString filePath = record.value("filePath");
            itemData.strFontName = record.value("fontName");
            QFileInfo filePathInfo(filePath);
            itemData.strFontFileName = filePathInfo.baseName();
            itemData.strFontPreview = FTM_DEFAULT_PREVIEW_TEXT;
            itemData.iFontSize = FTM_DEFAULT_PREVIEW_FONTSIZE;
            itemData.isEnabled = record.value("isEnabled").toInt();
            itemData.isCollected = record.value("isCollected").toInt();
            itemData.appFontId = record.value("appFontId").toInt();
            itemData.pFontInfo = getDFontInfo(record);

            fontItemDataList.push_back(itemData);
        }
    }

    DFontPreviewItemData itemData;
    itemData.strFontId = "-1";
    itemData.pFontInfo = nullptr;
    if (recordList.size() > 0) {
        itemData = fontItemDataList.first();
    }

    return itemData;
}

int DFMDBManager::getCurrMaxFontId()
{
    return m_sqlUtil->getMaxFontId();
}

bool DFMDBManager::isFontInfoExist(DFontInfo *newFileFontInfo)
{
    QList<DFontPreviewItemData> fontItemDataList;

    QList<QMap<QString, QString>> recordList;

    QList<QString> keyList;
    keyList.append("fontId");
    keyList.append("fontName");
    keyList.append("isEnabled");
    keyList.append("isCollected");
    keyList.append("filePath");
    keyList.append("familyName");
    keyList.append("styleName");
    keyList.append("type");
    keyList.append("version");
    keyList.append("copyright");
    keyList.append("description");
    keyList.append("sysVersion");
    keyList.append("isInstalled");
    keyList.append("isError");
    keyList.append("appFontId");

    QMap<QString, QString> whereMap;
    whereMap.insert("familyName", newFileFontInfo->familyName);
    whereMap.insert("styleName", newFileFontInfo->styleName);
    m_sqlUtil->findRecords(keyList, whereMap, &recordList);

    if (recordList.size() > 0) {
        return true;
    }

    return false;
}

QMap<QString, QString> DFMDBManager::mapItemData(DFontPreviewItemData itemData)
{
    QMap<QString, QString> mapData;
    mapData.insert("fontId", itemData.strFontId);
    mapData.insert("fontName", itemData.strFontName);
    mapData.insert("isEnabled", QString::number(itemData.isEnabled));
    mapData.insert("isCollected", QString::number(itemData.isCollected));
    mapData.insert("filePath", itemData.pFontInfo->filePath);
    mapData.insert("familyName", itemData.pFontInfo->familyName);
    mapData.insert("styleName", itemData.pFontInfo->styleName);
    mapData.insert("type", itemData.pFontInfo->type);
    mapData.insert("version", itemData.pFontInfo->version);
    mapData.insert("copyright", itemData.pFontInfo->copyright);
    mapData.insert("description", itemData.pFontInfo->description);
    mapData.insert("sysVersion", itemData.pFontInfo->sysVersion);
    mapData.insert("isInstalled", QString::number(itemData.pFontInfo->isInstalled));
    mapData.insert("isError", QString::number(itemData.pFontInfo->isError));
    mapData.insert("appFontId", QString::number(itemData.appFontId));

    return mapData;
}

bool DFMDBManager::addFontInfo(DFontPreviewItemData itemData)
{
    return m_sqlUtil->addRecord(mapItemData(itemData));
}

bool DFMDBManager::deleteFontInfo(const QString &strKey, const QString &strValue)
{
    QMap<QString, QString> where;
    where.insert(strKey, strValue);
    return m_sqlUtil->delRecord(where);
}

bool DFMDBManager::deleteFontInfoByFontId(const QString &strFontId)
{
    QMap<QString, QString> where;
    where.insert("fontId", strFontId);
    return m_sqlUtil->delRecord(where);
}

bool DFMDBManager::updateFontInfo(const QMap<QString, QString> &whereMap, const QMap<QString, QString> &dataMap)
{
    return m_sqlUtil->updateRecord(whereMap, dataMap);
}

bool DFMDBManager::updateFontInfoByFontId(const QString &strFontId, const QMap<QString, QString> &dataMap)
{
    QMap<QString, QString> where;
    where.insert("fontId", strFontId);

    return m_sqlUtil->updateRecord(where, dataMap);
}

bool DFMDBManager::updateFontInfoByFontId(const QString &strFontId, const QString &strKey, const QString &strValue)
{
    QMap<QString, QString> where;
    where.insert("fontId", strFontId);

    QMap<QString, QString> dataMap;
    dataMap.insert(strKey, strValue);

    return m_sqlUtil->updateRecord(where, dataMap);
}

void DFMDBManager::beginTransaction()
{
    m_sqlUtil->m_db.transaction();
}

void DFMDBManager::endTransaction()
{
    m_sqlUtil->m_db.commit();
}

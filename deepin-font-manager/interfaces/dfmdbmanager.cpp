#include "dfmdbmanager.h"
#include "dfontinfomanager.h"

#include <QFileInfo>

static DFMDBManager *INSTANCE = nullptr;

DFMDBManager::DFMDBManager(QObject *parent)
    : QObject(parent)
    , m_sqlUtil(new DSqliteUtil(".font_manager.db"))
{
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

QList<DFontPreviewItemData> DFMDBManager::getAllFontInfo()
{
    QList<DFontPreviewItemData> fontItemDataList;

    QList<QMap<QString, QString>> recordList;

    QList<QString> keyList;
    keyList.append("fontId");
    keyList.append("fontName");
    keyList.append("familyName");
    keyList.append("filePath");
    keyList.append("isEnabled");
    keyList.append("isCollected");
    keyList.append("appFontId");

    m_sqlUtil->findRecords(keyList, &recordList);

    DFontInfoManager *fontInfoMgr = DFontInfoManager::instance();

    for (int i = 0; i < recordList.size(); ++i) {
        QMap<QString, QString> record = recordList.at(i);
        if (record.size() > 0) {
            DFontPreviewItemData itemData;
            itemData.strFontId = record.value("fontId");
            QString filePath = record.value("filePath");
            QFileInfo filePathInfo(filePath);
            itemData.pFontInfo = fontInfoMgr->getFontInfo(filePath);
            itemData.strFontName = record.value("fontName");
            itemData.pFontInfo->familyName = record.value("familyName");
            itemData.strFontFileName = filePathInfo.baseName();
            itemData.strFontPreview = FTM_DEFAULT_PREVIEW_TEXT;
            itemData.iFontSize = FTM_DEFAULT_PREVIEW_FONTSIZE;
            itemData.isEnabled = record.value("isEnabled").toInt();
            itemData.isCollected = record.value("isCollected").toInt();
            itemData.appFontId = record.value("appFontId").toInt();

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
    keyList.append("familyName");
    keyList.append("filePath");
    keyList.append("isEnabled");
    keyList.append("isCollected");
    keyList.append("appFontId");

    m_sqlUtil->findRecords(keyList, whereMap, &recordList);

    DFontInfoManager *fontInfoMgr = DFontInfoManager::instance();

    for (int i = 0; i < recordList.size(); ++i) {
        QMap<QString, QString> record = recordList.at(i);
        if (record.size() > 0) {
            DFontPreviewItemData itemData;
            itemData.strFontId = record.value("fontId");
            QString filePath = record.value("filePath");
            QFileInfo filePathInfo(filePath);
            itemData.pFontInfo = fontInfoMgr->getFontInfo(filePath);
            itemData.strFontName = record.value("fontName");
            itemData.pFontInfo->familyName = record.value("familyName");
            itemData.strFontFileName = filePathInfo.baseName();
            itemData.strFontPreview = FTM_DEFAULT_PREVIEW_TEXT;
            itemData.iFontSize = FTM_DEFAULT_PREVIEW_FONTSIZE;
            itemData.isEnabled = record.value("isEnabled").toInt();
            itemData.isCollected = record.value("isCollected").toInt();
            itemData.appFontId = record.value("appFontId").toInt();

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
    keyList.append("familyName");
    keyList.append("filePath");
    keyList.append("isEnabled");
    keyList.append("isCollected");
    keyList.append("appFontId");

    QMap<QString, QString> whereMap;
    whereMap.insert("fontId", strFontId);
    m_sqlUtil->findRecords(keyList, whereMap, &recordList);

    DFontInfoManager *fontInfoMgr = DFontInfoManager::instance();

    for (int i = 0; i < recordList.size(); ++i) {
        QMap<QString, QString> record = recordList.at(i);
        if (record.size() > 0) {
            DFontPreviewItemData itemData;
            itemData.strFontId = record.value("fontId");
            QString filePath = record.value("filePath");
            QFileInfo filePathInfo(filePath);
            itemData.pFontInfo = fontInfoMgr->getFontInfo(filePath);
            itemData.strFontName = record.value("fontName");
            itemData.pFontInfo->familyName = record.value("familyName");
            itemData.strFontFileName = filePathInfo.baseName();
            itemData.strFontPreview = FTM_DEFAULT_PREVIEW_TEXT;
            itemData.iFontSize = FTM_DEFAULT_PREVIEW_FONTSIZE;
            itemData.isEnabled = record.value("isEnabled").toInt();
            itemData.isCollected = record.value("isCollected").toInt();
            itemData.appFontId = record.value("appFontId").toInt();

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
    keyList.append("familyName");
    keyList.append("filePath");
    keyList.append("isEnabled");
    keyList.append("isCollected");
    keyList.append("appFontId");

    QMap<QString, QString> whereMap;
    whereMap.insert("filePath", strFontFilePath);
    m_sqlUtil->findRecords(keyList, whereMap, &recordList);

    DFontInfoManager *fontInfoMgr = DFontInfoManager::instance();

    for (int i = 0; i < recordList.size(); ++i) {
        QMap<QString, QString> record = recordList.at(i);
        if (record.size() > 0) {
            DFontPreviewItemData itemData;
            itemData.strFontId = record.value("fontId");
            QString filePath = record.value("filePath");
            QFileInfo filePathInfo(filePath);
            itemData.pFontInfo = fontInfoMgr->getFontInfo(filePath);
            itemData.strFontName = record.value("fontName");
            itemData.pFontInfo->familyName = record.value("familyName");
            itemData.strFontFileName = filePathInfo.baseName();
            itemData.strFontPreview = FTM_DEFAULT_PREVIEW_TEXT;
            itemData.iFontSize = FTM_DEFAULT_PREVIEW_FONTSIZE;
            itemData.isEnabled = record.value("isEnabled").toInt();
            itemData.isCollected = record.value("isCollected").toInt();
            itemData.appFontId = record.value("appFontId").toInt();

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

bool DFMDBManager::isFontInfoExist(QString strFontFilePath)
{
    QList<DFontPreviewItemData> fontItemDataList;

    QList<QMap<QString, QString>> recordList;

    QList<QString> keyList;
    keyList.append("fontId");
    keyList.append("fontName");
    keyList.append("familyName");
    keyList.append("filePath");
    keyList.append("isEnabled");
    keyList.append("isCollected");
    keyList.append("appFontId");

    QMap<QString, QString> whereMap;
    whereMap.insert("filePath", strFontFilePath);
    m_sqlUtil->findRecords(keyList, whereMap, &recordList);

    if (recordList.size() > 0) {
        return true;
    }

    return false;
}

QMap<QString, QString> DFMDBManager::mapItemData(DFontPreviewItemData itemData)
{
    QMap<QString, QString> mapData;
    mapData.insert("fontName", itemData.strFontName);
    mapData.insert("familyName", itemData.pFontInfo->familyName);
    mapData.insert("isEnabled", QString::number(itemData.isEnabled));
    mapData.insert("isCollected", QString::number(itemData.isCollected));
    mapData.insert("filePath", itemData.pFontInfo->filePath);
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

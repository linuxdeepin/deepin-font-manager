#include "dfmdbmanager.h"
#include "dfontinfomanager.h"

#include <QDir>

#include <DLog>

static DFMDBManager *INSTANCE = nullptr;

DFMDBManager::DFMDBManager(QObject *parent)
    : QObject(parent)
    , m_sqlUtil(new DSqliteUtil(QDir::homePath() + "/.deepin-font-manager/.font_manager.db"))
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

inline bool DFMDBManager::isSystemFont(QString filePath)
{
    return filePath.contains("/usr/share/fonts/deepin-font-install") ? false : true;
}

DFontPreviewItemData DFMDBManager::parseRecordToItemData(const QMap<QString, QString> &record)
{
    DFontPreviewItemData itemData;

    itemData.strFontId = record.value("fontId");
    QString filePath = record.value("filePath");
    itemData.strFontName = record.value("fontName");
    QFileInfo filePathInfo(filePath);
    itemData.strFontFileName = filePathInfo.baseName();
    itemData.strFontPreview = FTM_DEFAULT_PREVIEW_TEXT;
    itemData.iFontSize = FTM_DEFAULT_PREVIEW_FONTSIZE;
    itemData.isEnabled = record.value("isEnabled").toInt();
    itemData.isPreviewEnabled = itemData.isEnabled;
    itemData.isCollected = record.value("isCollected").toInt();
    itemData.isChineseFont = record.value("isChineseFont").toInt();
    itemData.isMonoSpace = record.value("isMonoSpace").toInt();

    itemData.fontInfo = getDFontInfo(record);

    return itemData;
}

DFontInfo DFMDBManager::getDFontInfo(const QMap<QString, QString> &record)
{
    DFontInfo fontInfo;

    fontInfo.filePath = record.value("filePath");
    fontInfo.familyName = record.value("familyName");
    fontInfo.styleName = record.value("styleName");
    fontInfo.type = record.value("type");
    fontInfo.version = record.value("version");
    fontInfo.copyright = record.value("copyright");
    fontInfo.description = record.value("description");
    fontInfo.sysVersion = record.value("sysVersion");
    fontInfo.isInstalled = record.value("isInstalled").toInt();
    fontInfo.isError = record.value("isError").toInt();
    fontInfo.isSystemFont = isSystemFont(fontInfo.filePath);

    return fontInfo;
}

void DFMDBManager::appendAllKeys(QList<QString> &keyList)
{
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
    keyList.append("isChineseFont");
    keyList.append("isMonoSpace");
}

QList<DFontPreviewItemData> DFMDBManager::getAllFontInfo()
{
    QList<DFontPreviewItemData> fontItemDataList;

    QList<QMap<QString, QString>> recordList;

    QList<QString> keyList;
    appendAllKeys(keyList);

    m_sqlUtil->findRecords(keyList, &recordList);
    for (int i = 0; i < recordList.size(); ++i) {
        QMap<QString, QString> record = recordList.at(i);
        if (record.size() > 0) {
            DFontPreviewItemData itemData = parseRecordToItemData(record);
            fontItemDataList.push_back(itemData);
        }
    }

    return fontItemDataList;
}

int DFMDBManager::getRecordCount()
{
    return m_sqlUtil->getRecordCount();
}

int DFMDBManager::getCurrMaxFontId()
{
    return m_sqlUtil->getMaxFontId();
}

bool DFMDBManager::isFontInfoExist(const DFontInfo &newFileFontInfo)
{
    QList<DFontPreviewItemData> fontItemDataList;

    QList<QMap<QString, QString>> recordList;

    QList<QString> keyList;
    appendAllKeys(keyList);

    QMap<QString, QString> whereMap;
    whereMap.insert("familyName", newFileFontInfo.familyName);
    whereMap.insert("styleName", newFileFontInfo.styleName);
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
    mapData.insert("isChineseFont", QString::number(itemData.isChineseFont));
    mapData.insert("isMonoSpace", QString::number(itemData.isMonoSpace));
    mapData.insert("filePath", itemData.fontInfo.filePath);
    mapData.insert("familyName", itemData.fontInfo.familyName);
    mapData.insert("styleName", itemData.fontInfo.styleName);
    mapData.insert("type", itemData.fontInfo.type);
    mapData.insert("version", itemData.fontInfo.version);
    mapData.insert("copyright", itemData.fontInfo.copyright);
    mapData.insert("description", itemData.fontInfo.description);
    mapData.insert("sysVersion", itemData.fontInfo.sysVersion);
    mapData.insert("isInstalled", QString::number(itemData.fontInfo.isInstalled));
    mapData.insert("isError", QString::number(itemData.fontInfo.isError));

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

bool DFMDBManager::updateFontInfoByFontFilePath(const QString &strFontFilePath, const QString &strKey, const QString &strValue)
{
    QMap<QString, QString> where;
    where.insert("filePath", strFontFilePath);

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

#ifndef DFMDBMANAGER_H
#define DFMDBMANAGER_H

#include "dsqliteutil.h"
#include "dfontpreviewitemdef.h"

class DFMDBManager: public QObject
{
    Q_OBJECT
public:
    static DFMDBManager *instance();
    DFMDBManager(QObject *parent = nullptr);
    ~DFMDBManager();

    QList<DFontPreviewItemData> getAllFontInfo();
    int getRecordCount();

    int getCurrMaxFontId();

    bool isFontInfoExist(const DFontInfo &newFileFontInfo);

    bool addFontInfo(DFontPreviewItemData itemData);
    bool deleteFontInfoByFontMap(const QMap<QString, QString> &fontDelMap);
    bool updateFontInfo(const QMap<QString, QString> &whereMap, const QMap<QString, QString> &dataMap);
    bool updateFontInfoByFontId(const QString &strFontId, const QMap<QString, QString> &dataMap);
    bool updateFontInfoByFontId(const QString &strFontId, const QString &strKey, const QString &strValue);

    bool updateFontInfoByFontFilePath(const QString &strFontFilePath, const QString &strKey, const QString &strValue);

    void beginTransaction();
    void endTransaction();

private:
    DFontPreviewItemData parseRecordToItemData(const QMap<QString, QString> &record);
    QMap<QString, QString> mapItemData(DFontPreviewItemData itemData);
    DFontInfo getDFontInfo(const QMap<QString, QString> &record);
    inline bool isSystemFont(QString filePath);
    inline void appendAllKeys(QList<QString> &keyList);

    DSqliteUtil *m_sqlUtil;
};

#endif // DFMDBMANAGER_H

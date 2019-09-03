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

    QList<DFontPreviewItemData> findFontInfosByCondition(QMap<QString, QString> whereMap);

    DFontPreviewItemData findFontInfoByFontId(QString strFontId);
    DFontPreviewItemData findFontInfoByFilePath(QString strFontFilePath);
    int getCurrMaxFontId();

    bool isFontInfoExist(DFontInfo *newFileFontInfo);

    bool addFontInfo(DFontPreviewItemData itemData);
    bool deleteFontInfo(const QString &strKey, const QString &strValue);
    bool deleteFontInfoByFontId(const QString &strFontId);
    bool updateFontInfo(const QMap<QString, QString> &whereMap, const QMap<QString, QString> &dataMap);
    bool updateFontInfoByFontId(const QString &strFontId, const QMap<QString, QString> &dataMap);
    bool updateFontInfoByFontId(const QString &strFontId, const QString &strKey, const QString &strValue);

    void beginTransaction();
    void endTransaction();

private:
    QMap<QString, QString> mapItemData(DFontPreviewItemData itemData);
    DFontInfo *getDFontInfo(const QMap<QString, QString> &record);
    inline bool isSystemFont(QString filePath);

    DSqliteUtil *m_sqlUtil;
};

#endif // DFMDBMANAGER_H

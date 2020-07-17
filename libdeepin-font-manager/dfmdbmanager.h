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
    QStringList getInstalledFontsPath();//
    QString isFontInfoExist(const DFontInfo &newFileFontInfo);

    bool addFontInfo(const DFontPreviewItemData &itemData);
    bool deleteFontInfoByFontMap(const QMap<QString, QString> &fontDelMap);
    bool updateFontInfo(const QMap<QString, QString> &whereMap, const QMap<QString, QString> &dataMap);
    bool updateFontInfoByFontId(const QString &strFontId, const QMap<QString, QString> &dataMap);
    bool updateFontInfoByFontId(const QString &strFontId, const QString &strKey, const QString &strValue);

    bool updateFontInfoByFontFilePath(const QString &strFontFilePath, const QString &strKey, const QString &strValue);

    // batch operation
    void commitAddFontInfo();
    void addFontInfo(const QList<DFontPreviewItemData> &fontList);
    void deleteFontInfo(const DFontPreviewItemData &itemData);
    void deleteFontInfo(const QList<DFontPreviewItemData> &fontList);
    void commitDeleteFontInfo();
    void updateFontInfo(const DFontPreviewItemData &fontList, const QString &strKey);
    void updateFontInfo(const QList<DFontPreviewItemData> &fontList, const QString &strKey);
    void commitUpdateFontInfo();

    void beginTransaction();
    void endTransaction();

    bool isSystemFont(const QString &filePath);
    bool isUserFont(const QString &filePath);

private:
    DFontPreviewItemData parseRecordToItemData(const QMap<QString, QString> &record);
    QMap<QString, QString> mapItemData(DFontPreviewItemData itemData);
    DFontInfo getDFontInfo(const QMap<QString, QString> &record);
    inline void appendAllKeys(QList<QString> &keyList);

    DSqliteUtil *m_sqlUtil;
    QList<DFontPreviewItemData> m_addFontList;
    QList<DFontPreviewItemData> m_delFontList;
    QList<DFontPreviewItemData> m_updateFontList;
    QString m_strKey;
};

#endif // DFMDBMANAGER_H

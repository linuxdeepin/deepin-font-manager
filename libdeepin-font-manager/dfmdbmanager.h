#ifndef DFMDBMANAGER_H
#define DFMDBMANAGER_H

#include "dsqliteutil.h"
#include "dfontpreviewitemdef.h"

class DFMDBManager: public QObject
{
    Q_OBJECT
public:
    static DFMDBManager *instance();
    explicit DFMDBManager(QObject *parent = nullptr);
    ~DFMDBManager();

    QList<DFontPreviewItemData> getAllFontInfo(QList<DFontPreviewItemData> *deletedFontInfo = nullptr);
    int getRecordCount();
    int getCurrMaxFontId();
    QStringList getInstalledFontsPath();
    QString isFontInfoExist(const DFontInfo &newFileFontInfo);

    bool addFontInfo(const DFontPreviewItemData &itemData);
    bool updateFontInfo(const QMap<QString, QString> &whereMap, const QMap<QString, QString> &dataMap);
    void updateSP3FamilyName(const QList<DFontInfo> &fontList, bool inFontList = false);

    // batch operation
    void commitAddFontInfo();
    void addFontInfo(const QList<DFontPreviewItemData> &fontList);
    void deleteFontInfo(const DFontPreviewItemData &itemData);
    void deleteFontInfo(const QList<DFontPreviewItemData> &fontList);
    void commitDeleteFontInfo();
    void updateFontInfo(const DFontPreviewItemData &fontList, const QString &strKey);
    void updateFontInfo(const QList<DFontPreviewItemData> &fontList, const QString &strKey);
    void commitUpdateFontInfo();
    //去除非法记录
    void checkIfEmpty();

    //开启事务
    void beginTransaction()
    {
        m_sqlUtil->m_db.transaction();
    }

    //关闭事务
    void endTransaction()
    {
        m_sqlUtil->m_db.commit();
    }

    bool isSystemFont(const QString &filePath)
    {
        return filePath.contains("/usr/share/fonts/");
    }

private:
    DFontPreviewItemData parseRecordToItemData(const QMap<QString, QString> &record);
    DFontInfo getDFontInfo(const QMap<QString, QString> &record);
    inline void appendAllKeys(QList<QString> &keyList);

    DSqliteUtil *m_sqlUtil;
    QList<DFontPreviewItemData> m_addFontList;
    QList<DFontPreviewItemData> m_delFontList;
    QList<DFontPreviewItemData> m_updateFontList;
    QString m_strKey;
};

#endif // DFMDBMANAGER_H

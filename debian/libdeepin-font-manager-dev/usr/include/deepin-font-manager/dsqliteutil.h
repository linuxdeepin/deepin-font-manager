#ifndef DSQLITEUTIL_H
#define DSQLITEUTIL_H

#include "dfontpreviewitemdef.h"

#include <QList>
#include <QMap>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>
#include <QMutexLocker>

class DSqliteUtil
{
public:
    explicit DSqliteUtil(const QString &strDatabase = "sqlite3db.db");
    ~DSqliteUtil();

    //新建数据库连接
    bool createConnection(const QString &strConn);

    //创建数据库表
    bool createTable();

    //增加数据
    bool addRecord(QMap<QString, QString> data, const QString &table_name = "t_fontmanager");

    //删除一条记录
    bool delRecord(QMap<QString, QString> where, const QString &table_name = "t_fontmanager");

    //删除所有记录
    bool delAllRecords(const QString &table_name = "t_fontmanager");

    //更新数据
    bool updateRecord(QMap<QString, QString> where, QMap<QString, QString> data,
                      const QString &table_name = "t_fontmanager");

    //查找所有记录
    bool findAllRecords(const QList<QString> &key, QList<QMap<QString, QString>> &row,
                        const QString &table_name = "t_fontmanager");

    //查找所有记录
    bool findRecords(const QList<QString> &key, QList<QMap<QString, QString>> *row,
                     const QString &table_name = "t_fontmanager");

    //按条件查找
    bool findRecords(const QList<QString> &key, const QMap<QString, QString> &where, QList<QMap<QString, QString>> *row,
                     const QString &table_name = "t_fontmanager");

    int getRecordCount(const QString &table_name = "t_fontmanager");

    int getMaxFontId(const QString &table_name = "t_fontmanager");
    // batch operation
    void addFontInfo(const QList<DFontPreviewItemData> &fontList, const QString &table_name = "t_fontmanager");
    void deleteFontInfo(const QList<DFontPreviewItemData> &fontList, const QString &table_name = "t_fontmanager");
    void updateFontInfo(const QList<DFontPreviewItemData> &fontList, const QString &key, const QString &table_name = "t_fontmanager");
    QString escapeString(const QString &str);
    inline void finish()
    {
        if (m_query == nullptr)
            return;
//        qDebug() << __FUNCTION__ << m_query->isActive();
        m_query->finish();
//        m_query->clear();
    }

    QSqlDatabase m_db;

    //获取已安装字体路径
    QStringList getInstalledFontsPath();
protected:

private:
    QString m_strDatabase;

    QSqlQuery *m_query;
    mutable QMutex mutex;
};

#endif  // DSQLITEUTIL_H

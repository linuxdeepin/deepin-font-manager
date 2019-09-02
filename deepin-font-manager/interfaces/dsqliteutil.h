#ifndef DSQLITEUTIL_H
#define DSQLITEUTIL_H

#include <QList>
#include <QMap>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

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
    bool addRecord(QMap<QString, QString> data, QString table_name = "t_fontmanager");

    //删除一条记录
    bool delRecord(QMap<QString, QString> where, QString table_name = "t_fontmanager");

    //删除所有记录
    bool delAllRecords(QString table_name = "t_fontmanager");

    //更新数据
    bool updateRecord(QMap<QString, QString> where, QMap<QString, QString> data,
                      QString table_name = "t_fontmanager");

    //查找所有记录
    bool findRecords(QList<QString> key, QList<QMap<QString, QString>> *row,
                     QString table_name = "t_fontmanager");

    //按条件查找
    bool findRecords(QList<QString> key, QMap<QString, QString> where, QList<QMap<QString, QString>> *row,
                     QString table_name = "t_fontmanager");

    int getRecordCount(QString table_name = "t_fontmanager");

    QSqlDatabase m_db;

private:
    QString m_strDatabase;

    QSqlQuery *m_query;
};

#endif  // DSQLITEUTIL_H

/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:
*
* Maintainer:
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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

/*************************************************************************
 <Class>         DSqliteUtil
 <Description>   数据库处理类
 <Author>
 <Note>          null
*************************************************************************/
class DSqliteUtil
{
public:
    explicit DSqliteUtil(const QString &strDatabase = "sqlite3db.db");
    ~DSqliteUtil();

    //创建数据库连接
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
    //获取记录总数
    int getRecordCount(const QString &table_name = "t_fontmanager");
    //获取最大字体id
    int getMaxFontId(const QString &table_name = "t_fontmanager");
    //批量添加字体信息
    void addFontInfo(const QList<DFontPreviewItemData> &fontList, const QString &table_name = "t_fontmanager");
    //批量删除字体信息
    void deleteFontInfo(const QList<DFontPreviewItemData> &fontList, const QString &table_name = "t_fontmanager");
    //批量更新字体信息
    void updateFontInfo(const QList<DFontPreviewItemData> &fontList, const QString &key, const QString &table_name = "t_fontmanager");
    //字符串处理
    QString escapeString(const QString &str);
    //更新sp2的sp3familyName
    void updateSP3FamilyName(const QList<DFontInfo> &fontList, bool inFontList = false);
    //去除非法记录
    void checkIfEmpty();
    //内联函数-执行结束处理函数
    inline void finish()
    {
        if (m_query == nullptr)
            return;
        m_query->finish();
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

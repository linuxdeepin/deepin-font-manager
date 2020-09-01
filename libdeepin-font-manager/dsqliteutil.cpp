#include "dsqliteutil.h"

#include <QDir>

#include <DLog>

//#ifdef qDebug
//#undef qDebug
//#define qDebug while(false) QMessageLogger().noDebug
//#endif

/*************************************************************************
 <Function>      DSqliteUtil
 <Description>   构造函数-构造一个数据库处理类对象
 <Author>
 <Input>         null
 <Return>        DSqliteUtil        Description:返回一个数据库处理类对象
 <Note>          null
*************************************************************************/
DSqliteUtil::DSqliteUtil(const QString &strDatabase)
    : m_strDatabase(strDatabase)
    , m_query(nullptr)
{
    QDir dbdir(QDir::homePath() + "/.local/share/deepin/deepin-font-manager/");
    if (!dbdir.exists()) {
        dbdir.mkdir(QDir::homePath() + "/.local/share/deepin/deepin-font-manager/");
        qDebug() << __FUNCTION__ << QDir::homePath() + "/.local/share/deepin/deepin-font-manager/";
    }

    createConnection(m_strDatabase);
    createTable();
}

/*************************************************************************
 <Function>      ~DSqliteUtil
 <Description>   析构函数-析构数据库处理类对象
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DSqliteUtil::~DSqliteUtil()
{
    if (m_query) {
        delete m_query;
    }

    m_db.close();
    QSqlDatabase::removeDatabase("font_manager");
}

/*************************************************************************
 <Function>      createConnection
 <Description>   创建数据库连接
 <Author>
 <Input>
    <param1>     database        Description:数据库连接字符串
 <Return>        bool            Description:返回数据库连接结果
 <Note>          null
*************************************************************************/
bool DSqliteUtil::createConnection(const QString &database)
{
    QStringList drivers = QSqlDatabase::drivers();
    qDebug() << drivers;
    if (!drivers.contains("QSQLITE")) {
        qDebug() << "no sqlite driver!";
        drivers.clear();
        return false;
    }
    drivers.clear();

    //与数据库建立连接
    if (QSqlDatabase::contains("font_manager")) {
        m_db = QSqlDatabase::database("font_manager");
    } else {
        m_db = QSqlDatabase::addDatabase("QSQLITE", "font_manager");
    }
    //设置数据库名
    m_db.setDatabaseName(database);
    //打开数据库
    if (!m_db.open()) {
        qDebug() << "Open database failed!";
        return false;
    } else {
        qDebug() << "Open database success!";
        return true;
    }
}

/*************************************************************************
 <Function>      createTable
 <Description>   创建数据库表
 <Author>
 <Input>         null
 <Return>        bool            Description:返回执行结果
 <Note>          null
*************************************************************************/
bool DSqliteUtil::createTable()
{
    if (!m_db.isOpen()) {
        createConnection(m_strDatabase);
    }

    m_query = new QSqlQuery(m_db);

    QString createTableSql =
        "create table if not exists t_fontmanager(\
fontId INTEGER PRIMARY KEY AUTOINCREMENT,\
fontName TEXT,\
fontSize INTEGER,\
fontPreview TEXT,\
isEnabled TINYINT,\
isCollected TINYINT,\
filePath TEXT,\
familyName TEXT,\
styleName TEXT,\
type TEXT,\
version TEXT,\
copyright TEXT,\
description TEXT,\
sysVersion TEXT, \
isInstalled TINYINT,\
isError TINYINT,\
fullname TEXT, \
psname TEXT, \
trademark TEXT, \
isChineseFont TINYINT,\
isMonoSpace TINYINT)";

    if (!m_query->exec(createTableSql)) {
        qDebug() << "create table failed!";
        finish();
        return false;
    } else {
        finish();
        qDebug() << "create table sucess!";
        return true;
    }
}

/*************************************************************************
 <Function>      addRecord
 <Description>   向数据库中增加数据
 <Author>
 <Input>
    <param1>     data            Description:插入数据
    <param2>     table_name      Description:表名
 <Return>        bool            Description:返回执行结果
 <Note>          null
*************************************************************************/
bool DSqliteUtil::addRecord(QMap<QString, QString> data, const QString &table_name)
{
    QString sql = "insert into " + table_name + "(";
    QString values = " values(";
    QMutexLocker m_locker(&mutex);
    for (QMap<QString, QString>::const_iterator it = data.constBegin(); it != data.constEnd();
            it++) {
        sql += it.key() + ", ";
        QString escapeValue = it.value();
        escapeValue = escapeValue.replace("'", "''");
        values += "'" + escapeValue + "', ";
    }
    sql.chop(2);
    values.chop(2);
    sql += ")";
    values += ")";
    sql += values;
    qDebug() << sql;
    m_query->prepare(sql);

    if (!m_query->exec()) {
        finish();
        qDebug() << "add data failed!";
        return false;
    } else {
        finish();
        qDebug() << "add data success!";
        return true;
    }
}

/*************************************************************************
 <Function>      delRecord
 <Description>   删除记录
 <Author>
 <Input>
    <param1>     where           Description:位置索引
    <param2>     table_name      Description:表名
 <Return>        bool            Description:返回执行结果
 <Note>          null
*************************************************************************/
bool DSqliteUtil::delRecord(QMap<QString, QString> where, const QString &table_name)
{
    QString sql = "delete from ";
    sql += table_name;
    sql += " where ";
    QMutexLocker m_locker(&mutex);
    for (QMap<QString, QString>::const_iterator it = where.constBegin(); it != where.constEnd();
            it++) {
        //转义字符 ' -> ''
        QString value = it.value();
        value = value.replace("'", "''");
        sql += it.key() + "=";
        sql += "'" + value + "'";
        sql += " and ";
    }
    sql.chop(5);

    qDebug() << sql;
    m_query->prepare(sql);

    if (!m_query->exec()) {
        finish();
        qDebug() << "del data failed!";
        return false;
    } else {
        finish();
        qDebug() << "del data success!";
        return true;
    }
}

/*************************************************************************
 <Function>      updateRecord
 <Description>   修改数据库记录
 <Author>
 <Input>
    <param1>     where           Description:位置索引
    <param2>     data            Description:更新数据
    <param3>     table_name      Description:表名
 <Return>        bool            Description:返回执行结果
 <Note>          null
*************************************************************************/
bool DSqliteUtil::updateRecord(QMap<QString, QString> where, QMap<QString, QString> data,
                               const QString &table_name)
{
    QString sql = "update " + table_name + " set ";
    QMutexLocker m_locker(&mutex);
    for (QMap<QString, QString>::const_iterator it = data.constBegin(); it != data.constEnd();
            it++) {
        sql += it.key() + "=";
        sql += "'" + it.value() + "',";
    }
    sql.chop(1);
    sql += " where ";
    for (QMap<QString, QString>::const_iterator it = where.constBegin(); it != where.constEnd();
            it++) {
        sql += it.key() + "=";
        sql += "'" + it.value() + "'";
    }
    qDebug() << sql;
    m_query->prepare(sql);

    if (!m_query->exec()) {
        finish();
        qDebug() << "update data failed!";
        return false;
    } else {
        finish();
        qDebug() << "update data success!";
        return true;
    }
}

/*************************************************************************
 <Function>      findAllRecords
 <Description>   查找所有表
 <Author>
 <Input>
    <param1>     key             Description:查询内容
    <param2>     row             Description:表名键
    <param3>     table_name      Description:表名值
 <Return>        bool            Description:返回执行结果
 <Note>          null
*************************************************************************/
bool DSqliteUtil::findAllRecords(const QList<QString> &key, QList<QMap<QString, QString> > &row, const QString &table_name)
{
    QString sql = "select \
fontId,\
fontName,\
isEnabled,\
isCollected,\
filePath,\
familyName,\
styleName,\
type,\
version,\
copyright,\
description,\
sysVersion,\
isInstalled,\
isError,\
isChineseFont,\
isMonoSpace,\
fullname,\
psname,\
trademark, \
fontPreview from " + table_name;

    QMutexLocker m_locker(&mutex);
    m_query->prepare(sql);

    if (m_query->exec()) {
        while (m_query->next()) {
            QMap<QString, QString> mapRow;
            int columnLen = key.size();
            for (int i = 0; i < columnLen; i++) {
                mapRow.insert(key.at(i), m_query->value(i).toString());
            }
            row.append(mapRow);
            mapRow.clear();
        }
        finish();
        return true;
    }
    finish();
    return false;
}

/*************************************************************************
 <Function>      findRecords
 <Description>   查找所有记录
 <Author>
 <Input>
    <param1>     key             Description:查询内容
    <param2>     row             Description:传入参数,存放查询到的记录的容器
    <param3>     table_name      Description:表名值
 <Return>        bool            Description:返回执行结果
 <Note>          null
*************************************************************************/
bool DSqliteUtil::findRecords(const QList<QString> &key, QList<QMap<QString, QString>> *row,
                              const QString &table_name)
{
    QString sql = "select ";
    int columnLen = key.size();
    QMutexLocker m_locker(&mutex);
    for (int i = 0; i < columnLen; i++) {
        sql += key.at(i);
        sql += ",";
    }
    sql.chop(1);
    sql += " from " + table_name;
    m_query->prepare(sql);
    if (m_query->exec()) {
        while (m_query->next()) {
            QMap<QString, QString> mapRow;
            for (int i = 0; i < columnLen; i++) {
                mapRow.insert(key.at(i), m_query->value(i).toString());
            }
            row->append(mapRow);
        }
        finish();
//        qDebug() << "find all data success!";
        return true;
    } else {
        finish();
//        qDebug() << "find all data failed!";
        return false;
    }
}

/*************************************************************************
 <Function>      findRecords
 <Description>   按条件查找
 <Author>
 <Input>
    <param1>     key             Description:查询内容
    <param2>     where           Description:判断条件
    <param3>     row             Description:传入参数,存放查询到的记录的容器
    <param4>     table_name      Description:表名
 <Return>        bool            Description:返回执行结果
 <Note>          null
*************************************************************************/
bool DSqliteUtil::findRecords(const QList<QString> &key, const QMap<QString, QString> &where,
                              QList<QMap<QString, QString>> *row, const QString &table_name)
{
    QString sql = "select ";
    int columnLen = key.size();
    QMutexLocker m_locker(&mutex);
    for (int i = 0; i < columnLen; i++) {
        sql += key.at(i);
        sql += ",";
    }
    sql.chop(1);
    sql += " from " + table_name;
    sql += " where ";
    for (QMap<QString, QString>::const_iterator it = where.constBegin(); it != where.constEnd();
            it++) {
        sql += it.key() + "=\"" + escapeString(it.value()) + "\" and ";
    }
    sql.chop(5);
//    qDebug() << sql;
    m_query->prepare(sql);

    if (m_query->exec()) {
        while (m_query->next()) {
            QMap<QString, QString> mapRow;
            for (int i = 0; i < columnLen; i++) {
                mapRow.insert(key.at(i), m_query->value(i).toString());
            }
            row->append(mapRow);
            mapRow.clear();
        }
        finish();
//        qDebug() << "find data by condition success!";
        return true;
    } else {
        finish();
//        qDebug() << "find data by condition failed!";
        return false;
    }
}

/*************************************************************************
 <Function>      getRecordCount
 <Description>   获取记录总数
 <Author>
 <Input>
    <param1>     table_name      Description:表名
 <Return>        bool            Description:返回记录总数
 <Note>          null
*************************************************************************/
int DSqliteUtil::getRecordCount(const QString &table_name)
{
    QString sql = "select count(1) from " + table_name;
//    qDebug() << sql;
    QMutexLocker m_locker(&mutex);
    m_query->prepare(sql);

    int resultCount = 0;
    if (m_query->exec()) {
        if (m_query->next()) {
            resultCount = m_query->value(0).toInt();
        }
    }
    finish();

    return resultCount;
}

/*************************************************************************
 <Function>      getInstalledFontsPath
 <Description>   获取安装字体路径列表
 <Author>
 <Input>         null
 <Return>        QStringList      Description:返回安装字体路径列表
 <Note>          null
*************************************************************************/
QStringList DSqliteUtil::getInstalledFontsPath()
{
    QString sql = "select filePath from t_fontmanager where isInstalled = 1";
    QStringList installedList;
    QMutexLocker m_locker(&mutex);
    m_query->prepare(sql);
    if (m_query->exec()) {
        while (m_query->next()) {
            installedList.append(m_query->value(0).toString());
        }
    }
    finish();
    return installedList;
}

/*************************************************************************
 <Function>      getMaxFontId
 <Description>   获取最大字体id
 <Author>
 <Input>
    <param1>     table_name      Description:表名
 <Return>        int             Description:返回最大字体id
 <Note>          null
*************************************************************************/
int DSqliteUtil::getMaxFontId(const QString &table_name)
{
    QString sql = "select max(fontId) from " + table_name;
    qDebug() << sql;
    QMutexLocker m_locker(&mutex);
    m_query->prepare(sql);

    int maxFontId = 0;
    if (m_query->exec()) {
        if (m_query->next()) {
            maxFontId = m_query->value(0).toInt();
        }
    }
    finish();

    return maxFontId;
}

/*************************************************************************
 <Function>      addFontInfo
 <Description>   批量添加字体信息
 <Author>
 <Input>
    <param1>     fontList        Description:字体列表
    <param2>     table_name      Description:表名
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DSqliteUtil::addFontInfo(const QList<DFontPreviewItemData> &fontList, const QString &table_name)
{
    if (fontList.isEmpty())
        return;

    QMutexLocker m_locker(&mutex);
    QString sql = "insert into " + table_name +
                  "(fontName, \
isEnabled, \
isCollected, \
isChineseFont, \
isMonoSpace, \
filePath, \
familyName, \
styleName, \
type, \
version, \
copyright, \
description, \
sysVersion, \
isInstalled, \
isError, \
fullname, \
psname, \
trademark, \
fontPreview) values( \
:fontName, \
:isEnabled, \
:isCollected, \
:isChineseFont, \
:isMonoSpace, \
:filePath, \
:familyName, \
:styleName, \
:type, \
:version, \
:copyright, \
:description, \
:sysVersion, \
:isInstalled, \
:isError, \
:fullname, \
:psname, \
:trademark, \
:fontPreview)";

    qDebug() << sql;
    m_query->prepare(sql);

    QVariantList fontNameList;
    QVariantList isEnabledList;
    QVariantList isCollectedList;
    QVariantList isChineseList;
    QVariantList isMonoSpaceList;
    QVariantList filePathList;
    QVariantList familyNameList;
    QVariantList styleNameList;
    QVariantList typeList;
    QVariantList versionList;
    QVariantList copyrightList;
    QVariantList descriptionList;
    QVariantList sysVersionList;
    QVariantList isInstalledList;
    QVariantList isErrorList;
    QVariantList fullnameList;
    QVariantList psnameList;
    QVariantList trademarkList;
    QVariantList sp3FamilyList;
    for (const DFontPreviewItemData &item : fontList) {
        fontNameList << escapeString(item.fontData.strFontName);
        isEnabledList << QString::number(item.fontData.isEnabled());
        isCollectedList << QString::number(item.fontData.isCollected());
        isChineseList << QString::number(item.fontData.isChinese());
        isMonoSpaceList << QString::number(item.fontData.isMonoSpace());
        filePathList << escapeString(item.fontInfo.filePath);
        familyNameList << escapeString(item.fontInfo.familyName);
        styleNameList << escapeString(item.fontInfo.styleName);
        typeList << escapeString(item.fontInfo.type);
        versionList << escapeString(item.fontInfo.version);
        copyrightList << escapeString(item.fontInfo.copyright);
        descriptionList << escapeString(item.fontInfo.description);
        sysVersionList << escapeString(item.fontInfo.sysVersion);
        isInstalledList << QString::number(item.fontInfo.isInstalled);
        isErrorList << QString::number(item.fontInfo.isError);
        fullnameList << escapeString(item.fontInfo.fullname);
        psnameList << escapeString(item.fontInfo.psname);
        trademarkList << escapeString(item.fontInfo.trademark);
        sp3FamilyList << escapeString(item.fontInfo.sp3FamilyName);
    }

    m_query->addBindValue(fontNameList);
    m_query->addBindValue(isEnabledList);
    m_query->addBindValue(isCollectedList);
    m_query->addBindValue(isChineseList);
    m_query->addBindValue(isMonoSpaceList);
    m_query->addBindValue(filePathList);
    m_query->addBindValue(familyNameList);
    m_query->addBindValue(styleNameList);
    m_query->addBindValue(typeList);
    m_query->addBindValue(versionList);
    m_query->addBindValue(copyrightList);
    m_query->addBindValue(descriptionList);
    m_query->addBindValue(sysVersionList);
    m_query->addBindValue(isInstalledList);
    m_query->addBindValue(isErrorList);
    m_query->addBindValue(fullnameList);
    m_query->addBindValue(psnameList);
    m_query->addBindValue(trademarkList);
    m_query->addBindValue(sp3FamilyList);

    if (!m_query->execBatch()) {
        qDebug() << __FUNCTION__ << "add data failed!" << m_query->lastError();
    } else {
        qDebug() << __FUNCTION__ << "true";
    }

    finish();
}

/*************************************************************************
 <Function>      deleteFontInfo
 <Description>   批量删除字体信息
 <Author>
 <Input>
    <param1>     fontList        Description:字体列表
    <param2>     table_name      Description:表名
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DSqliteUtil::deleteFontInfo(const QList<DFontPreviewItemData> &fontList, const QString &table_name)
{
    QMutexLocker m_locker(&mutex);

    QString sql;
    sql = "delete from " + table_name +
          " where filePath = :filePath";
    qDebug() << sql;
    m_query->prepare(sql);

    QVariantList filePathList;
    for (const DFontPreviewItemData &item : fontList) {
        if (item.fontInfo.filePath.isEmpty())
            continue;
        //转义字符 ' -> ''
        filePathList << escapeString(item.fontInfo.filePath);
    }
    m_query->addBindValue(filePathList);
    if (!m_query->execBatch()) {
        qDebug() << "del data failed!" << filePathList;
    } else {
        qDebug() << __FUNCTION__ << "succ";
    }
    filePathList.clear();
    finish();
}

/*************************************************************************
 <Function>      updateFontInfo
 <Description>   批量更新字体信息
 <Author>
 <Input>
    <param1>     fontList            Description:字体列表
    <param2>     table_name          Description:表名
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DSqliteUtil::updateFontInfo(const QList<DFontPreviewItemData> &fontList, const QString &key, const QString &table_name)
{
    if ((key != "isCollected" && key != "isEnabled") || fontList.isEmpty())
        return;
    QMutexLocker m_locker(&mutex);

    QString sql = "update " + table_name + " set " + key + " = ? where filePath = ?";
    qDebug() << sql;
    m_query->prepare(sql);

    QVariantList keyList;
    QVariantList filePathList;
    for (const DFontPreviewItemData &item : fontList) {
        if (key == "isEnabled") {
            keyList << QString::number(item.fontData.isEnabled());
        } else if (key == "isCollected") {
            keyList << QString::number(item.fontData.isCollected());
        }

        filePathList << escapeString(item.fontInfo.filePath);
    }
    m_query->addBindValue(keyList);
    m_query->addBindValue(filePathList);

    if (!m_query->execBatch()) {
        qDebug() << "update data failed!" << filePathList;
    } else {
        qDebug() << __FUNCTION__ << "true";
    }
    keyList.clear();
    filePathList.clear();
    finish();
}

/*************************************************************************
 <Function>      escapeString
 <Description>   字符串处理
 <Author>
 <Input>
    <param1>     str             Description:待处理字符串
 <Return>        QString         Description:返回处理结果字符串
 <Note>          null
*************************************************************************/
QString DSqliteUtil::escapeString(const QString &str)
{
    if (str.isEmpty() || str.isNull())
        return "";
    return str;
}

void DSqliteUtil::updateSP3FamilyName(const QList<DFontInfo> &fontList)
{
    QString sql = "select filePath from t_fontmanager where fontPreview is NULL and filePath not like \"%/usr/share/fonts/%\"";
    qDebug() << sql;
    QMutexLocker m_locker(&mutex);
    m_query->prepare(sql);

    QStringList fileList;
    if (m_query->exec()) {
        while (m_query->next()) {
            fileList << m_query->value(0).toString();
        }
    }
    finish();

    if (fileList.isEmpty())
        return;

    qDebug() << __FUNCTION__ << fileList.size();
    // update
    sql = "update t_fontmanager set fontPreview = ? where filePath = ?";
    qDebug() << sql;
    m_query->prepare(sql);

    QVariantList keyList;
    QVariantList filePathList;
    for (const DFontInfo &data : fontList) {
        if (fileList.contains(data.filePath)) {
            keyList << data.sp3FamilyName;
            filePathList << data.filePath;
        }
    }

    m_query->addBindValue(keyList);
    m_query->addBindValue(filePathList);
    if (!m_query->execBatch()) {
        qDebug() << __FUNCTION__ << "update data failed!" << filePathList;
    } else {
        qDebug() << __FUNCTION__ << "true";
    }
    finish();
}

/*************************************************************************
 <Function>      delAllRecords
 <Description>   删除所有记录
 <Author>
 <Input>
    <param1>     table_name      Description:表名
 <Return>        bool            Description:返回处理结果
 <Note>          null
*************************************************************************/
bool DSqliteUtil::delAllRecords(const QString &table_name)
{
    QString sql = "delete from " + table_name;
    qDebug() << sql;
    QMutexLocker m_locker(&mutex);
    m_query->prepare(sql);

    if (!m_query->exec()) {
        finish();
        qDebug() << "delete all records failed!";
        return false;
    } else {
        finish();
        qDebug() << "delete all records success!";
        return true;
    }
}

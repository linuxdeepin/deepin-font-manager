#include "dsqliteutil.h"

#include <QDir>

#include <DLog>

//#ifdef qDebug
//#undef qDebug
//#define qDebug while(false) QMessageLogger().noDebug
//#endif

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

DSqliteUtil::~DSqliteUtil()
{
    if (m_query) {
        delete m_query;
    }

    m_db.close();
}

bool DSqliteUtil::createConnection(const QString &database)
{
    QStringList drivers = QSqlDatabase::drivers();
    qDebug() << drivers;
    if (!drivers.contains("QSQLITE")) {
        qDebug() << "no sqlite driver!";
        return false;
    }

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

//创建数据库表
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
        return false;
    } else {
        qDebug() << "create table sucess!";
        return true;
    }
}

bool DSqliteUtil::createIndex()
{
    if (!m_db.isOpen()) {
        createConnection(m_strDatabase);
    }

    m_query = new QSqlQuery(m_db);

    QString createIndexSql = "CREATE INDEX idx_familyStyle ON t_fontmanager(familyName, styleName)";
    if (!m_query->exec(createIndexSql)) {
        qDebug() << "create index failed!";
        return false;
    } else {
        qDebug() << "create index sucess!";
        return true;
    }
}

//向数据库中增加数据
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
        qDebug() << "add data failed!";
        return false;
    } else {
        qDebug() << "add data success!";
        return true;
    }
}

//删除记录
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
        qDebug() << "del data failed!";
        return false;
    } else {
        qDebug() << "del data success!";
        return true;
    }
}

//修改数据库记录
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
        qDebug() << "update data failed!";
        return false;
    } else {
        qDebug() << "update data success!";
        return true;
    }
}

//查找所有记录
bool DSqliteUtil::findRecords(QList<QString> key, QList<QMap<QString, QString>> *row,
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
    qDebug() << sql;
    m_query->prepare(sql);
    if (m_query->exec()) {
        while (m_query->next()) {
            QMap<QString, QString> mapRow;
            for (int i = 0; i < columnLen; i++) {
                mapRow.insert(key.at(i), m_query->value(i).toString());
            }
            row->append(mapRow);
        }
        qDebug() << "find all data success!";
        return true;
    } else {
        qDebug() << "find all data failed!";
        return false;
    }
}

//按条件查找
bool DSqliteUtil::findRecords(QList<QString> key, QMap<QString, QString> where,
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
        sql += it.key() + "='" + it.value() + "' and ";
    }
    sql.chop(5);
    qDebug() << sql;
    m_query->prepare(sql);
    if (m_query->exec()) {
        while (m_query->next()) {
            QMap<QString, QString> mapRow;
            for (int i = 0; i < columnLen; i++) {
                mapRow.insert(key.at(i), m_query->value(i).toString());
            }
            row->append(mapRow);
        }
        qDebug() << "find data by condition success!";
        return true;
    } else {
        qDebug() << "find data by condition failed!";
        return false;
    }
}

int DSqliteUtil::getRecordCount(const QString &table_name)
{
    QString sql = "select count(1) from " + table_name;
    qDebug() << sql;
    QMutexLocker m_locker(&mutex);
    m_query->prepare(sql);

    int resultCount = 0;
    if (m_query->exec()) {
        if (m_query->next()) {
            resultCount = m_query->value(0).toInt();
        }
    }

    return resultCount;
}

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

    return maxFontId;
}

void DSqliteUtil::addFontInfo(const QList<DFontPreviewItemData> &fontList, const QString &table_name)
{
    if (fontList.isEmpty())
        return;

    QMutexLocker m_locker(&mutex);
    bool succ = true;
    QString sql = "insert into " + table_name + "(" +
"fontName, \
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
trademark) values( \
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
:trademark)";

    qDebug() << sql;
    m_query->prepare(sql);

    for (DFontPreviewItemData item : fontList) {
        m_query->bindValue(":fontName", escapeString(item.strFontName));
        m_query->bindValue(":isEnabled", escapeString(QString::number(item.isEnabled)));
        m_query->bindValue(":isCollected", escapeString(QString::number(item.isCollected)));
        m_query->bindValue(":isChineseFont", escapeString(QString::number(item.isChineseFont)));
        m_query->bindValue(":isMonoSpace", escapeString(QString::number(item.isMonoSpace)));
        m_query->bindValue(":filePath", escapeString(item.fontInfo.filePath));
        m_query->bindValue(":familyName", escapeString(item.fontInfo.familyName));
        m_query->bindValue(":styleName", escapeString(item.fontInfo.styleName));
        m_query->bindValue(":type", escapeString(item.fontInfo.type));
        m_query->bindValue(":version", escapeString(item.fontInfo.version));
        m_query->bindValue(":copyright", escapeString(item.fontInfo.copyright));
        m_query->bindValue(":description", escapeString(item.fontInfo.description));
        m_query->bindValue(":sysVersion", escapeString(item.fontInfo.sysVersion));
        m_query->bindValue(":isInstalled", escapeString(QString::number(item.fontInfo.isInstalled)));
        m_query->bindValue(":isError", escapeString(QString::number(item.fontInfo.isError)));
        m_query->bindValue(":fullname", escapeString(item.fontInfo.fullname));
        m_query->bindValue(":psname", escapeString(item.fontInfo.psname));
        m_query->bindValue(":trademark", escapeString(item.fontInfo.trademark));
        if (!m_query->exec()) {
            qDebug() << __FUNCTION__ << "add data failed!" << item.fontInfo.filePath << m_query->lastError();
            succ = false;
        }
        qDebug() << __FUNCTION__ << "sql " << m_query->lastQuery() << item.fontInfo.toString();
    }
    qDebug() << __FUNCTION__ << succ;
}

void DSqliteUtil::deleteFontInfo(const QList<DFontPreviewItemData> &fontList, const QString &table_name)
{
    QMutexLocker m_locker(&mutex);

    QString sql;
    sql = "delete from " + table_name +
            " where filePath = :filePath";
    qDebug() << sql;
    m_query->prepare(sql);

    for (DFontPreviewItemData item : fontList) {
        if (item.fontInfo.filePath.isEmpty())
            continue;
        //转义字符 ' -> ''
        m_query->bindValue(":filePath", escapeString(item.fontInfo.filePath));

        if (!m_query->exec()) {
            qDebug() << "del data failed!" << item.fontInfo.filePath << m_query->lastError();
        }
        qDebug() << __FUNCTION__ << "sql " << m_query->lastQuery() << item.fontInfo.toString();
    }
}

void DSqliteUtil::updateFontInfo(const QList<DFontPreviewItemData> &fontList, const QString &key, const QString &table_name)
{
    if ((key != "isCollected" && key != "isEnabled") || fontList.isEmpty())
        return;
    QMutexLocker m_locker(&mutex);

    QString sql = "update " + table_name + " set " + key + " = ? where filePath = ?";
    qDebug() << sql;
    m_query->prepare(sql);
    bool succ = true;

    for (DFontPreviewItemData item : fontList) {
        if (key == "isEnabled") {
            m_query->addBindValue(escapeString(QString::number(item.isEnabled)));
        } else if (key == "isCollected") {
            m_query->addBindValue(escapeString(QString::number(item.isCollected)));
        }
        m_query->addBindValue(escapeString(item.fontInfo.filePath));

        if (!m_query->exec()) {
            succ = false;
            qDebug() << "update data failed!" << item.fontInfo.filePath << m_query->lastError();
        }
    }
    qDebug() << __FUNCTION__ << succ;
}

QString DSqliteUtil::escapeString(const QString &str)
{
    QString escapeStr = str;
    escapeStr = escapeStr.replace("'", "''");
//    escapeStr = "'" + escapeStr + "'";
    return escapeStr;
}

bool DSqliteUtil::delAllRecords(const QString &table_name)
{
    QString sql = "delete from " + table_name;
    qDebug() << sql;
    QMutexLocker m_locker(&mutex);
    m_query->prepare(sql);

    if (!m_query->exec()) {
        qDebug() << "delete all records failed!";
        return false;
    } else {
        qDebug() << "delete all records success!";
        return true;
    }
}

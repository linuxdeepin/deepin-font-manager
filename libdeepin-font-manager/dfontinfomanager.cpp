/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
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

#include "dfontinfomanager.h"
#include "dfmdbmanager.h"
#include "dfreetypeutil.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFontDatabase>
#include <QProcess>

#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include <iconv.h>

#include <DApplication>

DWIDGET_USE_NAMESPACE

#include FT_FREETYPE_H
#include FT_TYPE1_TABLES_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H


static QList<DFontInfo> dataList;
static DFontInfoManager *INSTANCE = 0;

inline bool isSystemFont(QString filePath)
{
    return filePath.contains("/.local/share/fonts") ? false : true;
}

QString convertToUtf8(char *content, int len)
{
    QString convertedStr = "";

    std::size_t inputBufferSize = len;
    std::size_t outputBufferSize = inputBufferSize * 4;
    char *outputBuffer = new char[outputBufferSize];
    char *backupPtr = outputBuffer;

    // UTF16BE to UTF8.
    iconv_t code = iconv_open("UTF-8", "UTF-16BE");
    std::size_t retVal = iconv(code, &content, &inputBufferSize, &outputBuffer, &outputBufferSize);
    std::size_t actuallyUsed = outputBuffer - backupPtr;

    convertedStr = QString::fromUtf8(QByteArray(backupPtr, actuallyUsed));
    iconv_close(code);

    delete[] backupPtr;
    return convertedStr;
}

DFontInfoManager *DFontInfoManager::instance()
{
    if (!INSTANCE) {
        INSTANCE = new DFontInfoManager;
    }

    return INSTANCE;
}

DFontInfoManager::DFontInfoManager(QObject *parent)
    : QObject(parent)
{
    //Should not be called in constructor
    //refreshList();
}

DFontInfoManager::~DFontInfoManager() {}

void DFontInfoManager::refreshList()
{
    if (!dataList.isEmpty()) {
        dataList.clear();
    }

    for (auto path : getAllFontPath()) {
        DFontInfo fontInfo = getFontInfo(path);
        fontInfo.isSystemFont = isSystemFont(path);
        dataList << fontInfo;
    }
}

QStringList DFontInfoManager::getAllFontPath() const
{
    QStringList pathList;
    QProcess *process = new QProcess;

    process->start("fc-list", QStringList() << ":" << "file");
    process->waitForFinished(-1);

    QString output = process->readAllStandardOutput();
    QStringList lines = output.split(QChar('\n'));
    process->deleteLater();

    for (QString line : lines) {
        QString filePath = line.remove(QChar(':')).simplified();
        if (filePath.length() > 0) {
            pathList << filePath;
        }
    }

    return pathList;
}

QStringList DFontInfoManager::getAllChineseFontPath() const
{
    QStringList pathList;
    QProcess *process = new QProcess;
    process->start("fc-list", QStringList() << ":lang=zh");
    process->waitForFinished(-1);

    QString output = process->readAllStandardOutput();
    QStringList lines = output.split(QChar('\n'));
    process->deleteLater();

    for (QString line : lines) {
        QString filePath = line.split(QChar(':')).first().simplified();
        if (filePath.length() > 0) {
            pathList << filePath;
        }
    }

    return pathList;
}


QStringList DFontInfoManager::getAllMonoSpaceFontPath() const
{
    QStringList pathList;
    QProcess *process = new QProcess;
    process->start("fc-list", QStringList() << ":spacing=mono");
    process->waitForFinished(-1);

    QString output = process->readAllStandardOutput();
    QStringList lines = output.split(QChar('\n'));
    process->deleteLater();

    for (QString line : lines) {
        QString filePath = line.split(QChar(':')).first().simplified();
        if (filePath.length() > 0) {
            pathList << filePath;
        }
    }

    return pathList;
}

QString DFontInfoManager::getInstalledFontPath(const DFontInfo &info)
{
    const QList<DFontInfo> famList = dataList;
    QString filePath = nullptr;

    for (const auto &famItem : famList) {
        if (info.familyName == famItem.familyName && info.styleName == famItem.styleName) {
            filePath = famItem.filePath;
            break;
        }
    }

    return filePath;
}

QStringList DFontInfoManager::getInstalledFontPath(const QStringList &fileList)
{
    DFMDBManager *dbManager = DFMDBManager::instance();
    QList<DFontPreviewItemData> allFontInfo = dbManager->getAllFontInfo();
    QStringList outlist;

    for (QString filePath : fileList) {
        QStringList list = getFamilyStyleName(filePath);
        if (list.isEmpty() || list.size() < 2)
            continue;

        for (const auto &famItem : allFontInfo) {
            if (list[0].startsWith(famItem.fontInfo.familyName) && list[1] == famItem.fontInfo.styleName) {
                outlist << famItem.fontInfo.filePath;
                break;
            }
        }
    }

    return outlist;
}

QString DFontInfoManager::getFontType(const QString &filePath)
{
    const QFileInfo fileInfo(filePath);
    const QString suffix = fileInfo.suffix().toLower();

    if (suffix == "ttf" || suffix == "ttc") {
        return "TrueType";
    } else if (suffix == "otf") {
        return "OpenType";
    } else {
        return DApplication::translate("FontDetailDailog", "Unknown");
    }
}

QStringList DFontInfoManager::getFamilyStyleName(const QString &filePath)
{
    QStringList ret;
    int appFontId = QFontDatabase::addApplicationFont(filePath);
    QStringList fontFamilyList = QFontDatabase::applicationFontFamilies(appFontId);
    QString fontFamily;
    QString styleName;
    if (fontFamilyList.size() > 0) {
        fontFamily = QString(fontFamilyList.first().toLocal8Bit()).trimmed();
    }

    FT_Library m_library = nullptr;
    FT_Face m_face = nullptr;

    FT_Init_FreeType(&m_library);
    FT_Error error = FT_New_Face(m_library, filePath.toUtf8().constData(), 0, &m_face);

    if (error != 0) {
        return ret;
    }

    if (fontFamily.isEmpty())
        fontFamily = QString::fromUtf8(DFreeTypeUtil::getFontFamilyName(m_face)).trimmed();

    if (fontFamily.isEmpty())
        fontFamily = QString::fromLatin1(m_face->family_name).trimmed();

    styleName = QString::fromLatin1(m_face->style_name);

    ret << fontFamily << styleName;

    return ret;
}

DFontInfo DFontInfoManager::getFontInfo(const QString &filePath)
{
    DFontInfo fontInfo;
    fontInfo.isSystemFont = isSystemFont(filePath);

    FT_Library m_library = 0;
    FT_Face m_face = 0;

    FT_Init_FreeType(&m_library);
    FT_Error error = FT_New_Face(m_library, filePath.toUtf8().constData(), 0, &m_face);

    if (error != 0) {
        qDebug() << __FUNCTION__ << " error " << error;
        fontInfo.isError = true;
        FT_Done_Face(m_face);
        FT_Done_FreeType(m_library);
        return fontInfo;
    }

    // get the basic data.
    fontInfo.isError = false;
    fontInfo.filePath = filePath;

    int appFontId = QFontDatabase::addApplicationFont(filePath);
    QStringList fontFamilyList = QFontDatabase::applicationFontFamilies(appFontId);
    if (fontFamilyList.size() > 0) {
        QString fontFamily = QString(fontFamilyList.first().toLocal8Bit());
        fontInfo.familyName = fontFamily;
    }

    if (fontInfo.familyName.trimmed().length() < 1) {
        fontInfo.familyName = QString::fromUtf8(DFreeTypeUtil::getFontFamilyName(m_face));
    }
    if (fontInfo.familyName.trimmed().length() < 1) {
        fontInfo.familyName = QString::fromLatin1(m_face->family_name);
    }

    fontInfo.styleName = QString::fromLatin1(m_face->style_name);
    fontInfo.type = getFontType(filePath);

    if (FT_IS_SFNT(m_face)) {
        FT_SfntName sname;
        const int count = FT_Get_Sfnt_Name_Count(m_face);

        for (int i = 0; i < count; ++i) {
            if (FT_Get_Sfnt_Name(m_face, i, &sname) != 0) {
                continue;
            }

            // only handle the unicode names for US langid.
            if (!(sname.platform_id == TT_PLATFORM_MICROSOFT
                  && sname.encoding_id == TT_MS_ID_UNICODE_CS
                  && sname.language_id == TT_MS_LANGID_ENGLISH_UNITED_STATES)) {
                continue;
            }

            // QString content;
            // for (int i = 0; i != sname.string_len; ++i) {
            //     char ch = static_cast<char>(sname.string[i]);
            //     content.append(ch);
            // }

            switch (sname.name_id) {
            case TT_NAME_ID_COPYRIGHT:
                fontInfo.copyright = convertToUtf8((char *)sname.string, sname.string_len);
                fontInfo.copyright = fontInfo.copyright.simplified();
                break;

            case TT_NAME_ID_VERSION_STRING:
                fontInfo.version = convertToUtf8((char *)sname.string, sname.string_len);
                fontInfo.version = fontInfo.version.remove("Version").simplified();
                break;

            case TT_NAME_ID_DESCRIPTION:
                fontInfo.description = convertToUtf8((char *)sname.string, sname.string_len);
                fontInfo.description = fontInfo.description.simplified();
                break;
            default:
                break;
            }
        }
    }

    DFMDBManager *dbManager = DFMDBManager::instance();
    if (dbManager->getRecordCount() > 0) {
        fontInfo.sysVersion = fontInfo.version;
        if (!dbManager->isFontInfoExist(fontInfo)) {
            fontInfo.isInstalled = false;
        } else {
            fontInfo.isInstalled = true;
        }
    } else {
        fontInfo.isInstalled = isFontInstalled(fontInfo);
    }

    // destroy object.
    FT_Done_Face(m_face);
    FT_Done_FreeType(m_library);

    return fontInfo;
}

bool DFontInfoManager::isFontInstalled(DFontInfo data)
{
    const QList<DFontInfo> list = dataList;

    for (int i = 0; i < list.count(); ++i) {
        DFontInfo item = list.at(i);

        if (data == item) {
            data.sysVersion = item.version;
            return true;
        }
    }

    return false;
}

//检查数据库中是否装有 相同名不同后缀大小写 的文件
bool DFontInfoManager::checkDBFontSameName(const DFontInfo &info)
{
    QFileInfo fileInfo = QFileInfo(info.filePath);
    QString fileName = fileInfo.fileName();
    int nPoint = fileName.indexOf(".");
    QString fileNameNoSuffix = fileName.left(nPoint);    

    //从数据库获取所有字体文件名
    QStringList allFontName = getAllFontName();

    const QString fileSuffixUp = fileInfo.suffix().toUpper();
    const QString fileSuffixLow = fileInfo.suffix().toLower();

    if (fileSuffixLow == fileInfo.suffix()) {
        if (allFontName.contains(fileNameNoSuffix + "." + fileSuffixUp)) {
            return true;
        }
    } else if (fileSuffixUp == fileInfo.suffix()) {
        if (allFontName.contains(fileNameNoSuffix + "." + fileSuffixLow)) {
            return true;
        }
    }
    return false;
}

// 从数据库获取所有字体文件名，并返回一个 QStringList
QStringList DFontInfoManager::getAllFontName() const
{
    QStringList allFileNameList;
    QFileInfo fileInfo;
    DFMDBManager *dbManager = DFMDBManager::instance();
    QList<DFontPreviewItemData> allFontInfo = dbManager->getAllFontInfo();

    for (int var = 0; var < allFontInfo.count(); ++var) {
        fileInfo = QFileInfo(allFontInfo[var].fontInfo.filePath);
        QString fileName = fileInfo.fileName();
        allFileNameList << fileName;
    }
    return allFileNameList;
}

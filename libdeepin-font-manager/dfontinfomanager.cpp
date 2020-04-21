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
static DFontInfoManager *INSTANCE = nullptr;

inline bool isSystemFont(QString filePath)
{
    if (filePath.contains("/usr/share/fonts/")) {
        return true;
    } else {
        return false;
    }
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
        DFontInfo fontInfo = getFontInfo(path, true);
        fontInfo.isSystemFont = isSystemFont(path);
        dataList << fontInfo;
    }
}

QStringList DFontInfoManager::getAllFontPath() const
{
    QStringList pathList;
    QProcess process;

    process.start("fc-list", QStringList() << ":" << "file");
    process.waitForFinished(-1);

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split(QChar('\n'));

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
    QProcess process;
    process.start("fc-list", QStringList() << ":lang=zh");
    process.waitForFinished(-1);

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split(QChar('\n'));

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
    QProcess process;
    process.start("fc-list", QStringList() << ":spacing=mono");
    process.waitForFinished(-1);

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split(QChar('\n'));

    for (QString line : lines) {
        QString filePath = line.split(QChar(':')).first().simplified();
        if (filePath.length() > 0) {
            pathList << filePath;
        }
    }

    return pathList;
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

DFontInfo DFontInfoManager::getFontInfo(const QString &filePath, bool force)
{
//    if (!force && m_fontInfoMap.contains(filePath)) {
//        return m_fontInfoMap.value(filePath);
//    } else if (!force) {
//        qDebug() << __FUNCTION__ << " not found " << filePath;
//    }

    FT_Library m_library = nullptr;
    FT_Face m_face = nullptr;

    DFontInfo fontInfo;
    fontInfo.isSystemFont = isSystemFont(filePath);

//    FT_Library m_library = 0;
//    FT_Face m_face = 0;

    if (m_library == nullptr)
        FT_Init_FreeType(&m_library);

    FT_Error error = 0;
    if (m_face == nullptr)
        error = FT_New_Face(m_library, filePath.toUtf8().constData(), 0, &m_face);

    if (error != 0) {
        qDebug() << __FUNCTION__ << " error " << error;
        fontInfo.isError = true;
        FT_Done_Face(m_face);
        m_face = nullptr;
        FT_Done_FreeType(m_library);
        m_library = nullptr;
        //try again
        error = FT_Init_FreeType(&m_library);
        if (error != 0) {
            FT_Done_FreeType(m_library);
            m_library = nullptr;
            return fontInfo;
        }
        error = FT_New_Face(m_library, filePath.toUtf8().constData(), 0, &m_face);
        if (error != 0) {
            FT_Done_Face(m_face);
            m_face = nullptr;

            FT_Done_FreeType(m_library);
            m_library = nullptr;
            return fontInfo;
        }
    }

    // get the basic data.
    fontInfo.isError = false;
    fontInfo.filePath = filePath;

//    int appFontId = QFontDatabase::addApplicationFont(filePath);
//    QStringList fontFamilyList = QFontDatabase::applicationFontFamilies(appFontId);
//    if (fontFamilyList.size() > 0) {
//        QString fontFamily = QString(fontFamilyList.first().toLocal8Bit());
//        fontInfo.familyName = fontFamily;
//    }

//    if (fontInfo.familyName.trimmed().length() < 1) {
//        fontInfo.familyName = QString::fromUtf8(DFreeTypeUtil::getFontFamilyName(m_face));
//    }
//    if (fontInfo.familyName.trimmed().length() < 1) {
//        fontInfo.familyName = QString::fromLatin1(m_face->family_name);
//    }

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
            if (sname.language_id == 0) {
                continue;
            }

            // QString content;
            // for (int i = 0; i != sname.string_len; ++i) {
            //     char ch = static_cast<char>(sname.string[i]);
            //     content.append(ch);
            // }

            switch (sname.name_id) {
            case TT_NAME_ID_COPYRIGHT:
                fontInfo.copyright = convertToUtf8((char *)sname.string, sname.string_len).simplified();
                break;

            case TT_NAME_ID_VERSION_STRING:
                fontInfo.version = convertToUtf8((char *)sname.string, sname.string_len);
                fontInfo.version = fontInfo.version.remove("Version").simplified();
                break;

            case TT_NAME_ID_DESCRIPTION:
                fontInfo.description = convertToUtf8((char *)sname.string, sname.string_len).simplified();
                break;

            case TT_NAME_ID_FULL_NAME:
                fontInfo.fullname = convertToUtf8((char *)sname.string, sname.string_len).simplified();
                break;

            case TT_NAME_ID_TRADEMARK:
                fontInfo.trademark = convertToUtf8((char *)sname.string, sname.string_len).simplified();
                break;

            case TT_NAME_ID_PS_NAME:
                fontInfo.psname = convertToUtf8((char *)sname.string, sname.string_len).simplified();
                break;

            default:
                break;
            }
        }
    }

    if (!fontInfo.fullname.isEmpty()) {
        fontInfo.familyName = fontInfo.fullname.replace(QRegExp(QString(" " + fontInfo.styleName + "$")), "");
    }
    if (fontInfo.familyName.trimmed().length() < 1) {
        fontInfo.familyName = QString::fromUtf8(DFreeTypeUtil::getFontFamilyName(m_face));
    }
    if (fontInfo.familyName.trimmed().length() < 1) {
        fontInfo.familyName = QString::fromLatin1(m_face->family_name);
    }

    // destroy object.
    FT_Done_Face(m_face);
    m_face = nullptr;
//    FT_Done_FreeType(m_library);

    DFMDBManager *dbManager = DFMDBManager::instance();
    if (dbManager->getRecordCount() > 0) {
        fontInfo.sysVersion = fontInfo.version;
        if (dbManager->isFontInfoExist(fontInfo).isEmpty()) {
            fontInfo.isInstalled = false;
        } else {
            fontInfo.isInstalled = true;
        }
    } else {
        fontInfo.isInstalled = isFontInstalled(fontInfo);
    }

//    if (force) {
//        fontInfo.filePath = getInstFontPath(filePath, fontInfo.familyName);
//        m_fontInfoMap.insert(fontInfo.filePath, fontInfo);
//        m_fontInfoMap.insert(filePath, fontInfo);
////        qDebug() << __FUNCTION__ << " insert " << fontInfo.filePath;
//    }

    return fontInfo;
}

QString DFontInfoManager::getInstFontPath(const QString &originPath, const QString &familyName)
{
    if (isSystemFont(originPath) || originPath.contains("/.local/share/fonts"))
        return originPath;

    const QFileInfo info(originPath);
    QString dirName = familyName;

    if (dirName.isEmpty()) {
        dirName = info.baseName();
    }

    const QString sysDir = QDir::homePath() + "/.local/share/fonts";

    QString target = QString("%1/%2/%3").arg(sysDir).arg(dirName).arg(info.fileName());
    return target;
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

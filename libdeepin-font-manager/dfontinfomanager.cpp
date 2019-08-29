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
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcess>

#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include <iconv.h>

#include FT_FREETYPE_H
#include FT_TYPE1_TABLES_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H

static QList<DFontInfo *> dataList;
static DFontInfoManager *INSTANCE = 0;

inline bool isSystemFont(QString filePath)
{
    return filePath.contains("/usr/share/fonts/deepin-font-install") ? false : true;
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
    refreshList();
}

DFontInfoManager::~DFontInfoManager() {}

void DFontInfoManager::refreshList()
{
    if (!dataList.isEmpty()) {
        qDeleteAll(dataList.begin(), dataList.end());
        dataList.clear();
    }

    for (auto path : getAllFontPath()) {
        DFontInfo *fontInfo = getFontInfo(path);
        fontInfo->isSystemFont = isSystemFont(path);
        dataList << fontInfo;
    }
}

QStringList DFontInfoManager::getAllFontPath() const
{
    QStringList pathList;
    QProcess *process = new QProcess;
    process->start("fc-list", QStringList() << ":"
                                            << "file");
    process->waitForFinished(-1);

    QString output = process->readAllStandardOutput();
    QStringList lines = output.split(QChar('\n'));
    process->deleteLater();

    for (QString line : lines) {
        pathList << line.remove(QChar(':')).simplified();
    }

    return pathList;
}

QString DFontInfoManager::getInstalledFontPath(DFontInfo *info)
{
    const QList<DFontInfo *> famList = dataList;
    QString filePath = nullptr;

    for (const auto &famItem : famList) {
        if (info->familyName == famItem->familyName && info->styleName == famItem->styleName) {
            filePath = famItem->filePath;
            break;
        }
    }

    return filePath;
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
        return "Unknown";
    }
}

DFontInfo *DFontInfoManager::getFontInfo(const QString &filePath)
{
    DFontInfo *fontInfo = new DFontInfo;
    fontInfo->isSystemFont = isSystemFont(filePath);

    FT_Library m_library = 0;
    FT_Face m_face = 0;

    FT_Init_FreeType(&m_library);
    FT_Error error = FT_New_Face(m_library, filePath.toUtf8().constData(), 0, &m_face);

    if (error != 0) {
        fontInfo->isError = true;
        FT_Done_Face(m_face);
        FT_Done_FreeType(m_library);
        return fontInfo;
    }

    // get the basic data.
    fontInfo->isError = false;
    fontInfo->filePath = filePath;
    fontInfo->familyName = QString::fromLatin1(m_face->family_name);
    fontInfo->styleName = QString::fromLatin1(m_face->style_name);
    fontInfo->type = getFontType(filePath);

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
                fontInfo->copyright = convertToUtf8((char *)sname.string, sname.string_len);
                fontInfo->copyright = fontInfo->copyright.simplified();
                break;

            case TT_NAME_ID_VERSION_STRING:
                fontInfo->version = convertToUtf8((char *)sname.string, sname.string_len);
                fontInfo->version = fontInfo->version.remove("Version").simplified();
                break;

            case TT_NAME_ID_DESCRIPTION:
                fontInfo->description = convertToUtf8((char *)sname.string, sname.string_len);
                fontInfo->description = fontInfo->description.simplified();
                break;
            default:
                break;
            }
        }
    }

    fontInfo->isInstalled = isFontInstalled(fontInfo);

    // destroy object.
    FT_Done_Face(m_face);
    FT_Done_FreeType(m_library);

    return fontInfo;
}

bool DFontInfoManager::isFontInstalled(DFontInfo *data)
{
    const QList<DFontInfo *> list = dataList;

    for (int i = 0; i < list.count(); ++i) {
        DFontInfo *item = list.at(i);

        if (*data == *item) {
            data->sysVersion = item->version;
            return true;
        }
    }

    return false;
}

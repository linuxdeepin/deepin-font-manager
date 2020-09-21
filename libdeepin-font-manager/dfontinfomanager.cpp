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
#include "dfontpreview.h"
#include "freetype/freetype.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFontDatabase>
#include <QProcess>
#include <QDateTime>

#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include <iconv.h>
#include <QDirIterator>

#include FT_FREETYPE_H
#include FT_TYPE1_TABLES_H
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H


static QList<DFontInfo> dataList;
static DFontInfoManager *INSTANCE = nullptr;

const QString FONT_USR_DIR = QDir::homePath() + "/.local/share/fonts/";
const QString FONT_SYSTEM_DIR = "/usr/share/fonts/";


/*************************************************************************
 <Function>      isSystemFont
 <Description>   判断是否为系统字体
 <Author>        null
 <Input>
    <param1>     filePath            Description:需要判断的字体路径
 <Return>        bool                Description:是否为系统字体
 <Note>          null
*************************************************************************/
inline bool isSystemFont(const QString &filePath)
{
    if (filePath.contains("/usr/share/fonts/")) {
        return true;
    } else {
        return false;
    }
}

/*************************************************************************
 <Function>      convertToUtf8
 <Description>   将字符串内容转换为utf8格式
 <Author>        null
 <Input>
    <param1>     content            Description:需要转换的内容
    <param2>     len                Description:转换长度
 <Return>        QString            Description:转换之后的内容
 <Note>          null
*************************************************************************/
QString convertToUtf8(unsigned char *content, unsigned int len)
{
    QString convertedStr = "";

    std::size_t inputBufferSize = static_cast<std::size_t>(len);
    std::size_t outputBufferSize = inputBufferSize * 4;
    char *outputBuffer = new char[outputBufferSize];
    char *backupPtr = outputBuffer;

    // UTF16BE to UTF8.
    iconv_t code = iconv_open("UTF-8", "UTF-16BE");
    iconv(code, reinterpret_cast<char **>(&content), &inputBufferSize, &outputBuffer, &outputBufferSize);
    int actuallyUsed = static_cast<int>(outputBuffer - backupPtr);

    convertedStr = QString::fromUtf8(QByteArray(backupPtr, actuallyUsed));
    iconv_close(code);

    delete[] backupPtr;
    return convertedStr;
}

QString getDefaultPreviewText(FT_Face face, qint8 &lang, int len = FTM_DEFAULT_PREVIEW_LENGTH);

/*************************************************************************
 <Function>      getDefaultPreviewText
 <Description>   获取默认的预览显示内容
 <Author>        null
 <Input>
    <param1>     face            Description:字体文件对应的FT_Face对象
    <param2>     lang            Description:传入参数,记录字体类型
    <param3>     len             Description:显示长度
 <Return>        QString         Description:预览显示内容
 <Note>          null
*************************************************************************/
QString getDefaultPreviewText(FT_Face face, qint8 &lang, int len)
{
    QString previewTxt;

    if (face == nullptr || face->num_charmaps == 0)
        return previewTxt;

    lang = FONT_LANG_NONE;
    QString language = QLocale::system().name();

    //first check chinese preview
    if (DFontPreview::checkFontContainText(face, FTM_DEFAULT_PREVIEW_CN_TEXT)) {
        lang = FONT_LANG_CHINESE;
        if (language.startsWith("zh_")) {
            return previewTxt;
        }
    }

    //not support chinese preview, than check english preview
    if (DFontPreview::checkFontContainText(face, FTM_DEFAULT_PREVIEW_EN_TEXT)) {
        lang |= FONT_LANG_ENGLISH;
        return previewTxt;
    }

    if (DFontPreview::checkFontContainText(face, FTM_DEFAULT_PREVIEW_DIGIT_TEXT)) {
        lang = FONT_LANG_DIGIT;
        return previewTxt;
    }

    return DFontPreview::buildCharlistForFace(face, len);
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

/*************************************************************************
 <Function>      refreshList
 <Description>   刷新字体链表
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontInfoManager::refreshList(const QStringList &allFontPathList)
{
    if (!dataList.isEmpty()) {
        dataList.clear();
    }

    for (auto &path : allFontPathList) {
        DFontInfo fontInfo = getFontInfo(path, true);
        fontInfo.isSystemFont = isSystemFont(path);
        dataList << fontInfo;
    }

    updateSP3FamilyName(dataList);
}

/*************************************************************************
 <Function>      getAllFontPath
 <Description>   获取所有的字体文件路径
 <Author>        null
 <Input>
    <param1>     isStartup              Description:是否为第一次启动
 <Return>        QStringList            Description:所有的字体文件路径
 <Note>          null
*************************************************************************/
QStringList DFontInfoManager::getAllFontPath(bool isStartup) const
{
#if 0
    QStringList pathList = getFonts(DFontInfoManager::All);
#endif
    QStringList pathList;
    QProcess process;

    process.start("fc-list", QStringList() << ":" << "file");
    process.waitForFinished(-1);

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split(QChar('\n'));
    for (QString &line : lines) {
        QString filePath = line.remove(QChar(':')).simplified();
        if (filePath.length() > 0 && !pathList.contains(filePath)) {
            pathList << filePath;
        }
    }
    //pathList << "/home/lx777/.local/share/fonts/Unifont Sample/unifontsample(1).ttf";

    /*qSort(pathList.begin(), pathList.end(), [](const QString & s1, const QString & s2) {
        return s1 < s2;
    });*/


//    QStringList dirlist = getDirPathOfSplDir(FONT_DIR);
//    foreach (QString str, dirlist) {
//        QStringList namelist = getFileNames(str);
//        for (int i = 0; i < namelist.count(); i++) {
//            QString filepath = str + "/" +  namelist.at(i);
//            if (!pathList.contains(filepath)) {
//                pathList << filepath;
//            }
//        }
//    }

    if (isStartup) {
        //系统字体文件
        QStringList systemfilelist = getFileNames(FONT_SYSTEM_DIR);
        for (const QString &str : systemfilelist) {
            if (!pathList.contains(str)) {
                pathList << str;
            }
        }
    }

    //用户字体文件
    QStringList usrfilelist = getFileNames(FONT_USR_DIR);
    for (const QString &str : usrfilelist) {
        if (!pathList.contains(str)) {
            pathList << str;
        }
    }

    return pathList;
}

/*************************************************************************
 <Function>      getFileNames
 <Description>   获取路径下的所有字体文件
 <Author>        null
 <Input>
    <param1>     path                   Description:路径
 <Return>        QStringList            Description:路径下的所有字体文件
 <Note>          null
*************************************************************************/
QStringList DFontInfoManager::getFileNames(const QString &path)const
{
    QStringList string_list;
    //判断路径是否存在
    QDir dir(path);
    if (!dir.exists()) {
        return string_list;
    }

    //获取所选文件类型过滤器
    QStringList filters;
    filters << "*.ttf" << "*.ttc" << "*.otf";

    //定义迭代器并设置过滤器
    QDirIterator dir_iterator(path, filters, QDir::Files | QDir::NoSymLinks, QDirIterator::Subdirectories);
    while (dir_iterator.hasNext()) {
        dir_iterator.next();
        QFileInfo file_info = dir_iterator.fileInfo();
        QString absolute_file_path = file_info.absoluteFilePath();
        string_list.append(absolute_file_path);
    }
    return string_list;
}

/*************************************************************************
 <Function>      getAllMonoSpaceFontPath
 <Description>   获得所有的中文字体
 <Author>        null
 <Input>
    <param1>     null                   Description:null
 <Return>        QStringList            Description:获取到的字体
 <Note>          null
*************************************************************************/
QStringList DFontInfoManager::getAllChineseFontPath() const
{
    QStringList pathList;
    QProcess process;
    process.start("fc-list", QStringList() << ":lang=zh");
    process.waitForFinished(-1);

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split(QChar('\n'));

    for (QString &line : lines) {
        QString filePath = line.split(QChar(':')).first().simplified();
        if (filePath.length() > 0) {
            pathList << filePath;
        }
    }

    return pathList;
#if 0
    return getFonts(Chinese);
#endif
}

/*************************************************************************
 <Function>      getAllMonoSpaceFontPath
 <Description>   获得所有的等款字体
 <Author>        null
 <Input>
    <param1>     null                   Description:null
 <Return>        QStringList            Description:获取到的字体
 <Note>          null
*************************************************************************/
QStringList DFontInfoManager::getAllMonoSpaceFontPath() const
{
    QStringList pathList;
    QProcess process;
    process.start("fc-list", QStringList() << ":spacing=mono");
    process.waitForFinished(-1);

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split(QChar('\n'));

    for (QString &line : lines) {
        QString filePath = line.split(QChar(':')).first().simplified();
        if (filePath.length() > 0) {
            pathList << filePath;
        }
    }
#if 0
    QStringList pathList = getFonts(MonoSpace);
#endif
    return pathList;
}

/*************************************************************************
 <Function>      getFontType
 <Description>   获取字体文件类型
 <Author>        null
 <Input>
    <param1>     filePath            Description:字体文件路径
 <Return>        QString             Description:字体文件类型
 <Note>          null
*************************************************************************/
QString DFontInfoManager::getFontType(const QString &filePath)
{
    const QFileInfo fileInfo(filePath);
    const QString suffix = fileInfo.suffix().toLower();

    if (suffix == "ttf" || suffix == "ttc") {
        return FONT_TTF;
    } else if (suffix == "otf") {
        return FONT_OTF;
    } else {
        return FONT_UNKNOWN;
    }
}

/*************************************************************************
 <Function>      getFontInfo
 <Description>   获取字体信息
 <Author>        null
 <Input>
    <param1>     filePath             Description:字体文件路径
 <Return>        DFontInfo            Description:该字体文件的字体信息
 <Note>          null
*************************************************************************/
DFontInfo DFontInfoManager::getFontInfo(const QString &filePath, bool withPreviewTxt)
{
    DFontInfo fontInfo;
    fontInfo.isSystemFont = isSystemFont(filePath);

    FT_Library library = nullptr;
    FT_Init_FreeType(&library);

    FT_Face face = nullptr;
    FT_Error error = FT_New_Face(library, filePath.toUtf8().constData(), 0, &face);

    if (error != 0) {
        qDebug() << __FUNCTION__ << " error " << error << filePath;
        fontInfo.isError = true;
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        return fontInfo;
    }

    // get the basic data.
    fontInfo.isError = false;
    fontInfo.filePath = filePath;

    fontInfo.styleName = QString::fromLatin1(face->style_name);

    fontInfo.type = getFontType(filePath);

    if (FT_IS_SFNT(face)) {
        FT_SfntName sname;
        const unsigned int count = FT_Get_Sfnt_Name_Count(face);

        for (unsigned int i = 0; i < count; ++i) {
            if (FT_Get_Sfnt_Name(face, i, &sname) != 0) {
                continue;
            }

            // only handle the unicode names for US langid.
            if (sname.language_id == 0) {
                continue;
            }

            switch (sname.name_id) {
            case TT_NAME_ID_COPYRIGHT:
                fontInfo.copyright = convertToUtf8(sname.string, sname.string_len).simplified();
                break;

            case TT_NAME_ID_VERSION_STRING:
                fontInfo.version = convertToUtf8(sname.string, sname.string_len);
                fontInfo.version = fontInfo.version.remove("Version").simplified();
                break;

            case TT_NAME_ID_DESCRIPTION:
                fontInfo.description = convertToUtf8(sname.string, sname.string_len).simplified();
                break;

            case TT_NAME_ID_FULL_NAME:
                fontInfo.fullname = convertToUtf8(sname.string, sname.string_len).simplified();
                break;

            case TT_NAME_ID_TRADEMARK:
                fontInfo.trademark = convertToUtf8(sname.string, sname.string_len).simplified();
                break;

            case TT_NAME_ID_PS_NAME:
                fontInfo.psname = convertToUtf8(sname.string, sname.string_len).simplified();
                break;

            default:
                break;
            }
        }
    }
    //compitable with SP2 update1 and previous versions
    if (!fontInfo.fullname.isEmpty())
        fontInfo.familyName = fontInfo.fullname.replace(QRegExp(QString(" " + fontInfo.styleName + "$")), "");

    if (fontInfo.familyName.trimmed().length() < 1) {
        fontInfo.familyName = QString::fromLatin1(face->family_name);
    }

    fontInfo.sp3FamilyName = QString::fromLatin1(face->family_name).trimmed();

    //default preview text
    if (withPreviewTxt) {
        if (fontInfo.familyName == "Noto Sans Grantha") {
            fontInfo.defaultPreview = getDefaultPreviewText(face, fontInfo.previewLang, INT_MAX);
        } else {
            fontInfo.defaultPreview = getDefaultPreviewText(face, fontInfo.previewLang);
        }
    }

    // destroy object.
    FT_Done_Face(face);
    face = nullptr;
    FT_Done_FreeType(library);
    library = nullptr;

    checkStyleName(fontInfo);

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

    return fontInfo;
}

/*************************************************************************
 <Function>      getDefaultPreview
 <Description>   获取字体初始的预览效果内容
 <Author>        null
 <Input>
    <param1>     filePath               Description:字体文件路径
    <param2>     preivewLang            Description:显示内容长度
 <Return>        QString                Description:字体初始的预览效果内容
 <Note>          null
*************************************************************************/
QString DFontInfoManager::getDefaultPreview(const QString &filePath, qint8 &preivewLang)
{
    FT_Library m_library = nullptr;
    FT_Face m_face = nullptr;
    QString defaultPreview;

    FT_Init_FreeType(&m_library);
    FT_Error error = FT_New_Face(m_library, filePath.toUtf8().constData(), 0, &m_face);

    if (error != 0) {
        qDebug() << __FUNCTION__ << " error " << error << filePath;
        FT_Done_Face(m_face);
        m_face = nullptr;
        FT_Done_FreeType(m_library);
        m_library = nullptr;
        return defaultPreview;
    }

    defaultPreview = getDefaultPreviewText(m_face, preivewLang);
    FT_Done_Face(m_face);
    FT_Done_FreeType(m_library);
    return defaultPreview;
}

/*************************************************************************
 <Function>      getCurrentFontFamily
 <Description>   获取当前使用字体的familyname
 <Author>        null
 <Input>
    <param1>     null                   Description:null
 <Return>        QStringList            Description:当前使用字体的familyname
 <Note>          null
*************************************************************************/
QStringList DFontInfoManager::getCurrentFontFamily()
{
    QStringList retStrList;
    QProcess process;

    process.start("fc-match");
    process.waitForFinished(-1);

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split(QChar('\n'));
    for (QString &line : lines) {
        retStrList = line.split(" \"");
        int index = 0;
        for (QString &fontStr : retStrList) {
            fontStr.remove(QChar('\"'));
            if (index == 0 && fontStr.endsWith(":"))
                fontStr.remove(":");
            index++;
        }
        if (!retStrList.isEmpty())
            break;
    }
#if 0
    FcPattern *pat = FcPatternCreate();
    if (!pat)
        return retStrList;

    FcConfigSubstitute(nullptr, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);
    FcFontSet *fs = FcFontSetCreate();
    if (!fs) {
        FcPatternDestroy(pat);
        FcFini();
        return retStrList;
    }

    FcResult result;
    FcPattern *match = FcFontMatch(nullptr, pat, &result);
    if (match)
        FcFontSetAdd(fs, match);
    FcPatternDestroy(pat);

    const FcChar8 *format = reinterpret_cast<const FcChar8 *>("%{=fcmatch}");

    for (int j = 0; j < fs->nfont; j++) {
        FcPattern *font = FcPatternFilter(fs->fonts[j], nullptr);
        FcChar8 *s = FcPatternFormat(font, format);

        if (s) {
            QString str = QString(reinterpret_cast<char *>(s));
            retStrList = str.split(" \"");
            int index = 0;
            for (QString &fontStr : retStrList) {
                fontStr.remove(QChar('\"'));
                if (index == 0 && fontStr.endsWith(":"))
                    fontStr.remove(":");
                index++;
            }
            FcStrFree(s);
        }
        qDebug() << __FUNCTION__ << retStrList;
        FcPatternDestroy(font);
        if (!retStrList.isEmpty())
            break;
    }
    FcFontSetDestroy(fs);
    FcFini();
#endif
    return retStrList;
}

/*************************************************************************
 <Function>      getFontPath
 <Description>   获取字体的路径信息
 <Author>        null
 <Input>
    <param1>     null
 <Return>        字体的路径
 <Note>          null
*************************************************************************/
QString DFontInfoManager::getFontPath()
{
    QString filePath;
    QStringList retStrList;
    QProcess process;

    process.start("fc-match -v |grep file");
    process.waitForFinished(-1);

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split(QChar('\n'));
    for (QString &line : lines) {
        line = line.simplified();
        if (!line.startsWith("file:"))
            continue;

        retStrList = line.split(" \"");
        for (QString &fontStr : retStrList) {
            int lastIndex = fontStr.lastIndexOf(QChar('\"'));
            if (lastIndex == -1)
                continue;

            fontStr.chop(fontStr.size() - lastIndex);
            if (!fontStr.isEmpty()) {
                return fontStr;
            }
        }
    }
    return filePath;
}

/*************************************************************************
 <Function>      getFontFamilyStyle
 <Description>   获取字体的family信息
 <Author>        null
 <Input>
    <param1>     filePah                Description:字体文件路径
 <Return>        QStringList            Description:获取到的信息
 <Note>          null
*************************************************************************/
QStringList DFontInfoManager::getFontFamilyStyle(const QString &filePah)
{
    QStringList fontFamilyList;
#if 0
    QProcess process;

    process.start("fc-list |grep " + filePah);
    process.waitForFinished(-1);

    QString output = process.readAllStandardOutput();
    QStringList lines = output.split(QChar('\n'));
    for (QString &line : lines) {
        qDebug() << __FUNCTION__ << line;
        QStringList retStrList = line.split(":");
        if (retStrList.size() != 3) {
            continue;
        }
        QString families = const_cast<const QString &>(retStrList.at(1)).simplified();
        fontFamilyList = families.split(",");
        for (QString &fontStr : fontFamilyList) {
            fontStr.remove(QChar('\\'));
        }

    }
#endif
    const FcChar8 *format = reinterpret_cast<const FcChar8 *>("%{=fclist}");
    FcObjectSet *os = FcObjectSetBuild(FC_FAMILY, FC_STYLE, FC_FILE, nullptr);
    FcPattern *pat = FcPatternCreate();
    FcFontSet *fs = FcFontList(nullptr, pat, os);

    if (os)
        FcObjectSetDestroy(os);
    if (pat)
        FcPatternDestroy(pat);

    if (fs) {
        for (int j = 0; j < fs->nfont; j++) {
            FcChar8 *s = FcPatternFormat(fs->fonts[j], format);
            if (s == nullptr)
                continue;

            QString str = QString(reinterpret_cast<char *>(s));
            QStringList retStrList = str.split(":");
            if (retStrList.size() != 3) {
                FcStrFree(s);
                continue;
            }
            QString fontpath = const_cast<const QString &>(retStrList.at(0));
            if (fontpath.simplified() == filePah) {
                qDebug() << __FUNCTION__ << " found " << filePah;
                QString families = const_cast<const QString &>(retStrList.at(1)).simplified();
                fontFamilyList = families.split(",");
                for (QString &fontStr : fontFamilyList) {
                    fontStr.remove(QChar('\\'));
                }

                FcStrFree(s);
                break;
            }  else {
                FcStrFree(s);
            }
        }
        FcFontSetDestroy(fs);
    }

//    FcFini();

    return fontFamilyList;
}

/*************************************************************************
 <Function>      getFonts
 <Description>   获取所需类型的字体
 <Author>        null
 <Input>
    <param1>     type                   Description:所需字体的类型
 <Return>        QStringList            Description:指定类型的字体
 <Note>          null
*************************************************************************/
QStringList DFontInfoManager::getFonts(DFontInfoManager::FontTYpe type) const
{
    QStringList fontList;
    const FcChar8 *format = reinterpret_cast<const FcChar8 *>("%{=fclist}");
    FcPattern *pat = nullptr;
    const FcChar8 *opt = nullptr;
    switch (type) {
    case Chinese:
        opt = reinterpret_cast<const FcChar8 *>(":lang=zh");
        break;
    case MonoSpace:
        opt = reinterpret_cast<const FcChar8 *>(":spacing=mono");
        break;
    case All:
        break;
    }

    if (opt != nullptr) {
        pat = FcNameParse(opt);
        if (pat == nullptr) {
            qDebug() << __FUNCTION__ << " err " << type;
            return fontList;
        }
    } else {
        pat = FcPatternCreate();
    }

    FcObjectSet *os = FcObjectSetBuild(FC_FILE, nullptr);
    FcFontSet *fs = FcFontList(nullptr, pat, os);

    if (os)
        FcObjectSetDestroy(os);
    if (pat)
        FcPatternDestroy(pat);

    if (fs) {
        for (int j = 0; j < fs->nfont; j++) {
            FcChar8 *s = FcPatternFormat(fs->fonts[j], format);
            if (s == nullptr)
                continue;

            QString str = QString(reinterpret_cast<char *>(s));
            str.remove(": ");
//            qDebug() << __FUNCTION__ << str;
            if (!fontList.contains(str) && !str.isEmpty())
                fontList << str;
            FcStrFree(s);
        }
        FcFontSetDestroy(fs);
    }

//    FcFini();
    return fontList;
}

/*************************************************************************
 <Function>      isFontInstalled
 <Description>   字体有没有被安装过
 <Author>        null
 <Input>
    <param1>     data            Description:需要处理的字体信息
 <Return>        bool            Description:有没有被安装过
 <Note>          null
*************************************************************************/
bool DFontInfoManager::isFontInstalled(DFontInfo &data)
{
    for (DFontInfo &item : dataList) {
        if (data == item) {
            data.sysVersion = item.version;
            return true;
        }
    }

    return false;
}

/*************************************************************************
 <Function>      getDefaultPreview
 <Description>   获取默认的字体预览效果现实内容
 <Author>        null
 <Input>
    <param1>     data            Description:需要处理的字体信息
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontInfoManager::getDefaultPreview(DFontInfo &data)
{
    for (DFontInfo &item : dataList) {
        if (data == item) {
            data.defaultPreview = item.defaultPreview;
            data.previewLang = item.previewLang;
            return;
        }
    }

    data.defaultPreview = getDefaultPreview(data.filePath, data.previewLang);
}

/**
* @brief DFontInfoManager::updateSP3FamilyName sp3的familyName兼容sp2 update1及之前的版本
* @param fontList 字体列表
* @param inFontList 是否使用字体列表，true：使用 false：不使用
* @return void
*/
void DFontInfoManager::updateSP3FamilyName(const QList<DFontInfo> &fontList, bool inFontList)
{
    if (fontList.isEmpty())
        return;
    //compitable with sp2 update1 and previous versions
    DFMDBManager::instance()->updateSP3FamilyName(fontList,  inFontList);
}

/*************************************************************************
 <Function>      checkStyleName
 <Description>   对某些特殊字体的stylename进行处理,使得显示内容规范
 <Author>        null
 <Input>
    <param1>     f               Description:需要处理的字体信息
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontInfoManager::checkStyleName(DFontInfo &f)
{
    QStringList str;
    str << "Regular" << "Bold" << "Light" << "Thin" << "ExtraLight" << "ExtraBold" << "Medium" << "DemiBold" << "Black"
        << "AnyStretch" << "UltraCondensed" << "ExtraCondensed" << "Condensed" << "SemiCondensed" << "Unstretched" << "SemiExpanded" << "Expanded"
        << "ExtraExpanded" << "UltraExpanded";
//有些字体文件因为不规范导致的stylename为空，通过psname来判断该字体的stylename。
    if (f.styleName.contains("?")) {
        if (f.psname != "") {
            if (f.psname.contains("Regular")) {
                f.styleName = "Regular";
            } else if (f.psname.contains("Bold")) {
                f.styleName = "Bold";
            } else if (f.psname.contains("Light")) {
                f.styleName = "Light";
            } else if (f.psname.contains("Thin")) {
                f.styleName = "Thin";
            } else if (f.psname.contains("ExtraLight")) {
                f.styleName = "ExtraLight";
            } else if (f.psname.contains("ExtraBold")) {
                f.styleName = "ExtraBold";
            } else if (f.psname.contains("Medium")) {
                f.styleName = "Medium";
            } else if (f.psname.contains("DemiBold")) {
                f.styleName = "DemiBold";
            } else if (f.psname.contains("AnyStretch")) {
                f.styleName = "AnyStretch";
            } else if (f.psname.contains("UltraCondensed")) {
                f.styleName = "UltraCondensed";
            } else if (f.psname.contains("ExtraCondensed")) {
                f.styleName = "ExtraCondensed";
            } else if (f.psname.contains("Condensed")) {
                f.styleName = "Condensed";
            } else if (f.psname.contains("SemiCondensed")) {
                f.styleName = "SemiCondensed";
            } else if (f.psname.contains("Unstretched")) {
                f.styleName = "Unstretched";
            } else if (f.psname.contains("SemiExpanded")) {
                f.styleName = "SemiExpanded";
            } else if (f.psname.contains("Expanded")) {
                f.styleName = "Expanded";
            } else if (f.psname.contains("ExtraExpanded")) {
                f.styleName = "ExtraExpanded";
            } else if (f.psname.contains("UltraExpanded")) {
                f.styleName = "UltraExpanded";
            } else {
                f.styleName = "Unknown";
            }
        }
//        //有些字体及其特殊，解析获得的信息中获取不到任何有关stylename的信息，对于这种字体，stylename设置为Unknown
//        if (!str.contains(f.styleName)) {
//            f.styleName = "Unknown";
//        }
    }
}

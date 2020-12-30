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

#ifndef DFONTINFO_H
#define DFONTINFO_H

#include <QObject>

const qint8 FONT_LANG_NONE = 0;
const qint8 FONT_LANG_CHINESE = 1;
const qint8 FONT_LANG_ENGLISH = 2;
const qint8 FONT_LANG_DIGIT = 4;

struct DFontInfo {
    QString filePath;
    QString familyName;
    QString styleName;
    QString type;
    QString version;
    QString copyright;
    QString description;
    QString sysVersion;
    //new font info
    QString fullname;
    QString psname;
    QString trademark;
    //add default preview text
    QString defaultPreview;
    //add since SP3 to be compitable with SP2 Update1 and previous versions
    //familyName no longer used since SP3 instead of sp3FamilyName
    //familyName is only kept to compatible with previous versions.
    QString sp3FamilyName;
    qint8 previewLang; //0: none; 1: Chinese; 2: English; others...


    bool isInstalled {false};
    bool isError {false};
    bool isSystemFont {true};  // Default is system font

    DFontInfo()
        : filePath("")
        , familyName("")
        , styleName("")
        , type("")
        , version("")
        , copyright("")
        , description("")
        , sysVersion("")
        , fullname("")
        , psname("")
        , trademark("")
        , defaultPreview("")
        , sp3FamilyName("")
        , previewLang(FONT_LANG_NONE)
        , isInstalled(false)
        , isError(false)
        , isSystemFont(true)

    {

    }

    DFontInfo(const QString &_filePath, const QString &_familyName, const QString &_styleName, const QString &_type,
              const QString &_version, const QString &_copyright, const QString &_desc, const QString &_sysVer,
              const QString &_fullname, const QString &_psname, const QString &_trademark, bool _isInstalled,
              bool _isError, bool _isSystemFont,  const QString &_sp3FamilyName)
        : filePath(_filePath)
        , familyName(_familyName)
        , styleName(_styleName)
        , type(_type)
        , version(_version)
        , copyright(_copyright)
        , description(_desc)
        , sysVersion(_sysVer)
        , fullname(_fullname)
        , psname(_psname)
        , trademark(_trademark)
        , defaultPreview("")
        , sp3FamilyName(_sp3FamilyName)
        , previewLang(FONT_LANG_NONE)
        , isInstalled(_isInstalled)
        , isError(_isError)
        , isSystemFont(_isSystemFont)

    {

    }

    DFontInfo(const DFontInfo &other)
        : filePath(other.filePath)
        , familyName(other.familyName)
        , styleName(other.styleName)
        , type(other.type)
        , version(other.version)
        , copyright(other.copyright)
        , description(other.description)
        , sysVersion(other.sysVersion)
        , fullname(other.fullname)
        , psname(other.psname)
        , trademark(other.trademark)
        , defaultPreview(other.defaultPreview)
        , sp3FamilyName(other.sp3FamilyName)
        , previewLang(other.previewLang)
        , isInstalled(other.isInstalled)
        , isError(other.isError)
        , isSystemFont(other.isSystemFont)

    {

    }

    bool operator==(const DFontInfo &info)
    {
        return info.familyName == familyName && info.styleName == styleName && info.fullname == fullname && info.filePath == filePath;
    }

    QString toString() const
    {
        return "FontInfo : " + familyName + ", " + styleName + ", psname = " + psname + ", trademark = " + trademark
               + ", fullname = " + fullname;
    }
};

class DFontInfoManager : public QObject
{
    Q_OBJECT

public:
    static DFontInfoManager *instance();
    explicit DFontInfoManager(QObject *parent = nullptr);
    ~DFontInfoManager();

    enum FontTYpe  {
        All,
        Chinese,
        MonoSpace,
    };

    void refreshList(const QStringList &allFontPathList);
    QStringList getAllFontPath(bool isStartup = false) const;
    QStringList getAllChineseFontPath() const;
    QStringList getAllMonoSpaceFontPath() const;
    QStringList getAllFclistPathList()const;
    QString getFontType(const QString &filePath);
    DFontInfo getFontInfo(const QString &filePath, bool withPreviewTxt = false);
    QString getDefaultPreview(const QString &filePath, qint8 &preivewLang);
    QStringList getCurrentFontFamily();
    QString getFontPath();
    QStringList getFontFamilyStyle(const QString &filePah);
    QStringList getFonts(FontTYpe type) const;
    bool isFontInstalled(DFontInfo &data);
    void getDefaultPreview(DFontInfo &data);
    void updateSP3FamilyName(const QList<DFontInfo> &fontList, bool inFontList = false);
    void checkStyleName(DFontInfo &f);

    //获取文件夹下及子文件夹下所有字体文件
    QStringList getFileNames(const QString &path)const;
};

#endif

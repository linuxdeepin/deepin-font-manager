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
#include <QMap>

//#include <ft2build.h>
//#include FT_FREETYPE_H

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
    qint8 previewLang; //0: none; 1: Chinese; 2: English; others...


    bool isInstalled {false};
    bool isError {false};
    bool isSystemFont {true};  // Default is system font

    bool operator==(const DFontInfo &info)
    {
        return info.familyName == familyName && info.styleName == styleName;
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
    DFontInfoManager(QObject *parent = nullptr);
    ~DFontInfoManager();

    enum FontTYpe  {
        All,
        Chinese,
        MonoSpace,
    };

    void refreshList();
    QStringList getAllFontPath(bool isStartup = false) const;
    QStringList getAllChineseFontPath() const;
    QStringList getAllMonoSpaceFontPath() const;
    QString getFontType(const QString &filePath);
    DFontInfo getFontInfo(const QString &filePath);
    QString getDefaultPreview(const QString &filePath, qint8 &preivewLang);
    QStringList getCurrentFontFamily();
    QStringList getFontFamilyStyle(const QString &filePah);
    QStringList getFonts(FontTYpe type) const;
    QString getInstFontPath(const QString &originPath, const QString &familyName);
    bool isFontInstalled(DFontInfo &data);
    void getDefaultPreview(DFontInfo &data);
    void checkStyleName(DFontInfo &f);

//    //获取指定文件夹下所有的子文件夹
//    QStringList getDirPathOfSplDir(QString dirPath)const;
//    //获取文件夹下的字体文件
//    QStringList getFileNames(const QString &path)const;

    //获取文件夹下及子文件夹下所有字体文件
    QStringList getFileNames(const QString &path)const;

private:
//    QMap<QString, DFontInfo> m_fontInfoMap;
//    FT_Library m_library;
//    FT_Face m_face;
};

#endif

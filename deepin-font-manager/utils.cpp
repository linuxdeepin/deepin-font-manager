/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
 * Maintainer: rekols <rekols@foxmail.com>
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

#include "utils.h"
#include "globaldef.h"

#include <QUrl>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QFontInfo>
#include <QMimeType>
#include <QApplication>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QImageReader>
#include <QPixmap>
#include <QFile>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QTextLayout>

QHash<QString, QPixmap> Utils::m_imgCacheHash;
QHash<QString, QString> Utils::m_fontNameCache;

/*************************************************************************
 <Function>      Utils
 <Description>   构造函数-全文索引类
 <Author>
 <Input>
    <param1>     parent          Description:父对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
Utils::Utils(QObject *parent)
    : QObject(parent)
{
}

/*************************************************************************
 <Function>      Utils
 <Description>   析构函数
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
Utils::~Utils()
{
}

/*************************************************************************
 <Function>      getQssContent
 <Description>   读取文件字符串内容
 <Author>
 <Input>
    <param1>     filePath        Description:当前文件路径
 <Return>        QString         Description:返回当前读取内容字符串
 <Note>          null
*************************************************************************/
QString Utils::getQssContent(const QString &filePath)
{
    QFile file(filePath);
    QString qss;

    if (file.open(QIODevice::ReadOnly)) {
        qss = file.readAll();
    }

    return qss;
}

/*************************************************************************
 <Function>      getConfigPath
 <Description>   读取应用配置文件信息字符串
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
QString Utils::getConfigPath()
{
    QDir dir(QDir(QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first())
             .filePath(qApp->organizationName()));

    return dir.filePath(qApp->applicationName());
}

/*************************************************************************
 <Function>      isFontMimeType
 <Description>   判断当前是否为MIME类型的数据库
 <Author>        null
 <Input>
    <param1>     filePath        Description:文件路径
 <Return>        bool            Description:是否为MIME类型的数据库，true，是；false：否
 <Note>          null
*************************************************************************/
bool Utils::isFontMimeType(const QString &filePath)
{
    const QString mimeName = QMimeDatabase().mimeTypeForFile(filePath).name();

    if (QString(FONT_FILE_MIME).contains(mimeName)) {
        return true;
    }

    return false;
}

/*************************************************************************
 <Function>      suffixList
 <Description>   获取文件后缀类型
 <Author>
 <Input>         null
 <Return>        QString            Description:文件后缀类型
 <Note>          null
*************************************************************************/
QString Utils::suffixList()
{
    return QString("Font Files (*.ttf *.ttc *.otf)");
}

/*************************************************************************
 <Function>      renderSVG
 <Description>   读取图像信息
 <Author>
 <Input>
    <param1>     filePath        Description:文件路径
    <param2>     size            Description:size大小
 <Return>        QPixmap         Description:返回读取到的图像信息
 <Note>          null
*************************************************************************/
QPixmap Utils::renderSVG(const QString &filePath, const QSize &size)
{
    if (m_imgCacheHash.contains(filePath)) {
        return m_imgCacheHash.value(filePath);
    }

    QImageReader reader;
    QPixmap pixmap;

    reader.setFileName(filePath);

    if (reader.canRead()) {
        const qreal ratio = qApp->devicePixelRatio();
        reader.setScaledSize(size * ratio);
        pixmap = QPixmap::fromImage(reader.read());
        pixmap.setDevicePixelRatio(ratio);
    } else {
        pixmap.load(filePath);
    }

    m_imgCacheHash.insert(filePath, pixmap);

    return pixmap;
}

/*************************************************************************
 <Function>      loadFontFamilyFromFiles
 <Description>   获取字体familyName信息
 <Author>
 <Input>
    <param1>     fontFileName    Description:字体文件名
 <Return>        QString         Description:字体familyName信息
 <Note>          null
*************************************************************************/
QString Utils::loadFontFamilyFromFiles(const QString &fontFileName)
{
    if (m_fontNameCache.contains(fontFileName)) {
        return m_fontNameCache.value(fontFileName);
    }

    QString fontFamilyName = "";

    QFile fontFile(fontFileName);
    if (!fontFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Open font file error";
        return fontFamilyName;
    }

    int loadedFontID = QFontDatabase::addApplicationFontFromData(fontFile.readAll());
    QStringList loadedFontFamilies = QFontDatabase::applicationFontFamilies(loadedFontID);
    if (!loadedFontFamilies.empty()) {
        fontFamilyName = loadedFontFamilies.at(0);
    }
    fontFile.close();

    m_fontNameCache.insert(fontFileName, fontFamilyName);
    return fontFamilyName;
}

/*************************************************************************
 <Function>      holdTextInRect
 <Description>   获取输入文本QTextLine信息
 <Author>
 <Input>
    <param1>     font            Description:字体信息
    <param2>     text            Description:文本参数
    <param3>     size            Description:size参数
 <Return>        QString         Description:文本QTextLine信息
 <Note>          null
*************************************************************************/
const QString Utils::holdTextInRect(const QFont &font, QString text, const QSize &size)
{
    QFontMetrics fm(font);
    QTextLayout layout(text);

    layout.setFont(font);

    QStringList lines;
    QTextOption &text_option = *const_cast<QTextOption *>(&layout.textOption());

    text_option.setWrapMode(QTextOption::WordWrap);
    text_option.setAlignment(Qt::AlignTop | Qt::AlignLeft);

    layout.beginLayout();

    QTextLine line = layout.createLine();
    int height = 0;
    int lineHeight = fm.height();

    while (line.isValid()) {
        height += lineHeight;

        if (height + lineHeight > size.height()) {
            const QString &end_str = fm.elidedText(text.mid(line.textStart()), Qt::ElideRight, size.width());

            layout.endLayout();
            layout.setText(end_str);

            text_option.setWrapMode(QTextOption::NoWrap);
            layout.beginLayout();
            line = layout.createLine();
            line.setLineWidth(size.width() - 1);
            text = end_str;
        } else {
            line.setLineWidth(size.width());
        }

        lines.append(text.mid(line.textStart(), line.textLength()));

        if (height + lineHeight > size.height()) break;

        line = layout.createLine();
    }

    layout.endLayout();

    return lines.join("");
}

/*************************************************************************
 <Function>      convertToPreviewString
 <Description>   转换成预览文本信息
 <Author>
 <Input>
    <param1>     fontFilePath    Description:字体路径
    <param3>     srcString       Description:字符串参数
 <Return>        QString         Description:预览文本信息
 <Note>          null
*************************************************************************/
QString Utils::convertToPreviewString(QString fontFilePath, QString srcString)
{
    if (fontFilePath.isEmpty()) {
        return srcString;
    }

    QString strFontPreview = srcString;

    QRawFont rawFont(fontFilePath, 0, QFont::PreferNoHinting);
    bool isSupport = rawFont.supportsCharacter(QChar('a'));
    bool isSupportF = rawFont.supportsCharacter(QChar('a' | 0xf000));
    if ((!isSupport && isSupportF)) {
        QChar *chArr = new QChar[srcString.length() + 1];
        for (int i = 0; i < srcString.length(); i++) {
            int ch = srcString.at(i).toLatin1();
            //判断字符ascii在32～126范围内(共95个)
            if (ch >= 32 && ch <= 126) {
                ch |= 0xf000;
                chArr[i] = QChar(ch);
            } else {
                chArr[i] = srcString.at(i);
            }
        }
        chArr[srcString.length()] = '\0';
        QString strResult(chArr);
        strFontPreview = strResult;
        delete[] chArr;
    }

    return strFontPreview;
}

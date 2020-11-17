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

#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QHash>

/*************************************************************************
 <Class>         Utils
 <Description>   全文索引类
 <Author>
 <Note>          null
*************************************************************************/
class Utils
{
public:
    static QHash<QString, QPixmap> m_imgCacheHash;
    static QHash<QString, QString> m_fontNameCache;
    //读取应用配置文件信息字符串
    static QString getConfigPath();
    //判断当前是否为MIME类型的数据库
    static bool isFontMimeType(const QString &filePath);
    //获取文件后缀类型
    static QString suffixList();
    //读取图像信息
    static QPixmap renderSVG(const QString &filePath, const QSize &size);
    //转换成预览文本信息
    static QString convertToPreviewString(const QString &fontFilePath, const QString &srcString);
};

#endif

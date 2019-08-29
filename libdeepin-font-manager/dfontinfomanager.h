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

struct DFontInfo {
    QString filePath;
    QString familyName;
    QString styleName;
    QString type;
    QString version;
    QString copyright;
    QString description;
    QString sysVersion;
    bool isInstalled;
    bool isError;
    bool isSystemFont {true};  // Default is system font

    bool operator==(const DFontInfo &info)
    {
        return info.familyName == familyName && info.styleName == styleName;
    }
};

class DFontInfoManager : public QObject
{
    Q_OBJECT

public:
    static DFontInfoManager *instance();
    DFontInfoManager(QObject *parent = nullptr);
    ~DFontInfoManager();

    void refreshList();
    QStringList getAllFontPath() const;
    QString getInstalledFontPath(DFontInfo *info);
    QString getFontType(const QString &filePath);
    DFontInfo *getFontInfo(const QString &filePath);
    bool isFontInstalled(DFontInfo *data);
};

#endif

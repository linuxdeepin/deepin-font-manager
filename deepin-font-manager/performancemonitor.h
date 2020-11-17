/*****************************************************************************
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Xiao Zhiguo <xiaozhiguo@uniontech.com>
* Date:       2020-09-15
*
* Maintainer: Xiao Zhiguo <xiaozhiguo@uniontech.com>
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
*
*****************************************************************************/

#ifndef PERFORMANCEMONITOR_H
#define PERFORMANCEMONITOR_H

#include <QObject>

const QString GRAB_POINT = "[GRABPOINT]";
const QString APP_NAME = "DEEPIN_FONT_MANAGER";
const QString INIT_APP_TIME = "0001";
const QString LOAD_FONTS_TIME = "0002";
const QString INSTALL_FONTS_TIME = "0003";
const QString EXPORT_FONTS_TIME = "0004";
const QString DELETE_FONTS_TIME = "0005";
const QString FAVOURITE_FONTS_TIME = "0006";
const QString INIT_AND_LOAD_TIME = "0007";

class PerformanceMonitor : public QObject
{
    Q_OBJECT
public:
    explicit PerformanceMonitor(QObject *parent = nullptr);

    static void initializeAppStart();
    static void initializeAppFinish();

    static void loadFontStart();
    static void loadFontFinish();

    static void installFontStart();
    static void installFontFinish(int fontCount);

    static void exportFontStart();
    static void exportFontFinish(int fontCount);

    static void deleteFontStart();
    static void deleteFontFinish(int fontCount);

private:
    Q_DISABLE_COPY(PerformanceMonitor)

    static qint64 initializeAppStartMs;
    static qint64 initializeAppFinishMs;
    static qint64 loadFontStartMs;
    static qint64 loadFontFinishMs;
    static qint64 installFontStartMs;
    static qint64 installFontFinishMs;
    static qint64 exportFontStartMs;
    static qint64 exportFontFinishMs;
    static qint64 deleteFontStartMs;
    static qint64 deleteFontFinishMs;
    static qint64 favoriteFontStartMs;
    static qint64 favoriteFontFinishMs;
};

#endif // PERFORMANCEMONITOR_H

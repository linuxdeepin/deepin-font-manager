/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     lilinling <lilinling@uniontech.com>
*
* Maintainer: lilinling <lilinling@uniontech.com>
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
#pragma once

#include <QObject>
#include <QRunnable>

class DComWorker : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit DComWorker(QObject *parent = nullptr);
    void run();

};

class GetFontList  : public DComWorker
{
    Q_OBJECT
public:
    enum FontType {
        ALL,
        CHINESE,
        MONOSPACE,
    };
    explicit GetFontList(FontType type, bool isStartup, QObject *parent = nullptr);
    void run();

private:
    void removeUserAddFonts();

private:
    FontType m_type;
    bool m_isStartup;
};

class FontManager : public QObject
{
    Q_OBJECT
public:
    FontManager();
    static void getFontList(bool isStartup);
};

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

#define FONTS_DESKTOP_DIR (QString("%1/%2/").arg(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)).arg(QApplication::translate("DFontMgrMainWindow", "Fonts")))

/**
* @brief 拷贝文件线程类
* 进行文件的批量拷贝
*/
class CopyFontThread : public QObject, public QRunnable
{
    Q_OBJECT
public:
    /**
    * @brief The OPType （拷贝类型：导出/安装）
    */
    enum OPType {
        EXPORT = 0,
        INSTALL,
    };

    CopyFontThread(OPType type, qint8 index, const QStringList &copyFiles);
    void run() override;
    //取消安装
    inline void cancelInstall()
    {
        m_isCanceled = true;
    }

signals:
    void fileInstalled(const QString &familyName, const QString &targetName);

private:
    //拷贝类型：导出 安装
    short m_opType;
    qint8 m_index;
    //是否取消安装
    bool m_isCanceled;
    //需要拷贝文件的源路径列表
    QStringList m_srcFiles;
};

/**
* @brief 拷贝文件管理类
* 进行文件批量拷贝的管理
*/
class DCopyFilesManager : public QObject
{
    Q_OBJECT
public:
    explicit DCopyFilesManager(QObject *parent = nullptr);
    //拷贝文件列表
    static void copyFiles(CopyFontThread::OPType type, const QStringList &fontList);

signals:

public slots:
};

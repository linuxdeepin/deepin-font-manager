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
        INVALID,
    };

    CopyFontThread(OPType type, short index, const QStringList &copyFiles);
    void run() override;

signals:
    void fileInstalled(const QString &familyName, const QString &targetName);

private:
    //拷贝类型：导出 安装
    short m_opType;
    //线程下标
    short m_index;
    //需要拷贝文件的源路径列表
    QStringList m_srcFiles;
    //需要拷贝文件的目标路径列表
    QStringList m_targetFiles;
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
    //获取字体源路径、目标路径和familyName
    static QString getTargetPath(const QString &inPath, QString &srcPath, QString &targetPath);
    //取消安装
    static inline void cancelInstall()
    {
        if (m_type != CopyFontThread::INSTALL)
            return;

        m_installCanceled = true;
    }

    //安装是否已被取消
    static inline bool isInstallCanceled()
    {
        return m_installCanceled;
    }

    //删除取消安装时已经安装的字体文件列表
    static void deleteFiles(const QStringList  &fileList, bool isTarget);

private:
    //文件拷贝类型：导出 安装
    static CopyFontThread::OPType m_type;
    //安装是否被取消
    static volatile bool m_installCanceled;
};

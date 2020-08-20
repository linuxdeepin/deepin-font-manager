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
#include "dcopyfilesmanager.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <QStandardPaths>
#include <QApplication>
#include <QThreadPool>
#include <QDebug>


CopyFontThread::CopyFontThread(OPType type, qint8 index, const QStringList &copyFiles)
    : m_opType(type)
    , m_index(index)
    , m_isCanceled(false)
    , m_srcFiles(copyFiles)
{
}

void CopyFontThread::run()
{
    if (!autoDelete())
        setAutoDelete(true);

    if (m_srcFiles.isEmpty())
        return;

    if (m_opType != EXPORT && m_opType != INSTALL)
        return;

    for (const QString &fontFile : m_srcFiles) {
        if (m_opType == EXPORT) {
            QString target = FONTS_DESKTOP_DIR + QFileInfo(fontFile).fileName();
            if (QFileInfo(target).exists() && (QFileInfo(fontFile).size() == QFileInfo(target).size())) {
                continue;
            } else {
                QFile(target).remove();
            }

            if (!QFile::copy(fontFile, target))
                qDebug() << __FUNCTION__ << " copy file error " << fontFile << m_index;
        }
    }
}

DCopyFilesManager::DCopyFilesManager(QObject *parent)
    : QObject(parent)
{

}

/**
* @brief DCopyFilesManager::copyFiles 拷贝文件列表，打印出拷贝文件耗用时间
* @param type 拷贝文件的类型：导出 安装
* @param fontList 要拷贝的源文件列表
* @return void
*/
void DCopyFilesManager::copyFiles(CopyFontThread::OPType type, const QStringList &fontList)
{
    qint64 start = QDateTime::currentMSecsSinceEpoch();
    int tcount = QThread::idealThreadCount() > 0 ? QThread::idealThreadCount() : 1;

    qDebug() << __FUNCTION__ << tcount  << QThread::currentThreadId();

    int count = qRound(1.0f * fontList.size() / tcount);
    if (count == 0)
        ++count;
    int index = 0;
    QList<QStringList> fontsList;
    QStringList list;

    for (const QString &file : fontList) {
        if (fontsList.size() < tcount - 1) {
            if (index < count) {
                list << file;
            }
            ++index;
            if (index == count) {
                if (!list.isEmpty()) {
                    fontsList << list;
                    list.clear();
                }
                index = 0;
            }
        } else {
            list << file;
        }
    }

    if (!list.isEmpty())
        fontsList << list;

    QThreadPool *global_pool = QThreadPool::globalInstance();
    index = 0;
    for (const QStringList &fileList : fontsList) {
        CopyFontThread *thread = new CopyFontThread(type, static_cast<qint8>(index), fileList);

        global_pool->start(thread);
        index++;
    }
    global_pool->waitForDone();
    qDebug() << __FUNCTION__ << " take (ms) :" << QDateTime::currentMSecsSinceEpoch() - start;
}

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
#include "dfontmanager.h"

#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <QStandardPaths>
#include <QApplication>
#include <QThreadPool>
#include <QDir>
#include <QDebug>

const QString sysDir = QDir::homePath() + "/.local/share/fonts";

CopyFontThread::CopyFontThread(OPType type, short index)
    : m_opType(type)
    , m_index(index)
{
    if (!autoDelete())
        setAutoDelete(true);
    if (!m_srcFiles.isEmpty())
        qDebug() << __FUNCTION__ << index << m_srcFiles.size();
}

void CopyFontThread::run()
{
    if (m_srcFiles.isEmpty())
        return;

//    qDebug() << __FUNCTION__ << m_index << m_srcFiles.size() << m_srcFiles;
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
        } else if (m_opType == INSTALL) {
            if (DCopyFilesManager::isInstallCanceled()) {
                DCopyFilesManager::deleteFiles(m_targetFiles, true);
                return;
            }

            QString filePathOrig;
            QString target;
            QString familyName = DCopyFilesManager::getTargetPath(fontFile, filePathOrig, target);
            QFile::copy(filePathOrig, target);
            m_targetFiles << target;
            Q_EMIT fileInstalled(familyName, target);
        }
    }
}

void CopyFontThread::appendFile(const QString &filePath)
{
    if (!m_srcFiles.contains(filePath))
        m_srcFiles.append(filePath);
}

//文件拷贝类型：导出 安装
CopyFontThread::OPType DCopyFilesManager::m_type = CopyFontThread::INVALID;
//安装是否被取消
volatile bool DCopyFilesManager::m_installCanceled = false;

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
    if (fontList.isEmpty())
        return;

    m_type = type;
    qint64 start = QDateTime::currentMSecsSinceEpoch();
    int tcount = QThread::idealThreadCount() > 0 ? QThread::idealThreadCount() : 1;
    if (tcount > fontList.size())
        tcount = fontList.size();

    QList<CopyFontThread *> threads = QList<CopyFontThread *>();
    for (int i = 0; i < tcount; ++i) {
        CopyFontThread *thread = new CopyFontThread(type, static_cast<short>(i));
        threads.append(thread);
    }

    //debug log
    qDebug() << __FUNCTION__ << tcount  << type << fontList;

    int index = 0;

    int maxMod = (2 * tcount - 1) % (2 * tcount);
    for (const QString &file : fontList) {
        int modVal = (index % (2 * tcount));
        if (modVal < tcount) {
            threads.at(modVal)->appendFile(file);
        } else {
            threads.at(maxMod - modVal)->appendFile(file);
        }
        index++;
    }

    QThreadPool *global_pool = QThreadPool::globalInstance();
    for (CopyFontThread *thread : threads) {
        if (type == CopyFontThread::INSTALL) {
            connect(thread, &CopyFontThread::fileInstalled, DFontManager::instance(), &DFontManager::onInstallResult);
        }
        global_pool->start(thread);
    }
    global_pool->waitForDone();

    if (m_installCanceled) {
        m_installCanceled = false;
        deleteFiles(fontList, false);
    }
    m_type = CopyFontThread::INVALID;
    qDebug() << __FUNCTION__ << " take (ms) :" << QDateTime::currentMSecsSinceEpoch() - start;
}

/**
* @brief DCopyFilesManager::getTargetPath 获取安装前字体的安装后目标路径
* @param inPath 传入的带familyName的字体路径
* @param srcPath 安装前字体的路径
* @param targetPath 安装后字体的路径
* @return 字体的familyName
*/
QString DCopyFilesManager::getTargetPath(const QString &inPath, QString &srcPath, QString &targetPath)
{
    QString targetDir;
    QStringList fileParamList = inPath.split("|");

    srcPath = fileParamList.at(0);
    QString familyName = fileParamList.at(1);

    //这里有过familyname中带有 /  的话，创建的目录会多一层，导致与其他不统一，也会造成删除时删除不完全的问题
    if (familyName.contains("/")) {
        familyName.replace("/", "-");
    }
    const QFileInfo info(srcPath);
    QString dirName = familyName;

    if (dirName.isEmpty()) {
        dirName = info.baseName();
    }

    targetPath = QString("%1/%2/%3").arg(sysDir).arg(dirName).arg(info.fileName());
    targetDir = QString("%1/%2").arg(sysDir).arg(dirName);

    QDir dir(targetDir);
    dir.mkpath(".");
    return familyName;
}

/**
* @brief DCopyFilesManager::deleteFiles 删除取消安装时已经安装的字体文件列表
* @param fileList 传入的待安装字体列表
* @param isTarget 是否是目标文件，还是带familyName的源文件路径
* @return void
*/
void DCopyFilesManager::deleteFiles(const QStringList &fileList, bool isTarget)
{
    for (const QString &font : fileList) {
        QString target = font;
        QString src;
        if (!isTarget)
            getTargetPath(font, src, target);

        QFile(target).remove();
        QDir fileDir(QFileInfo(target).path());
        if (fileDir.isEmpty()) {
            fileDir.removeRecursively();
        }
    }
}

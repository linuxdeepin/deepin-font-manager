// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddlog.h"
#include "dcomworker.h"
#include "dfontinfomanager.h"
#include "dfontpreviewlistdatathread.h"
#include "dfmdbmanager.h"
#include "dcopyfilesmanager.h"

#include <QDir>

#include <unistd.h>

FontManager *FontManager::m_fontManager = nullptr;

FontManager *FontManager::instance()
{
    // qDebug() << "FontManager instance requested";
    if (m_fontManager == nullptr) {
        // qDebug() << "Creating new FontManager instance";
        m_fontManager = new FontManager;
    }

    // qDebug() << "Returning FontManager instance";
    return m_fontManager;
}

DComWorker::DComWorker(QObject *parent)
    : QObject(parent)
{
    // qDebug() << "DComWorker created";
    if (!autoDelete()) {
        // qDebug() << "Setting autoDelete to true";
        setAutoDelete(true);
    }
}

void DComWorker::run()
{
    // qDebug() << "DComWorker::run() started";
}

GetFontListWorker::GetFontListWorker(GetFontListWorker::Type type, QObject *parent)
    : DComWorker(parent)
    , m_type(type)
{
    // qDebug() << "GetFontListWorker created with type:" << type;
}


void GetFontListWorker::run()
{
    qCDebug(fmLog) << __FUNCTION__ << m_type << "begin";
    DFontInfoManager *inst = DFontInfoManager::instance();
    DFontPreviewListDataThread *thread = DFontPreviewListDataThread::instance();
    if (m_type == Startup) {
        qCDebug(fmLog) << "Processing Startup font list";
        thread->m_allFontPathList.clear();
        thread->m_allFontPathList = inst->getAllFontPath(true);
//        removeUserAddFonts();

        if(DFMDBManager::instance()->isDBDeleted()){
            qCDebug(fmLog) << "Database is deleted, updating database";
            thread->updateDb();//想把它放在第1个，getStartFontList之前
            DFMDBManager::instance()->getAllRecords();
            QList<DFontPreviewItemData> list = DFMDBManager::instance()->getFontInfo(50, &thread->m_delFontInfoList);//DFMDBManager::recordList
            thread->m_fontModelList.clear();
            thread->m_fontModelList.append(list);
        }
        else {
            qCDebug(fmLog) << "Database exists, retrieving records";
            DFMDBManager::instance()->getAllRecords();
            QList<DFontPreviewItemData> list = DFMDBManager::instance()->getFontInfo(50, &thread->m_delFontInfoList);
            thread->m_startModelList = list;
            thread->m_fontModelList.append(list);
        }
    }

    if (m_type == ALL || m_type == AllInSquence) {
        qCDebug(fmLog) << "Processing ALL or AllInSquence font list";
        thread->m_allFontPathList.clear();
        thread->m_allFontPathList = inst->getAllFontPath(false);
    }

    if (m_type == CHINESE || m_type == AllInSquence) {
        qCDebug(fmLog) << "Processing CHINESE or AllInSquence font list";
        thread->m_chineseFontPathList.clear();
        thread->m_chineseFontPathList = inst->getAllChineseFontPath();
    }

    if (m_type == MONOSPACE || m_type == AllInSquence) {
        qCDebug(fmLog) << "Processing MONOSPACE or AllInSquence font list";
        thread->m_monoSpaceFontPathList.clear();
        thread->m_monoSpaceFontPathList = inst->getAllMonoSpaceFontPath();
    }
    qCDebug(fmLog) << __FUNCTION__ << m_type << "end";
}

void GetFontListWorker::removeUserAddFonts()
{
    qCDebug(fmLog) << "Starting removeUserAddFonts";
    if (geteuid() == 0) {
        qCWarning(fmLog) << "Running as root, skipping user font removal";
        return;
    }
    QStringList installFont = DFMDBManager::instance()->getInstalledFontsPath();

    QList<QString>::iterator iter = DFontPreviewListDataThread::instance()->m_allFontPathList.begin();
    while (iter != DFontPreviewListDataThread::instance()->m_allFontPathList.end()) {
        QString filePath = *iter;
        if (!DFMDBManager::instance()->isSystemFont(filePath) && !installFont.contains(filePath)) {
            // qDebug() << "Removing user-added font:" << filePath;
            QFileInfo openFile(filePath);
            QFile::remove(filePath);

            QDir fileDir(openFile.path());
            if (fileDir.isEmpty()) {
                // qDebug() << "Removing empty directory:" << fileDir.path();
                fileDir.removeRecursively();
            }

            iter = DFontPreviewListDataThread::instance()->m_allFontPathList.erase(iter);
        } else {
            ++iter;
        }
    }
    qCDebug(fmLog) << "Finished removeUserAddFonts";
}

void FontManager::getFontListInSequence()
{
    qCDebug(fmLog) << "Getting font list in sequence";
    GetFontListWorker getFontList(GetFontListWorker::AllInSquence);
    getFontList.run();
    qCDebug(fmLog) << "Font list sequence completed";
}


/*************************************************************************
 <Function>      getStartFontList
 <Description>   获取启动时的需要用到的字体列表
 <Author>        null
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void FontManager::getStartFontList()
{
    qCDebug(fmLog) << "Getting startup font list";
    QThreadPool *threadPool = DCopyFilesManager::instance()->getPool();
    qCInfo(fmLog) << "Thread pool worker count:" << threadPool->maxThreadCount();

    qCDebug(fmLog) << "Creating and starting GetAll worker";
    GetFontListWorker *getAll = new GetFontListWorker(GetFontListWorker::Startup);
    threadPool->start(getAll);
    GetFontListWorker *getChinese = new GetFontListWorker(GetFontListWorker::CHINESE);
    threadPool->start(getChinese);
    GetFontListWorker *getMonospace = new GetFontListWorker(GetFontListWorker::MONOSPACE);
    threadPool->start(getMonospace);
    threadPool->waitForDone();
    qCDebug(fmLog) << "All workers completed";
}

/*************************************************************************
 <Function>      getChineseAndMonoFont
 <Description>   获取当前系统中的中文字体和等宽字体
 <Author>        null
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void FontManager::getChineseAndMonoFont()
{
    qCDebug(fmLog) << "Getting Chinese and monospace fonts";
    QThreadPool *threadPool = DCopyFilesManager::instance()->getPool();
    qCInfo(fmLog) << "Thread pool worker count:" << threadPool->maxThreadCount();

    GetFontListWorker *getChinese = new GetFontListWorker(GetFontListWorker::CHINESE);
    threadPool->start(getChinese);
    GetFontListWorker *getMonospace = new GetFontListWorker(GetFontListWorker::MONOSPACE);
    threadPool->start(getMonospace);
    threadPool->waitForDone();
    qCDebug(fmLog) << "Workers completed";
}

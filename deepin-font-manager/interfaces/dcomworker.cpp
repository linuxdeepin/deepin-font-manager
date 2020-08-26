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
#include "dcomworker.h"
#include "dfontinfomanager.h"
#include "dfontpreviewlistdatathread.h"
#include "dfmdbmanager.h"

#include <QThreadPool>

DComWorker::DComWorker(QObject *parent) : QObject(parent)
{
    if (!autoDelete())
        setAutoDelete(true);
}

void DComWorker::run()
{

}

GetFontList::GetFontList(GetFontList::FontType type, bool isStartup, QObject *parent)
    : DComWorker(parent)
    , m_type(type)
    , m_isStartup(isStartup)
{

}

void GetFontList::run()
{
    DFontInfoManager *inst = DFontInfoManager::instance();
    DFontPreviewListDataThread *thread = DFontPreviewListDataThread::instance();
    switch (m_type) {
    case ALL:
        thread->m_allFontPathList.clear();
        thread->m_allFontPathList = inst->getAllFontPath(m_isStartup);
        if (m_isStartup) {
            qDebug() << __FUNCTION__ << "LLLLLLLLLLLLLLLLLL";
            inst->refreshList(thread->m_allFontPathList);
            thread->m_fontModelList = DFMDBManager::instance()->getAllFontInfo(&thread->m_delFontInfoList);
        }
        break;
    case CHINESE:
        thread->m_chineseFontPathList.clear();
        thread->m_chineseFontPathList = inst->getAllChineseFontPath();
        break;
    case MONOSPACE:
        thread->m_monoSpaceFontPathList.clear();
        thread->m_monoSpaceFontPathList = inst->getAllMonoSpaceFontPath();
        break;
    }
}

void FontManager::getFontList(bool isStartup)
{
    QThreadPool *threadPool = QThreadPool::globalInstance();
    GetFontList *getAll = new GetFontList(GetFontList::ALL, isStartup);
    threadPool->start(getAll);
    GetFontList *getChinese = new GetFontList(GetFontList::CHINESE, isStartup);
    threadPool->start(getChinese);
    GetFontList *getMonospace = new GetFontList(GetFontList::MONOSPACE, isStartup);
    threadPool->start(getMonospace);
    threadPool->waitForDone();
}

/*
 *
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:
*
* Maintainer:
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

#include "loadfontdatathread.h"
#include "dfontpreviewlistview.h"
#include <QTimer>

LoadFontDataThread::LoadFontDataThread(QList<QMap<QString, QString>> &list, QObject *parent)
    : QThread(parent)
    , m_list(list)
{
}

void LoadFontDataThread::run()
{
//    qDebug() << QThread::currentThreadId() << __func__ << "------------";
    SignalManager::m_isOnLoad = true;
    DFontPreviewListDataThread *thread = DFontPreviewListDataThread::instance();
    QList<DFontPreviewItemData> fontinfoList = DFMDBManager::instance()->getFontInfo(m_list, &m_delFontInfoList);

    thread->m_fontModelList.append(fontinfoList);

    for (DFontPreviewItemData &itemData : m_delFontInfoList) {
        //如果字体文件已经不存在，则从t_manager表中删除
        //删除字体之前启用字体，防止下次重新安装后就被禁用
        thread->getView()->enableFont(itemData.fontInfo.filePath);
        DFMDBManager::instance()->deleteFontInfo(itemData);
    }

    DFMDBManager::instance()->commitDeleteFontInfo();
    thread->getView()->enableFonts();

    foreach (auto it, fontinfoList) {
        thread->addPathWatcher(it.fontInfo.filePath);
    }
    m_delFontInfoList.clear();
    SignalManager::m_isOnLoad = false;
    SignalManager::m_isDataLoadFinish = true;

    emit dataLoadFinish(fontinfoList);
}


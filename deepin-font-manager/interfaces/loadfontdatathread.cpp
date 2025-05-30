// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "loadfontdatathread.h"
#include "dfontpreviewlistview.h"
#include <QTimer>

LoadFontDataThread::LoadFontDataThread(QList<QMap<QString, QString>> &list, QObject *parent)
    : QThread(parent)
    , m_list(list)
{
    qDebug() << "Creating LoadFontDataThread with" << list.size() << "fonts to load";
}

void LoadFontDataThread::run()
{
    qDebug() << "Starting LoadFontDataThread, thread id:" << QThread::currentThreadId();
    
    SignalManager::m_isOnLoad = true;
    DFontPreviewListDataThread *thread = DFontPreviewListDataThread::instance();
    QList<DFontPreviewItemData> fontinfoList = DFMDBManager::instance()->getFontInfo(m_list, &m_delFontInfoList);

    thread->m_fontModelList.append(fontinfoList);

    qDebug() << "Processing" << m_delFontInfoList.size() << "fonts to delete";
    for (DFontPreviewItemData &itemData : m_delFontInfoList) {
        //如果字体文件已经不存在，则从t_manager表中删除
        //删除字体之前启用字体，防止下次重新安装后就被禁用
        thread->getView()->enableFont(itemData.fontInfo.filePath);
        DFMDBManager::instance()->deleteFontInfo(itemData);
    }

    qDebug() << "Committing deleted font info";
    DFMDBManager::instance()->commitDeleteFontInfo();
    thread->getView()->enableFonts();

    qDebug() << "Setting up file watchers for" << fontinfoList.size() << "fonts";
    foreach (auto it, fontinfoList) {
        thread->addPathWatcher(it.fontInfo.filePath);
    }

    m_delFontInfoList.clear();
    SignalManager::m_isOnLoad = false;
    SignalManager::m_isDataLoadFinish = true;

    qDebug() << "Emitting dataLoadFinish signal with" << fontinfoList.size() << "fonts";
    emit dataLoadFinish(fontinfoList);
    
    qDebug() << "LoadFontDataThread completed";
}


// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ddlog.h"
#include "getuseraddfontthread.h"
#include "dfmdbmanager.h"
#include "dcomworker.h"
#include "dfontpreviewlistdatathread.h"

GetUserAddFontThread::GetUserAddFontThread(QObject *parent)
    : QThread(parent)
{
    qCDebug(fmLog) << "Creating GetUserAddFontThread";

}

void GetUserAddFontThread::run()
{
    qCDebug(fmLog) << "Starting GetUserAddFontThread";
    
    DFontInfoManager *fontInfoManager = DFontInfoManager::instance();
    DFMDBManager *fDbManager = DFMDBManager::instance();
    qCDebug(fmLog) << "Got font info and DB manager instances";

    QStringList dbPathlist = fDbManager->getInstalledFontsPath();
    QStringList fclistPathList = fontInfoManager->getAllFclistPathList();
    qCDebug(fmLog) << "Got DB paths:" << dbPathlist.size() << "and fclist paths:" << fclistPathList.size();

    QList<DFontInfo> fontInfolist;
    QStringList reduceSameFontList;
    DFontInfo fontInfo;
    //通过fc-list获取安装字体并与数据库对比，如果存在选项不在数据库这该字体不是通过字体管理器安装，把它加入列表
    for (const auto &it : fclistPathList) {
        if (!dbPathlist.contains(it)) {
            qCDebug(fmLog) << "Found user-added font:" << it;
            fontInfo = fontInfoManager->getFontInfo(it, true);
            //去重
            if (!fontInfolist.contains(fontInfo) && fDbManager->isFontInfoExist(fontInfo) == QString()) {
                qCDebug(fmLog) << "Adding unique font:" << fontInfo.filePath;
                fontInfolist << fontInfo;
            } else {
                qCDebug(fmLog) << "Skipping duplicate font:" << fontInfo.filePath;
            }
        }
    }

    qCInfo(fmLog) << "Found" << fontInfolist.size() << "user-added fonts";

    FontManager::instance()->getChineseAndMonoFont();

    DFontPreviewListDataThread::instance()->onRefreshUserAddFont(fontInfolist);
    qCDebug(fmLog) << "GetUserAddFontThread completed";
}

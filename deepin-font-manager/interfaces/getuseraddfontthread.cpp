#include "getuseraddfontthread.h"
#include "dfmdbmanager.h"
#include "dcomworker.h"
#include "dfontpreviewlistdatathread.h"

GetUserAddFontThread::GetUserAddFontThread()
{

}

void GetUserAddFontThread::run()
{
    DFontInfoManager *fontInfoManager = DFontInfoManager::instance();
    DFMDBManager *fDbManager = DFMDBManager::instance();


    QStringList dbPathlist = fDbManager->getInstalledFontsPath();
    QStringList fclistPathList = fontInfoManager->getAllFclistPathList();
    QList<DFontInfo> fontInfolist;

    QStringList reduceSameFontList;

    DFontInfo fontInfo;

    foreach (auto it, fclistPathList) {
        if (!dbPathlist.contains(it)) {
            fontInfo = fontInfoManager->getFontInfo(it,true);
            //去重
            if(!fontInfolist.contains(fontInfo)&&fDbManager->isFontInfoExist(fontInfo) == QString()){
                fontInfolist << fontInfo;
            }
        }
    }

    FontManager::instance()->getChineseAndMonoFont();

    DFontPreviewListDataThread::instance()->onRefreshUserAddFont(fontInfolist);

}

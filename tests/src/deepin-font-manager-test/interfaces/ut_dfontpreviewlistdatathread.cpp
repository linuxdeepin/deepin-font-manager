/*
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

#include "dfontpreviewlistdatathread.h"


#include "dfontpreviewlistview.h"
#include <QWidget>
#include <QDir>
#include <QSignalSpy>
#include <QList>

#include "dfmxmlwrapper.h"
#include "dfontpreviewlistview.h"
#include "dcomworker.h"

#include <QMutexLocker>
#include <QFontDatabase>
#include <QApplication>
#include <QFileSystemWatcher>
#include <QFile>

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

namespace {
class TestDFontPreviewListDataThread : public testing::Test
{

protected:
    void SetUp()
    {
        listview = new DFontPreviewListView(w);
        dfdatathead = DFontPreviewListDataThread::instance(listview);
    }
    void TearDown()
    {

    }
    QWidget *w = new QWidget;
    DFontPreviewListView *listview;
    DFontPreviewListDataThread *dfdatathead;
public:
    bool stub_exists();


};

QList<DFontPreviewItemData> stub_getAllFontInfo()
{
    DFontPreviewItemData data;
    QString filepath = "/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf";
    data.fontInfo.filePath = filepath;
    data.fontInfo.type = "Truetype";
    QList<DFontPreviewItemData> s;
    s << data;
    return s;

}

void stub_return()
{
    return ;
}

bool TestDFontPreviewListDataThread::stub_exists()
{
    return false;
}

QStringList stub_getFontDisableFontlist()
{
    QStringList list;
    list << "first";
    return list;
}
}
TEST_F(TestDFontPreviewListDataThread, checkInitFileSystemWatcher)
{
    dfdatathead->initFileSystemWatcher();
    QString FONTS_DIR = QDir::homePath() + "/.local/share/fonts/";
    QDir d(FONTS_DIR);
    d.removeRecursively();
    dfdatathead->initFileSystemWatcher();
    QDir d2(FONTS_DIR);
    d2.mkdir(".");

}


TEST_F(TestDFontPreviewListDataThread, checkUpdateChangedDir)
{
    QString filepath = "/home/zhaogongqiang/Desktop/1048字体";
    dfdatathead->updateChangedDir();
}

TEST_F(TestDFontPreviewListDataThread, checkRefreshFontListData)
{
    Stub s;
    s.set(ADDR(DFMDBManager, getAllFontInfo), stub_getAllFontInfo);

    Stub s2;
    s2.set(ADDR(DFMDBManager, commitUpdateFontInfo), stub_return);
    QSignalSpy spy(dfdatathead->m_view, SIGNAL(itemsSelected(const QStringList & files, bool isFirstInstall = false)));
    QSignalSpy spy2(dfdatathead->m_view, SIGNAL(multiItemsAdded(QList<DFontPreviewItemData> &, DFontSpinnerWidget::SpinnerStyles)));

    dfdatathead->refreshFontListData(true, QStringList());
    qDebug() << spy2.count() << endl;
    EXPECT_TRUE(spy2.count() == 1);

    EXPECT_TRUE(dfdatathead->m_fontModelList.isEmpty());
    dfdatathead->m_fontModelList.clear();
    dfdatathead->refreshFontListData(false, QStringList());
    EXPECT_TRUE(spy2.count() == 2);
    qDebug() << dfdatathead->m_fontModelList.count() << endl;
    EXPECT_FALSE(dfdatathead->m_fontModelList.isEmpty());

    SignalManager::m_isDataLoadFinish = false;
    DFontPreviewItemData dpitemd;
    dfdatathead->m_delFontInfoList.append(dpitemd);
    dfdatathead->m_startModelList.append(dpitemd);
    dfdatathead->refreshFontListData(true, QStringList("12345"));

    SignalManager::m_isDataLoadFinish = true;
    dfdatathead->m_delFontInfoList.append(dpitemd);
    dfdatathead->refreshFontListData(true, QStringList("12345"));

    dfdatathead->refreshFontListData(false, QStringList("12345"));
}

TEST_F(TestDFontPreviewListDataThread, checkRemovePathWatcher)
{
    QString FONTS_DIR = QDir::homePath() + "/.local/share/fonts/";
    dfdatathead->removePathWatcher(FONTS_DIR);
    EXPECT_FALSE(dfdatathead->m_fsWatcher->directories().contains(FONTS_DIR));
    dfdatathead->m_fsWatcher = nullptr;
    dfdatathead->removePathWatcher(FONTS_DIR);
    dfdatathead->m_fsWatcher = new QFileSystemWatcher(dfdatathead);
}

TEST_F(TestDFontPreviewListDataThread, checkOnRemoveFileWatchers)
{
    QString FONTS_DIR = QDir::homePath() + "/.local/share/fonts/";
    dfdatathead->onRemoveFileWatchers(QStringList());
    EXPECT_FALSE(dfdatathead->m_fsWatcher->directories().contains(FONTS_DIR));

}

TEST_F(TestDFontPreviewListDataThread, checkOnExportFont)
{
    QString desktopPath = QString("%1/%2/").arg(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation))
                          .arg(QApplication::translate("DFontMgrMainWindow", "Fonts"));

    QDir d(desktopPath);
    d.removeRecursively();

    QString filepath = "|home|zhaogongqiang|Desktop|1048字体|ArkanaScriptRough.otf";
    QStringList l;
    l << filepath;
    QSignalSpy spy(dfdatathead, SIGNAL(exportFontFinished(int)));

    dfdatathead->onExportFont(l);
    EXPECT_TRUE(spy.count() == 1);

    dfdatathead->onExportFont(l);
    EXPECT_TRUE(spy.count() == 2);
}

TEST_F(TestDFontPreviewListDataThread, checkGetFontData)
{
    FontData data;
    data.strFontName = "aaa";

    dfdatathead->m_fontModelList.clear();

    DFontPreviewItemData data1;
    data1.appFontId = 2;

    FontData data2;
    data2.strFontName = "aaa";
    data1.fontData = data2;
    dfdatathead->m_fontModelList.append(data1);


    DFontPreviewItemData da;

    da = DFontPreviewListDataThread::getFontData(data);
    EXPECT_TRUE(da.appFontId == 2);

    data.strFontName = "bbb";
    da = DFontPreviewListDataThread::getFontData(data);
    EXPECT_TRUE(da.appFontId == -1);

}


TEST_F(TestDFontPreviewListDataThread, checkSyncFontEnableDisableStatusData)
{
    Stub s;
    s.set(ADDR(DFMDBManager, commitUpdateFontInfo), stub_return);

    Stub s1;
    s1.set((void (DFMDBManager::*)(const DFontPreviewItemData &, const QString &))ADDR(DFMDBManager, updateFontInfo), stub_return);

    QStringList disableFontPathList;
    disableFontPathList << "first";

    DFontPreviewItemData data;
    data.fontData.setEnabled(false);
    data.fontInfo.filePath = "first";
    dfdatathead->m_fontModelList.clear();
    dfdatathead->m_fontModelList.append(data);
    dfdatathead->syncFontEnableDisableStatusData(disableFontPathList);

    data.fontData.setEnabled(true);
    data.fontInfo.filePath = "first";
    dfdatathead->m_fontModelList.clear();
    dfdatathead->m_fontModelList.append(data);
    dfdatathead->syncFontEnableDisableStatusData(disableFontPathList);
    EXPECT_FALSE(dfdatathead->m_fontModelList.first().fontData.isEnabled());
}

TEST_F(TestDFontPreviewListDataThread, checkDoWork)
{
    Stub s;
    s.set(ADDR(DFMXmlWrapper, getFontConfigDisableFontPathList), stub_getFontDisableFontlist);

    Stub s1;
    s1.set(ADDR(FontManager, getStartFontList), stub_return);

    Stub s2;
    s2.set(ADDR(DFMDBManager, commitUpdateFontInfo), stub_return);

    Stub s3;
    s3.set(ADDR(DFontPreviewListView, onMultiItemsAdded), stub_return);

    Stub s4;
    s4.set(ADDR(DFMDBManager, commitDeleteFontInfo), stub_return);

    QSignalSpy spy(dfdatathead->m_view, SIGNAL(multiItemsAdded(QList<DFontPreviewItemData> &, DFontSpinnerWidget::SpinnerStyles)));

    dfdatathead->m_fontModelList.clear();
    dfdatathead->m_allFontPathList.clear();
    dfdatathead->m_allFontPathList << "second";

    dfdatathead->doWork();

    EXPECT_TRUE(spy.count() == 1);

    DFontPreviewItemData data;
    dfdatathead->m_fontModelList << data;
    dfdatathead->doWork();

    Stub st;
    st.set((bool (QList<DFontPreviewItemData>::*)() const)ADDR(QList<DFontPreviewItemData>, isEmpty), ADDR(TestDFontPreviewListDataThread, stub_exists));
    dfdatathead->m_fontModelList << data;
    dfdatathead->doWork();
}

TEST_F(TestDFontPreviewListDataThread, checkGetFontModelList)
{
    dfdatathead->m_fontModelList.clear();
    DFontPreviewItemData data;
    data.fontData.setEnabled(true);
    dfdatathead->m_fontModelList << data;
    dfdatathead->m_view->onFinishedDataLoad();

    QList<DFontPreviewItemData> list = dfdatathead->getFontModelList();
    EXPECT_TRUE(list.count() == 1);
    EXPECT_TRUE(list.first().fontData.isEnabled());

    dfdatathead->m_view->m_bLoadDataFinish = false;
    list = dfdatathead->getFontModelList();
    EXPECT_TRUE(list.count() == 0);

}

TEST_F(TestDFontPreviewListDataThread, refreshStartupFontListData)
{

    Stub s;
    s.set(ADDR(DFontPreviewListView, onMultiItemsAdded), stub_return);

    dfdatathead->m_delFontInfoList.clear();
    dfdatathead->m_startModelList.clear();

    DFontPreviewItemData data;
    data.fontData.setChinese(true);

    dfdatathead->m_delFontInfoList << data;
    dfdatathead->m_startModelList << data;

    dfdatathead->refreshStartupFontListData();
    EXPECT_TRUE(dfdatathead->m_delFontInfoList.isEmpty());
}

TEST_F(TestDFontPreviewListDataThread, checkOnFileDeleted)
{
    QStringList list;
    dfdatathead->onFileDeleted(list);
}

TEST_F(TestDFontPreviewListDataThread, checkOnFileAdded)
{
    QStringList list;
    dfdatathead->onFileAdded(list);

    list << "first";

    Stub s;
    s.set(ADDR(DFontPreviewListDataThread, refreshFontListData), stub_return);
    dfdatathead->onFileAdded(list);

}

TEST_F(TestDFontPreviewListDataThread, checkUpdateItemStatus)
{
    DFontPreviewItemData data;
    data.fontData.setChinese(true);

    dfdatathead->m_startModelList.append(data);

    DFontPreviewItemData data2;
    data2.fontData.setChinese(false);
    dfdatathead->updateItemStatus(0, data2);
}

TEST_F(TestDFontPreviewListDataThread, checkForceDeleteFiles)
{
    Stub s;
    s.set(ADDR(DFontPreviewListView, deleteFontFiles), stub_return);

    QSignalSpy spy(dfdatathead, SIGNAL(requestBatchReInstallContinue()));

    QStringList list;
    list << "first";

    dfdatathead->forceDeleteFiles(list);

    EXPECT_TRUE(spy.count() == 1);
}



TEST_F(TestDFontPreviewListDataThread, onRefreshUserAddFont)
{
    QList<DFontInfo> fontInfoList;

    DFontInfo fontInfo;
    fontInfoList.append(fontInfo);


    dfdatathead->onRefreshUserAddFont(fontInfoList);

}

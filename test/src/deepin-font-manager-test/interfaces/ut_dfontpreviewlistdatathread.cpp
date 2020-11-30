#include "dfontpreviewlistdatathread.h"


#include "dfontpreviewlistview.h"
#include <QWidget>
#include <QDir>
#include <QSignalSpy>

#include "dfmxmlwrapper.h"
#include "dfontpreviewlistview.h"

#include <QFontDatabase>
#include <QApplication>
#include <QFileSystemWatcher>

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

void stub_commitAddFontInfo()
{
    return ;
}

//QList<DFontPreviewItemData>  stub_getFontModelList()
//{
//    QList<DFontPreviewItemData>  list;
//    DFontPreviewItemData data;
//    data.appFontId = 2;

//    FontData data2;
//    data2.strFontName = "aaa";
//    data.fontData = data2;
//    list.append(data);

//    return list;
//}

}

TEST_F(TestDFontPreviewListDataThread, checkInitFileSystemWatcher)
{
    dfdatathead->initFileSystemWatcher();
//    QString FONTS_DIR = QDir::homePath() + "/.local/share/fonts/";
//    QDir d(FONTS_DIR);
//    d.removeRecursively();
//    dfdatathead->initFileSystemWatcher();
//    QDir d2(FONTS_DIR);
//    d2.mkdir(".");

}

TEST_F(TestDFontPreviewListDataThread, checkUpdateChangedFile)
{
    QString filepath = "/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf";
    dfdatathead->updateChangedFile(filepath);
}

TEST_F(TestDFontPreviewListDataThread, checkUpdateChangedDir)
{
    QString filepath = "/home/zhaogongqiang/Desktop/1048字体";
    dfdatathead->updateChangedDir();
}

//TEST_F(TestDFontPreviewListDataThread, checkAddPathWatcher)
//{
//    QString FONTS_DIR = QDir::homePath() + "/.local/share/fonts/";
//    QString filepath = "/usr/share/fonts/opentype/noto/NotoSerifCJK-Regular.ttc";

//    dfdatathead->addPathWatcher(filepath);
//    //addpath后Qfilesystemwatcher自动将添加的路径删除
//    EXPECT_TRUE(dfdatathead->m_fsWatcher->directories().contains(FONTS_DIR));
//}

//TEST_F(TestDFontPreviewListDataThread, checkRefreshFontListData)
//{
//    Stub s;
//    s.set(ADDR(DFMDBManager, getAllFontInfo), stub_getAllFontInfo);

//    Stub s2;
//    s2.set(ADDR(DFMDBManager, commitAddFontInfo), stub_commitAddFontInfo);
//    QSignalSpy spy(dfdatathead->m_view, SIGNAL(itemsSelected(const QStringList & files, bool isFirstInstall = false)));
////    QSignalSpy spy(flt, SIGNAL(loadFinished(QByteArray)));
//    QSignalSpy spy2(dfdatathead->m_view, SIGNAL(multiItemsAdded(QList<DFontPreviewItemData> &, DFontSpinnerWidget::SpinnerStyles)));

////    dfdatathead->m_fontModelList.clear();
////    qDebug() << dfdatathead->m_fontModelList.count() << endl;
//    dfdatathead->refreshFontListData(true, QStringList());
//    qDebug() << spy2.count() << endl;
//    EXPECT_TRUE(spy2.count() == 1);

////    qDebug() << dfdatathead->m_fontModelList.count() << endl;
//    EXPECT_TRUE(dfdatathead->m_fontModelList.isEmpty());
//    dfdatathead->m_fontModelList.clear();
//    dfdatathead->refreshFontListData(false, QStringList());
//    EXPECT_TRUE(spy2.count() == 2);
//    qDebug() << dfdatathead->m_fontModelList.count() << endl;
//    EXPECT_FALSE(dfdatathead->m_fontModelList.isEmpty());
//}

TEST_F(TestDFontPreviewListDataThread, checkRemovePathWatcher)
{
    QString FONTS_DIR = QDir::homePath() + "/.local/share/fonts/";
    dfdatathead->removePathWatcher(FONTS_DIR);
    EXPECT_FALSE(dfdatathead->m_fsWatcher->directories().contains(FONTS_DIR));

//    delete dfdatathead->m_fsWatcher;
    dfdatathead->m_fsWatcher = nullptr;
    dfdatathead->removePathWatcher(FONTS_DIR);
    dfdatathead->m_fsWatcher = new QFileSystemWatcher(dfdatathead);
//    EXPECT_FALSE(dfdatathead->m_fsWatcher->directories().contains(FONTS_DIR));
}

//TEST_F(TestDFontPreviewListDataThread, checkOnAutoDirWatchers)
//{
//    QString FONTS_DIR = QDir::homePath() + "/.local/share/fonts/";
//    dfdatathead->removePathWatcher(FONTS_DIR);
//    EXPECT_FALSE(dfdatathead->m_fsWatcher->directories().contains(FONTS_DIR));
//    dfdatathead->onAutoDirWatchers();
//    EXPECT_TRUE(dfdatathead->m_fsWatcher->directories().contains(FONTS_DIR));
////    EXPECT_FALSE(dfdatathead->m_fsWatcher->directories().contains(FONTS_DIR));
//}

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
//    exportFontFinished
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










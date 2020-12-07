
#include "loadfontdatathread.h"
#include "dfontpreviewlistview.h"
#include "dfmdbmanager.h"
#include "../third-party/stub/stub.h"

#include <QTimer>
#include <QSignalSpy>

#include <gtest/gtest.h>



namespace {
class TestLoadFontDataThread : public testing::Test
{

protected:
    void SetUp()
    {
        fpm = new LoadFontDataThread(list);
    }
    void TearDown()
    {
        fpm->quit();
        fpm->wait();
        delete fpm;
    }
    // Some expensive resource shared by all tests.
    LoadFontDataThread *fpm;
    QList<QMap<QString, QString>> list;
};

QList<DFontPreviewItemData> stub_getFontInfo()
{
    QList<DFontPreviewItemData> list;
    DFontPreviewItemData data;
    data.appFontId = 1;

    list << data;

    return list;
}

}


TEST_F(TestLoadFontDataThread, checkDowork)
{
    Stub s;
    s.set((QList<DFontPreviewItemData>(DFMDBManager::*)(QList<QMap<QString, QString> >, QList<DFontPreviewItemData> *))ADDR(DFMDBManager, getFontInfo), stub_getFontInfo);

    QSignalSpy spy(fpm, SIGNAL(dataLoadFinish(QList<DFontPreviewItemData> &)));
    DFontPreviewItemData data;
    data.appFontId = 2;

    fpm->m_delFontInfoList << data;
    fpm->doWork();

    EXPECT_TRUE(spy.count() == 1);
}

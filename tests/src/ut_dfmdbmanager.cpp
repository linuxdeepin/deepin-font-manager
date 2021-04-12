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

#include "dfmdbmanager.h"


#include "dfontinfomanager.h"
#include "dsqliteutil.h"

#include <QDir>

#include <gtest/gtest.h>

#include "../third-party/stub/stub.h"

namespace {
class TestDFMDBManager : public testing::Test
{

protected:
    void SetUp()
    {
        fmd = DFMDBManager::instance();
    }
    void TearDown()
    {
    }
    // Some expensive resource shared by all tests.
    DFMDBManager *fmd;
};

QStringList stub_getInstalledFontsPath()
{
    QStringList list;
    list << "first" << "second" << "third";
    return list;
}

int stub_getMaxFontId(const QString &table_name = "t_fontmanager")
{
    Q_UNUSED(table_name)
    return 10;
}

void  stub_addFontInfo(const QList<DFontPreviewItemData> &fontList)
{
    Q_UNUSED(fontList)
    return;
}

}

TEST_F(TestDFMDBManager, initTest)
{

}

//传入系统字体判断是否为系统字体
TEST_F(TestDFMDBManager, check_SystemFont_IsSystemFont)
{
    Stub stub;
    EXPECT_EQ(true, fmd->isSystemFont("/usr/share/fonts/truetype/liberation/LiberationMono-Italic.ttf"));
}

//传入用户字体判断是否为系统字体
TEST_F(TestDFMDBManager, check_UserFont_IsSystestub_delRecordmFont)
{
    EXPECT_EQ(false, fmd->isSystemFont(QDir::homePath() + "/Desktop/1048字体/Addictype-Regular.otf"));
}

////传入用户字体判断是否为用户字体
//TEST_F(TestDFMDBManager, check_SystemFont_IsUserFont)
//{
//    EXPECT_EQ(false, fmd->isUserFont("/usr/share/fonts/truetype/liberation/LiberationMono-Italic.ttf"));
//}

////传入未安装用户字体判断是否为用户字体
//TEST_F(TestDFMDBManager, check_UserFontUnInstalled_IsUserFont)
//{
//    EXPECT_EQ(false, fmd->isUserFont(QDir::homePath() + "/Desktop/1048字体/Addictype-Regular.otf"));
//}

////传入已安装的用户字体
//TEST_F(TestDFMDBManager, check_UserFontInstalled_IsUserFont)
//{
//    EXPECT_EQ(true, fmd->isUserFont(QDir::homePath() + "/.local/share/fonts/UpcEBwrP72xTt/V200020_.TTF"));
//}

TEST_F(TestDFMDBManager, checkParseRecordToItemData)
{
//    QList<QString> keylist;
//    fmd->appendAllKeys(keylist);
//    QMap<QString, QString> record;

//    foreach (auto it, keylist) {
//        record.insert(it, "1");
//    }

//    DFontPreviewItemData data = fmd->parseRecordToItemData(record);

//    EXPECT_TRUE(data.fontInfo.psname == "1");
}

TEST_F(TestDFMDBManager, check_getInstalledFontsPath)
{
    Stub s;
    s.set(ADDR(DSqliteUtil, getInstalledFontsPath), stub_getInstalledFontsPath);
    EXPECT_EQ(3, fmd->getInstalledFontsPath().count());
    EXPECT_EQ(true, fmd->getInstalledFontsPath().contains("first"));
}

TEST_F(TestDFMDBManager, check_getRecordCount)
{
    Stub s;
    s.set(ADDR(DSqliteUtil, getMaxFontId), stub_getMaxFontId);
    EXPECT_EQ(10, fmd->getCurrMaxFontId());
//    EXPECT_EQ(true, fmd->getInstalledFontsPath().contains("first"));
}

//TEST_F(TestDFMDBManager, checkMapItemData)
//{
//    DFontPreviewItemData data;
//    data.fontInfo.psname = "first";

//    QMap<QString, QString> dataMap = fmd->mapItemData(data);

//    EXPECT_TRUE(dataMap.value("psname") == "first");
//}

TEST_F(TestDFMDBManager, checkGetAllFontInfo)
{
    QList<DFontPreviewItemData> list;

    QList<DFontPreviewItemData> fontItemDataList = fmd->getAllFontInfo(&list);
}

void checkIfEmpty_stub()
{
    return;
}
TEST_F(TestDFMDBManager, checkcheckIfEmpty)
{
    Stub s;
    s.set(ADDR(DSqliteUtil, checkIfEmpty), checkIfEmpty_stub);
    fmd->checkIfEmpty();
}

////updateFontInfoByFontId
//TEST_F(TestDFMDBManager, checkUpdateFontInfoByFontId)
//{
//    EXPECT_TRUE(fmd->updateFontInfoByFontId("", "psname", "first"));
//}

//TEST_F(TestDFMDBManager, checkUpdateFontInfoByFontIdByMap)
//{
//    QMap<QString, QString> dataMap;
//    dataMap.insert("pansme", "first");


//    EXPECT_FALSE(fmd->updateFontInfoByFontId("", dataMap));
//}


//TEST_F(TestDFMDBManager, checkUpdateFontInfoByFontFilePath)
//{
//    EXPECT_TRUE(fmd->updateFontInfoByFontFilePath("", "psname", "first"));
//}

//commitAddFontInfo
TEST_F(TestDFMDBManager, checkCommitAddFontInfo)
{
    Stub s;
//    stub.set((int(A::*)(int))ADDR(A,foo), foo_stub_int);
    s.set((void(DFMDBManager::*)(const QList<DFontPreviewItemData> &))ADDR(DFMDBManager, addFontInfo), stub_addFontInfo);

    fmd->m_addFontList.clear();
    fmd->commitAddFontInfo();

    DFontPreviewItemData data;
    data.fontInfo.psname = "first";

    fmd->m_addFontList << data;

    fmd->commitAddFontInfo();
}

//addFontInfo
TEST_F(TestDFMDBManager, checkAddFontInfo)
{
    QList<DFontPreviewItemData> list;

    fmd->addFontInfo(list);
}

//deleteFontInfo
TEST_F(TestDFMDBManager, checkDeleteFontInfoFirst)
{
    fmd->m_delFontList.clear();

    DFontPreviewItemData data;
    data.fontInfo.psname = "first";

    fmd->deleteFontInfo(data);

    EXPECT_TRUE(fmd->m_delFontList.contains(data));

}

//deleteFontInfo
TEST_F(TestDFMDBManager, checkDeleteFontInfoSecond)
{
    QList<DFontPreviewItemData> list;

    fmd->deleteFontInfo(list);
}

//commitUpdateFontInfo
TEST_F(TestDFMDBManager, checkCommitDeleteFontInfo)
{
    fmd->m_delFontList.clear();
    fmd->commitDeleteFontInfo();

    DFontPreviewItemData data;
    data.fontInfo.psname = "first";

    fmd->m_delFontList << data;

    fmd->commitDeleteFontInfo();
}

//updateFontInfo
TEST_F(TestDFMDBManager, checkUpdateFontInfoFirst)
{
    fmd->m_strKey = "";
    DFontPreviewItemData data;

    data.fontInfo.psname = "first";

    fmd->updateFontInfo(data, "psname");

    EXPECT_TRUE(fmd->m_strKey == "psname");
}

//updateFontInfo
TEST_F(TestDFMDBManager, checkUpdateFontInfoSecond)
{
    QList<DFontPreviewItemData> list;
    QString key = "psname";

    fmd->updateFontInfo(list, key);
}

//updateFontInfo
bool updateRecord_stub(QMap<QString, QString> where, QMap<QString, QString> data, const QString &table_name)
{
    return true;
}
TEST_F(TestDFMDBManager, checkUpdateFontInfoThird)
{
    Stub s;
    s.set(ADDR(DSqliteUtil, updateRecord), updateRecord_stub);

    QMap<QString, QString>  whereMap;
    QMap<QString, QString> dataMap;
    ASSERT_EQ(fmd->updateFontInfo(whereMap, dataMap), true);
}

//commitUpdateFontInfo
TEST_F(TestDFMDBManager, checkCommitUpdateFontInfo)
{
//    Stub s;
//    s.set(ADDR(DFMDBManager, endTransaction), stub_endTransaction);

    fmd->m_updateFontList.clear();
    fmd->commitUpdateFontInfo();

    DFontPreviewItemData data;
    data.fontInfo.psname = "first";

    fmd->m_updateFontList << data;

    fmd->commitUpdateFontInfo();
}

TEST_F(TestDFMDBManager, checkBeginAndEndTransaction)
{
    /*
       Stub s;
       s.set(ADDR(DFMDBManager, endTransaction), stub_endTransaction);*/

    fmd->beginTransaction();
    fmd->endTransaction();
}

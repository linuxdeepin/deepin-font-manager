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

#include "dfontpreviewproxymodel.h"
#include "dfontpreviewlistdatathread.h"

#include "views/dfontmgrmainwindow.h"
#include "dfontpreviewitemdef.h"
#include "views/dsplitlistwidget.h"
#include "dfontpreviewlistdatathread.h"
#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

#include <DLog>

namespace {
class TestDFontPreviewProxyModel : public testing::Test
{

protected:
    void SetUp()
    {
        fpm = new DFontPreviewProxyModel();
    }
    void TearDown()
    {
        delete fpm;
    }
    // Some expensive resource shared by all tests.
    QWidget *w = new QWidget;
    DFontPreviewProxyModel *fpm;
};
}

TEST_F(TestDFontPreviewProxyModel, checkSetFilterGroup)
{
    fpm->setFilterGroup(2);
    EXPECT_TRUE(fpm->m_filterGroup == 2);
    EXPECT_FALSE(fpm->m_useSystemFilter);
}

TEST_F(TestDFontPreviewProxyModel, checkSetFilterFontNamePattern)
{
    fpm->setFilterFontNamePattern("first");
    EXPECT_TRUE(fpm->m_fontNamePattern == "first");
    EXPECT_FALSE(fpm->m_useSystemFilter);
}

TEST_F(TestDFontPreviewProxyModel, checkIsFontNameContainsPattern)
{
    EXPECT_TRUE(fpm->isFontNameContainsPattern("aaa"));

    fpm->setFilterFontNamePattern("qqq");
    EXPECT_TRUE(fpm->isFontNameContainsPattern("qqq"));
}

TEST_F(TestDFontPreviewProxyModel, checkFilterAcceptsRow)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    fpm->setSourceModel(m_fontPreviewItemModel);
//    DFontMgrMainWindow *mw = new DFontMgrMainWindow;
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRow(0, QModelIndex());

//    QModelIndex index = fpm->index(0, 0);
    EXPECT_TRUE(fpm->filterAcceptsRow(0, QModelIndex()));

    fpm->setFilterGroup(1);
    EXPECT_FALSE(fpm->filterAcceptsRow(0,  QModelIndex()));
}

TEST_F(TestDFontPreviewProxyModel, checkFilterAcceptsRowAllFonts)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRow(0, QModelIndex());

    QModelIndex index = m_fontPreviewItemModel->index(0, 0);
    FontData fdata("aaa", false, false, false, false, FontType::OTF, true);
    m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    fpm->setFilterGroup(0);
    fpm->setFilterFontNamePattern("aaa");
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
}

TEST_F(TestDFontPreviewProxyModel, checkFilterAcceptsRowAllSysFonts)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRow(0, QModelIndex());

    QModelIndex index = m_fontPreviewItemModel->index(0, 0);
    FontData fdata("aaa", false, false, false, false, FontType::OTF, true);
    DFontPreviewItemData itemdata;
    itemdata.fontData = fdata;
    itemdata.fontInfo.filePath = "/usr/share/fonts/";
    itemdata.fontData.strFontName = "aaa";
    DFontPreviewListDataThread::instance()->m_fontModelList.append(itemdata);

//    QList<DFontPreviewItemData> list = DFontPreviewListDataThread::instance()->getFontModelList();
//    FontData fdata = list.last().fontData;
    m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
    fpm->setFilterGroup(1);
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    fpm->setFilterFontNamePattern("aaa");
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    DFontPreviewListDataThread::instance()->m_fontModelList.removeLast();
}

TEST_F(TestDFontPreviewProxyModel, checkFilterAcceptsRowAllUserFonts)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRow(0, QModelIndex());

    QModelIndex index = m_fontPreviewItemModel->index(0, 0);
    FontData fdata("aaa", false, false, false, false, FontType::OTF, true);
    DFontPreviewItemData itemdata;
    itemdata.fontData = fdata;
    itemdata.fontInfo.filePath = "/.local/share/fonts/";
    itemdata.fontData.strFontName = "aaa";
    DFontPreviewListDataThread::instance()->m_fontModelList.append(itemdata);

//    QList<DFontPreviewItemData> list = DFontPreviewListDataThread::instance()->getFontModelList();
//    FontData fdata = list.last().fontData;
    m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
    fpm->setFilterGroup(1);
    qDebug() << fpm->filterAcceptsRow(0,  QModelIndex());
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    fpm->setFilterFontNamePattern("aaa");
    qDebug() << fpm->filterAcceptsRow(0,  QModelIndex());
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    DFontPreviewListDataThread::instance()->m_fontModelList.removeLast();
}


TEST_F(TestDFontPreviewProxyModel, checkFilterAcceptsRowAllCollectFonts)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRow(0, QModelIndex());

    QModelIndex index = m_fontPreviewItemModel->index(0, 0);
    FontData fdata("aaa", false, true, false, false, FontType::OTF, true);
    DFontPreviewItemData itemdata;
    itemdata.fontData = fdata;
    itemdata.fontData.strFontName = "aaa";
    DFontPreviewListDataThread::instance()->m_fontModelList.append(itemdata);

//    QList<DFontPreviewItemData> list = DFontPreviewListDataThread::instance()->getFontModelList();
//    FontData fdata = list.last().fontData;
    m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
    fpm->setFilterGroup(3);
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    fpm->setFilterFontNamePattern("aaa");
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    fpm->setFilterGroup(4);
    EXPECT_FALSE(fpm->filterAcceptsRow(0,  QModelIndex()));
    DFontPreviewListDataThread::instance()->m_fontModelList.removeLast();
}

TEST_F(TestDFontPreviewProxyModel, checkFilterAcceptsRowAllActiviteFonts)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRow(0, QModelIndex());

    QModelIndex index = m_fontPreviewItemModel->index(0, 0);
    FontData fdata("aaa", true, true, true, true, FontType::OTF, true);
    DFontPreviewItemData itemdata;
    itemdata.fontData = fdata;
    itemdata.fontData.strFontName = "aaa";
    DFontPreviewListDataThread::instance()->m_fontModelList.append(itemdata);

//    QList<DFontPreviewItemData> list = DFontPreviewListDataThread::instance()->getFontModelList();
//    FontData fdata = list.last().fontData;
    m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
    fpm->setFilterGroup(4);
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    fpm->setFilterFontNamePattern("aaa");
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    DFontPreviewListDataThread::instance()->m_fontModelList.removeLast();
}

TEST_F(TestDFontPreviewProxyModel, checkFilterAcceptsRowAllChineseFonts)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRow(0, QModelIndex());

    QModelIndex index = m_fontPreviewItemModel->index(0, 0);
    FontData fdata("aaa", true, true, true, true, FontType::OTF, true);
    DFontPreviewItemData itemdata;
    itemdata.fontData = fdata;
    itemdata.fontData.strFontName = "aaa";
    DFontPreviewListDataThread::instance()->m_fontModelList.append(itemdata);

//    QList<DFontPreviewItemData> list = DFontPreviewListDataThread::instance()->getFontModelList();
//    FontData fdata = list.last().fontData;
    m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
    fpm->setFilterGroup(5);
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    fpm->setFilterFontNamePattern("aaa");
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    DFontPreviewListDataThread::instance()->m_fontModelList.removeLast();
}

TEST_F(TestDFontPreviewProxyModel, checkFilterAcceptsRowAllMonoFonts)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRow(0, QModelIndex());

    QModelIndex index = m_fontPreviewItemModel->index(0, 0);
    FontData fdata("aaa", true, true, true, true, FontType::OTF, true);
    DFontPreviewItemData itemdata;
    itemdata.fontData = fdata;
    itemdata.fontData.strFontName = "aaa";
    DFontPreviewListDataThread::instance()->m_fontModelList.append(itemdata);

//    QList<DFontPreviewItemData> list = DFontPreviewListDataThread::instance()->getFontModelList();
//    FontData fdata = list.last().fontData;
    m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
    fpm->setFilterGroup(6);
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    fpm->setFilterFontNamePattern("aaa");
    EXPECT_TRUE(fpm->filterAcceptsRow(0,  QModelIndex()));
    DFontPreviewListDataThread::instance()->m_fontModelList.removeLast();
}

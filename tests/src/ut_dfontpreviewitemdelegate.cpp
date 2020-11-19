#include "dfontpreviewitemdelegate.h"
#include "dfontpreviewlistview.h"

#include "globaldef.h"
#include "utils.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DStyleHelper>
#include <DPalette>
#include <DLog>
#include <DCheckBox>
#include <DLabel>
#include <DFontSizeManager>

#include <QAbstractItemView>
#include <QPainter>
#include <QHoverEvent>

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

namespace {
class TestDFontPreviewItemDelegate : public testing::Test
{

protected:
    void SetUp()
    {
        listview = new DFontPreviewListView();
        DfpDelegate = new DFontPreviewItemDelegate(listview);
    }
    void TearDown()
    {
//        p->restore();
//        delete listview;
        delete DfpDelegate;
    }
    // Some expensive resource shared by all tests.
    QWidget *w = new QWidget;
    DFontPreviewListView *listview;
    DFontPreviewItemDelegate *DfpDelegate;
    QPainter *p = new QPainter;
    QStyleOptionViewItem option;
//    QModelIndex index;
    FontData f;

};

QList<DFontPreviewItemData> stub_getFontModeList()
{

    return DFontPreviewListDataThread::instance()->m_fontModelList;
}

bool stub_getIsTabFocus()
{
    return true;
}

}

TEST_F(TestDFontPreviewItemDelegate, checkPaint)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    DFontPreviewProxyModel *fpm = new DFontPreviewProxyModel();
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->m_bLoadDataFinish = true;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRow(0, QModelIndex());

    QModelIndex index = m_fontPreviewItemModel->index(0, 0);

    FontData fdata("aaa", true, true, true, true, FontType::TTF, true);
    DFontPreviewItemData itemdata;
    itemdata.fontData = fdata;
    itemdata.fontData.strFontName = "aaa";
    itemdata.fontInfo.filePath = "/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf";
    DFontPreviewListDataThread::instance()->m_fontModelList.append(itemdata);

//    QList<DFontPreviewItemData> list = DFontPreviewListDataThread::instance()->getFontModelList();
//    FontData fdata = list.last().fontData;
    m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);

    Stub s;
    s.set(ADDR(DFontPreviewListDataThread, getFontModelList), stub_getFontModeList);
//    listview->onFinishedDataLoad();
    DfpDelegate->paint(p, option, index);
//    index.isValid();
}

TEST_F(TestDFontPreviewItemDelegate, checkPaintBackgroundSelect)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    DFontPreviewProxyModel *fpm = new DFontPreviewProxyModel();
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->m_bLoadDataFinish = true;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRow(0, QModelIndex());

    QModelIndex index = m_fontPreviewItemModel->index(0, 0);
//    listview->selectAll();

    FontData fdata("aaa", true, true, true, true, FontType::OTF, true);
    DFontPreviewItemData itemdata;
    itemdata.fontData = fdata;
    itemdata.fontData.strFontName = "aaa";
    itemdata.fontInfo.filePath = "/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf";
    DFontPreviewListDataThread::instance()->m_fontModelList.append(itemdata);

    m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);

    Stub s;
    s.set(ADDR(DFontPreviewListDataThread, getFontModelList), stub_getFontModeList);

    option.state.setFlag(QStyle::State_Selected);
    DfpDelegate->paint(p, option, index);
}

TEST_F(TestDFontPreviewItemDelegate, checkPaintBackgroundSelectAndTabfocus)
{
    QStandardItemModel *m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    DFontPreviewProxyModel *fpm = new DFontPreviewProxyModel();
    fpm->setSourceModel(m_fontPreviewItemModel);
    DFontPreviewListView *listview = new DFontPreviewListView;
    listview->m_bLoadDataFinish = true;
    listview->setModel(fpm);
    m_fontPreviewItemModel->insertRow(0, QModelIndex());

    QModelIndex index = m_fontPreviewItemModel->index(0, 0);
//    listview->selectAll();

    FontData fdata("aaa", true, true, true, true, FontType::OTF, true);
    DFontPreviewItemData itemdata;
    itemdata.fontData = fdata;
    itemdata.fontData.strFontName = "aaa";
    itemdata.fontInfo.filePath = "/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf";
    DFontPreviewListDataThread::instance()->m_fontModelList.append(itemdata);

    m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);

    Stub s;
    s.set(ADDR(DFontPreviewListDataThread, getFontModelList), stub_getFontModeList);

    Stub ss;
    ss.set(ADDR(DFontPreviewListView, getIsTabFocus), stub_getIsTabFocus);
    option.state.setFlag(QStyle::State_Selected);
//    listview->m_IsTabFocus = true;
    DfpDelegate->paint(p, option, index);
}

TEST_F(TestDFontPreviewItemDelegate, checkEventFilter)
{
    QPointF p1;
    QPointF p2;
    QHoverEvent *e = new QHoverEvent(QEvent::HoverLeave, p1, p2);
    DfpDelegate->eventFilter(listview, e);
}


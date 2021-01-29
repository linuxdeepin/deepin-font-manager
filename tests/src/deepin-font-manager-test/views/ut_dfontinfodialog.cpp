
#include "views/dfontinfodialog.h"


#include "dfontpreviewitemdef.h"
#include "utils.h"
#include "views/fonticontext.h"
#include "views/dfontinfoscrollarea.h"

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

#include <QBitmap>
#include <QTest>
#include <QFileInfo>
#include <QFontMetrics>
#include <QTextBlock>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QScrollBar>

#include <DGuiApplicationHelper>
#include <DApplication>
#include <DApplicationHelper>
#include <DLog>
#include <DFontSizeManager>
#include <DTipLabel>

namespace {
class TestDFontInfoDialog: public testing::Test
{

protected:
    void SetUp()
    {
        w = new QWidget;
        dFontInforDialog = new DFontInfoDialog(&d, w);

    }
    void TearDown()
    {
        delete dFontInforDialog;
        delete  w;
    }
    DFontPreviewItemData d;
    QWidget *w;
    DFontInfoDialog *dFontInforDialog;

};

DGuiApplicationHelper::ColorType stub_darktype()
{
    return DGuiApplicationHelper::DarkType;
}

}

TEST_F(TestDFontInfoDialog, checkKeyPressEvent)
{
    QTest::keyClick(dFontInforDialog, Qt::Key_I, Qt::ControlModifier);

    EXPECT_FALSE(dFontInforDialog->isVisible());
}

TEST_F(TestDFontInfoDialog, checkAutoHeight)
{
    dFontInforDialog->autoHeight(200);
    EXPECT_TRUE(dFontInforDialog->m_scrollArea->viewport()->height() == static_cast<int>(200 * 1.1 + 10));
    dFontInforDialog->autoHeight(350);
    EXPECT_TRUE(dFontInforDialog->m_scrollArea->viewport()->height() == 375);
}

TEST_F(TestDFontInfoDialog, checkResizeEvent)
{
    QSize s;
    QResizeEvent *re = new QResizeEvent(s, s);
    dFontInforDialog->resizeEvent(re);
}

TEST_F(TestDFontInfoDialog, checkInitConnection)
{
    emit DApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::DarkType);

    emit DApplicationHelper::instance()->themeTypeChanged(DGuiApplicationHelper::LightType);
}

TEST_F(TestDFontInfoDialog, checkAutoFeed)
{
    QString str;
    str.fill('a', 100);
    qDebug() << str << endl;
    str = dFontInforDialog->AutoFeed(str);
    //进行处理之后字符串长度发生变化
    EXPECT_TRUE(str.size() != 100);


    str.fill('a', 300);
    qDebug() << str << endl;
    str = dFontInforDialog->AutoFeed(str);
    //进行处理之后字符串长度发生变化
    EXPECT_TRUE(str.size() != 300);


    QString str2;
    str2.fill('a', 20);
    str2 = dFontInforDialog->AutoFeed(str2);
    EXPECT_TRUE(str2.size() == 20) << str2.size();
}

//fontinfoArea
TEST_F(TestDFontInfoDialog, checkfontinfoAreaEventFilter)
{
    QEvent *e = new QEvent(QEvent::FontChange);

    dFontInforDialog->m_fontinfoArea->eventFilter(dFontInforDialog->m_fontinfoArea, e);
}

TEST_F(TestDFontInfoDialog, checkInfoScroolArea)
{
    DFontPreviewItemData d2;
    d2.fontInfo.version = "first";
    d2.fontInfo.description = "second";
    QWidget *w2 = new QWidget;
    DFontInfoDialog *dialog = new DFontInfoDialog(&d2, w2);
    Q_UNUSED(dialog)
}

TEST_F(TestDFontInfoDialog, checkDarkType)
{
    Stub s;
    s.set(ADDR(DGuiApplicationHelper, themeType), stub_darktype);
    DFontPreviewItemData d2;
    d2.fontInfo.version = "first";
    d2.fontInfo.description = "second";
    QWidget *w2 = new QWidget;
    DFontInfoDialog *dialog = new DFontInfoDialog(&d2, w2);

    emit DApplicationHelper::instance()->themeTypeChanged(DApplicationHelper::DarkType);
    delete dialog;

}













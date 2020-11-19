
#include "views/dfdeletedialog.h"

#include "utils.h"
#include "globaldef.h"
#include "views/dfontmgrmainwindow.h"

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

#include <QSignalSpy>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QScrollBar>
#include <QTest>

#include <DGuiApplicationHelper>
#include <DApplication>
#include <DApplicationHelper>
#include <DFrame>
#include <DFontSizeManager>
#include <DTipLabel>
#include <DScrollArea>

namespace {
class TestDFDeleteDialog : public testing::Test
{

protected:
    void SetUp()
    {
        fm = new DFDeleteDialog(w);
    }
    void TearDown()
    {
        delete fm;
    }
    // Some expensive resource shared by all tests.
    DFontMgrMainWindow *w = new DFontMgrMainWindow;
    DFDeleteDialog *fm;
};

//DApplicationHelper::ColorType stub_themeTypeDark()
//{
//    return DApplicationHelper::DarkType;
//}

//DApplicationHelper::ColorType stub_themeTypeLight()
//{
//    return DApplicationHelper::LightType;
//}

}

TEST_F(TestDFDeleteDialog, checksetTheme)
{
//    Stub s;
//    s.set(ADDR(DApplicationHelper, themeType), stub_themeTypeDark());
    fm->setTheme();
    fm->onFontChanged(QFont());
////    s.set(ADDR(DApplicationHelper, themeType), stub_themeTypeLight());
//    fm->setTheme();
}

TEST_F(TestDFDeleteDialog, checkConnect)
{
    QTest::mouseClick(fm->m_confirmBtn, Qt::LeftButton);
    EXPECT_TRUE(fm->m_deleting);
    QTest::mouseClick(fm->m_confirmBtn, Qt::LeftButton);
    EXPECT_TRUE(fm->m_deleting);

    QSignalSpy spy(fm->m_signalManager, SIGNAL(cancelDel()));
    QTest::mouseClick(fm->m_cancelBtn, Qt::LeftButton);
    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(TestDFDeleteDialog, checkKeyPressEvent)
{
    QTest::keyPress(fm, Qt::Key_Escape);
    QTest::keyPress(fm, Qt::Key_Return);
    QTest::keyPress(fm, Qt::Key_1);
}

TEST_F(TestDFDeleteDialog, checkInitMessageDetail)
{
    fm->m_systemCnt = 0;
    fm->m_hasCurFont = false;
    fm->m_deleteCnt = 1;

    fm->initMessageDetail();
    EXPECT_TRUE(fm->messageDetail->text() == "This font will not be available to applications");

    fm->m_deleteCnt = 5;
    fm->initMessageDetail();
    EXPECT_TRUE(fm->messageDetail->text() == "These fonts will not be available to applications");

    fm->m_systemCnt = 10;
    fm->initMessageDetail();
    EXPECT_TRUE(fm->messageDetail->text() ==  QString("The other %1 system fonts cannot be deleted").arg(fm->m_systemCnt));

    fm->m_systemCnt = 0;
    fm->m_hasCurFont = true;
    fm->initMessageDetail();

    fm->m_systemCnt = 10;
    fm->initMessageDetail();
}





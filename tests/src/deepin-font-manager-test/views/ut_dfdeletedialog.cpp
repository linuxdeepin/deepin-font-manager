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


#include "views/dfdeletedialog.h"

#include "utils.h"
#include "globaldef.h"
#include "views/dfontmgrmainwindow.h"

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
#include "commonheaderfile.h"
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
#include <DCheckBox>

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

}

DGuiApplicationHelper::ColorType stub_themeType()
{
    return DGuiApplicationHelper::DarkType;
}

DGuiApplicationHelper::ColorType stub_themeType2()
{
    return DGuiApplicationHelper::LightType;
}

TEST_F(TestDFDeleteDialog, checksetTheme)
{
    Stub s1;
    s1.set(ADDR(DGuiApplicationHelper, themeType), stub_themeType);
    fm->setTheme();
    EXPECT_TRUE(DApplicationHelper::instance()->palette(fm).color(DPalette::Background) == QColor(25, 25, 25, 80));

    Stub s2;
    s2.set(ADDR(DGuiApplicationHelper, themeType), stub_themeType2);
    fm->setTheme();
    EXPECT_TRUE(DApplicationHelper::instance()->palette(fm).color(DPalette::Background) == QColor(247, 247, 247, 80));
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


TEST_F(TestDFDeleteDialog, getDeleting)
{
    DFontMgrMainWindow *w1 = new DFontMgrMainWindow;
    QString file;
    DFDeleteTTCDialog *fm1 = new DFDeleteTTCDialog(w1, file);
    fm1->m_bAapplyToAll = false;
    EXPECT_FALSE(fm1->getAapplyToAll());
    QKeyEvent *kev = new QKeyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    fm1->initMessageDetail();
    EXPECT_TRUE(fm1->applyAllCkb->text() == "Apply to all selected font families");
    fm1->eventFilter((QObject *)fm1->applyAllCkb, kev);
    EXPECT_TRUE(fm1->m_bAapplyToAll);
    fm1->eventFilter((QObject *)fm1, kev);
    fm1->keyPressEvent(kev);
    SAFE_DELETE_ELE(kev);
    kev = new QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    fm1->keyPressEvent(kev);
    SAFE_DELETE_ELE(kev);
    SAFE_DELETE_ELE(fm1);

    bool isEnable = false;
    DFDisableTTCDialog *fm2 = new DFDisableTTCDialog(w1, file, isEnable);
    fm2->m_bAapplyToAll = true;
    EXPECT_TRUE(fm2->getAapplyToAll());
    fm2->m_confirm = false;
    EXPECT_FALSE(fm2->getDeleting());
    kev = new QKeyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    fm2->eventFilter((QObject *)fm2->applyAllCkb, kev);
    fm2->eventFilter((QObject *)fm2, kev);
    fm2->keyPressEvent(kev);
    SAFE_DELETE_ELE(kev);
    kev = new QKeyEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    fm2->keyPressEvent(kev);
    EXPECT_TRUE(fm2->messageTitle->text().contains(QLatin1String("disabled")));
    fm2->initMessageTitle();

    SAFE_DELETE_ELE(fm2);
    isEnable = true;
    fm2 = new DFDisableTTCDialog(w1, file, isEnable);
//    fm2->m_isEnable=true;
    fm2->initMessageTitle();
    EXPECT_TRUE(fm2->messageTitle->text().contains(QLatin1String("enabled")));
    SAFE_DELETE_ELE(fm2);
    SAFE_DELETE_ELE(w1);
    SAFE_DELETE_ELE(kev);

}


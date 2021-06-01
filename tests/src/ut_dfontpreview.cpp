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

#include "dfontpreviewer.h"

#include "utils.h"

#include <QPainter>
#include <QPaintEvent>
#include <QRect>
#include <QRawFont>

#include <DPalette>
#include <DApplicationHelper>
#include "dfmdbmanager.h"

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
#include "commonheaderfile.h"
#include "interfaces/dcomworker.h"

int cuntch = 0;

namespace {
class TestDFontPreviewer : public testing::Test
{

protected:
    void SetUp()
    {
        fm = new DFontPreviewer();
    }
    void TearDown()
    {
        delete fm;
    }
    // Some expensive resource shared by all tests.
    QWidget *w = new QWidget;
    DFontPreviewer *fm;

public:
    bool stub_supportsCharacter(QChar character);
    bool stub_bool();

};

}

TEST_F(TestDFontPreviewer, checkInitData)
{
    fm->InitData();
    EXPECT_FALSE(fm->m_previewTexts.isEmpty());
    EXPECT_TRUE(fm->m_previewTexts.contains("汉体书写信息技术标准相容"));
}

bool TestDFontPreviewer::stub_supportsCharacter(QChar character)
{
    if(cuntch == 0){
        cuntch++;
        return false;
    }else {
        return true;
    }
}

bool TestDFontPreviewer::stub_bool()
{
    return false;
}


TEST_F(TestDFontPreviewer, checkOnPreviewFontChanged)
{
    fm->onPreviewFontChanged();
    EXPECT_FALSE(fm->m_previewTexts.isEmpty());
    Stub st;
    st.set((bool (QRawFont::*)(QChar character) const)ADDR(QRawFont, supportsCharacter), ADDR(TestDFontPreviewer, stub_supportsCharacter));
    cuntch = 0;
    fm->setPreviewFontPath("/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf");
    fm->onPreviewFontChanged();
    EXPECT_FALSE(fm->m_previewTexts.isEmpty());
}

TEST_F(TestDFontPreviewer, checkPaintevent)
{
    QRect r;
    QPaintEvent *p = new QPaintEvent(r);
    fm->paintEvent(p);

    SAFE_DELETE_ELE(p)
}


TEST_F(TestDFontPreviewer, removeUserAddFonts)
{

   GetFontListWorker getFontList(GetFontListWorker::AllInSquence);
   getFontList.removeUserAddFonts();

   Stub st;
   st.set((bool (DFMDBManager::*)(const QString &filePath))ADDR(DFMDBManager, isSystemFont), ADDR(TestDFontPreviewer, stub_bool));
   st.set((bool (QStringList::*)(const QString &str, Qt::CaseSensitivity cs) const)ADDR(QStringList, contains), ADDR(TestDFontPreviewer, stub_bool));
   getFontList.removeUserAddFonts();

}











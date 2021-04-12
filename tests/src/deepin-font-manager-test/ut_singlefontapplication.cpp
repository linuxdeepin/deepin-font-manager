/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     chendu <chendu@uniontech.com>
*
* Maintainer: chendu <chendu@uniontech.com>
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

#include "singlefontapplication.h"

//#include "utils.h"

//#include <QPainter>
//#include <QPaintEvent>
//#include <QRect>

//#include <DPalette>
//#include <DApplicationHelper>

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
#include "commonheaderfile.h"

//namespace {
//class TestSingleFontApplication : public testing::Test
//{

//protected:
//    void SetUp()
//    {
//        int argc = 0;
//        char **argv = nullptr;
//        fm = new SingleFontApplication(argc, argv);
//    }
//    void TearDown()
//    {
//        delete fm;
//    }
//    // Some expensive resource shared by all tests.
//    QWidget *w = new QWidget;
//    SingleFontApplication *fm;
//};
//}
//TEST_F(TestSingleFontApplication, initTest)
//{

//}

//TEST_F(TestDFontPreviewer, checkInitData)
//{
//    fm->InitData();
//    EXPECT_FALSE(fm->m_previewTexts.isEmpty());
//    EXPECT_TRUE(fm->m_previewTexts.contains("汉体书写信息技术标准相容"));
//}

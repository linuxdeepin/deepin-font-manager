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

#include "utils.h"
#include <singlefontapplication.h>

#include <QSize>
#include <QPixmap>
#include <QRawFont>

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
//#include "commonheaderfile.h"

namespace {
class TestUtils : public testing::Test
{

protected:
    void SetUp()
    {
        fm = new Utils();
    }
    void TearDown()
    {
        delete fm;
    }

    Utils *fm;

};
}

TEST_F(TestUtils, initTest)
{

}

TEST_F(TestUtils, checksuffixList)
{
    EXPECT_EQ(fm->suffixList().toStdString(), "Font Files (*.ttf *.ttc *.otf)");
}

TEST_F(TestUtils, checkisFontMimeType)
{
    EXPECT_EQ(fm->isFontMimeType("方正兰亭特黑_SC.otf"), false);
}

TEST_F(TestUtils, checkgetConfigPath)
{
    EXPECT_EQ(fm->getConfigPath().isEmpty(), false);
}


TEST_F(TestUtils, checkrenderSVG_001)
{
    EXPECT_EQ(fm->renderSVG("://ok.svg", QSize(32, 32)).isNull(), false);
}

TEST_F(TestUtils, checkconvertToPreviewString)
{
    EXPECT_EQ(fm->convertToPreviewString("", "hello").toStdString(), "hello");
}




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

#include "dfontpreviewitemdef.h"

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

namespace {
class TestFontData : public testing::Test
{

protected:

};
}

TEST_F(TestFontData, checkFontData)
{
    FontData fd("aaa", true, false, false, true, FontType::OTF, true);
    EXPECT_EQ(fd.isEnabled(), true);
    EXPECT_EQ(fd.isCollected(), false);
    EXPECT_EQ(fd.isChinese(), false);
    EXPECT_EQ(fd.isMonoSpace(), true);

    FontData fd2("aaa", true, false, false, true, "TrueType", true);
    EXPECT_EQ(fd2.isEnabled(), true);
    EXPECT_EQ(fd2.isCollected(), false);
    EXPECT_EQ(fd2.isChinese(), false);
    EXPECT_EQ(fd2.isMonoSpace(), true);

    fd2.setFontType(FontType::TTF);
    EXPECT_EQ(fd2.getFontType(), FontType::TTF);
    fd2.setFontType(FontType::TTC);
    EXPECT_EQ(fd2.getFontType(), FontType::TTC);
    fd2.setFontType(FontType::UNKNOWN);
    EXPECT_EQ(fd2.getFontType(), FontType::UNKNOWN);

}

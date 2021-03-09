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

#include "dfontloadthread.h"

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

#include "dfontinfomanager.h"
#include "dsqliteutil.h"

#include <QFile>
#include <QDir>
#include <QSignalSpy>

namespace {
class TestDdFontloadthread : public testing::Test
{

protected:
    void SetUp()
    {
        flt = new DFontLoadThread;
    }
    void TearDown()
    {
    }
    // Some expensive resource shared by all tests.
    DFontLoadThread *flt;
};

}

TEST_F(TestDdFontloadthread, checkRun)
{
    QSignalSpy spy(flt, SIGNAL(loadFinished(QByteArray)));
    flt->open("");
    flt->run();
    EXPECT_EQ(1, spy.count());

    flt->open("/usr/share/fonts/truetype/noto/NotoSansNKo-Regular.ttf");
    flt->run();
    EXPECT_EQ(2, spy.count());
    delete flt;
}

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
        delete flt;
    }
    // Some expensive resource shared by all tests.
    DFontLoadThread *flt;
};

static QString g_funcname;
QByteArray stub_readAll()
{
    g_funcname = __FUNCTION__;
    return QByteArray();
}
bool stub_open(QIODevice::OpenMode)
{
    g_funcname = __FUNCTION__;
    return true;
}
}

TEST_F(TestDdFontloadthread, checkopen)
{
    flt->open("testfilepath");
    EXPECT_TRUE(flt->m_filePath == "testfilepath");
}

TEST_F(TestDdFontloadthread, checkRun)
{
    QSignalSpy spy(flt, SIGNAL(loadFinished(QByteArray)));
    flt->open("");
    g_funcname.clear();
    flt->run();
    EXPECT_EQ(1, spy.count());
    EXPECT_TRUE(g_funcname.isEmpty());

    Stub s;
    s.set(ADDR(QIODevice, readAll), stub_readAll);
    typedef bool (*fptr)(QFile *, QIODevice::OpenMode);
    fptr QFile_open = (fptr)((bool(QFile::*)(QIODevice::OpenMode))&QFile::open);   //获取虚函数地址
    s.set(QFile_open, stub_open);

    flt->open("/usr123/share/fonts/truetype/noto/NotoSansNKo-Regular.ttf");
    flt->run();
    EXPECT_EQ(2, spy.count());
    EXPECT_TRUE(g_funcname == QLatin1String("stub_readAll"));
}

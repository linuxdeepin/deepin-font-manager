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

#include "getuseraddfontthread.h"
#include "dcomworker.h"
#include "dfontpreviewlistdatathread.h"

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
//#include "commonheaderfile.h"

namespace {
class TestGetUserAddFontThread : public testing::Test
{

protected:
    void SetUp()
    {
        fm = new GetUserAddFontThread();
    }
    void TearDown()
    {
        delete fm;
    }

    GetUserAddFontThread *fm;
};
}

TEST_F(TestGetUserAddFontThread, initTest)
{

}

void getChineseAndMonoFont_stub()
{
    return;
}
void onRefreshUserAddFont_stub(QList<DFontInfo> &fontInfoList)
{
    return;
}

QStringList getAllFclistPathList_stub()
{
    QStringList strlist;
    strlist << "111" << "22334";
    return strlist;
}

TEST_F(TestGetUserAddFontThread, checkrun)
{
    Stub stub;
    stub.set(ADDR(FontManager, getChineseAndMonoFont), getChineseAndMonoFont_stub);
    stub.set(ADDR(DFontPreviewListDataThread, onRefreshUserAddFont), onRefreshUserAddFont_stub);
    stub.set(ADDR(DFontInfoManager, getAllFclistPathList), getAllFclistPathList_stub);
    fm->run();
}

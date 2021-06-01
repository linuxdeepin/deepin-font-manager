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

#include "loadfontdatathread.h"
#include "dfontpreviewlistview.h"
#include "dfmdbmanager.h"
#include "../third-party/stub/stub.h"

#include <QTimer>
#include <QSignalSpy>

#include <gtest/gtest.h>



namespace {
class TestLoadFontDataThread : public testing::Test
{

protected:
    void SetUp()
    {
        fpm = new LoadFontDataThread(list);
    }
    void TearDown()
    {
        delete fpm;
    }
    // Some expensive resource shared by all tests.
    LoadFontDataThread *fpm;
    QList<QMap<QString, QString>> list;
};

QList<DFontPreviewItemData> stub_getFontInfo()
{
    QList<DFontPreviewItemData> list;
    DFontPreviewItemData data;
    data.appFontId = 1;

    list << data;

    return list;
}

}


TEST_F(TestLoadFontDataThread, checkDowork)
{
    DFontPreviewItemData ptiemdata;
    fpm->m_delFontInfoList.append(ptiemdata);
    fpm->run();


}

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

#include "dfontwidget.h"

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

#include <QFile>
#include <QTimer>
#include <QTranslator>
#include <QDebug>

namespace {
class TestDFontWidget : public testing::Test
{
protected:
    void SetUp()
    {
        fw = new DFontWidget();
    }
    void TearDown()
    {
        delete  fw;
    }
    // Some expensive resource shared by all tests.
    QString filepath = "/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf";
    DFontWidget *fw;
};
}

TEST_F(TestDFontWidget, initTest)
{

}

TEST_F(TestDFontWidget, checkSetFileUrl)
{
    QString filepath = "/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf";
    fw->setFileUrl(filepath);
    EXPECT_EQ(true, fw->m_spinner->isPlaying());
    EXPECT_EQ(true, fw->m_layout->currentIndex() == 0);
    EXPECT_EQ(true, fw->m_filePath == filepath);
    fw->m_thread->quit();
    fw->m_thread->wait();
}

TEST_F(TestDFontWidget, checkHandleFinished_dataEmpty)
{
    QByteArray data;
    fw->handleFinished(data);
    EXPECT_EQ(true, fw->m_layout->currentIndex() == 0);
    EXPECT_EQ(false, fw->m_preview->isVisible());
}

TEST_F(TestDFontWidget, checkHandleFinished_data)
{
    QString filepath = "/home/zhaogongqiang/Desktop/1048字体/ArkanaScriptRough.otf";
    QFile file(filepath);
    file.open(QIODevice::ReadOnly);
    QByteArray fileContent = file.readAll();
    fw->handleFinished(fileContent);
    EXPECT_EQ(true, fw->m_errMsg->text() == DApplication::translate("DFontWidget", "Broken file"));
}

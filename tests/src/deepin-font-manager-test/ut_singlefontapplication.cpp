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
#include "signalmanager.h"
#include "utils.h"
#include <QDebug>
#include <QCommandLineParser>

namespace {
class TestSingleFontApplication : public testing::Test
{

protected:
    void SetUp()
    {
        int argc = 0;
        char **argv = nullptr;
        //fm = new SingleFontApplication(argc, argv);
    }
    void TearDown()
    {
        //delete fm;
    }
    // Some expensive resource shared by all tests.
    QWidget *w = new QWidget;
    SingleFontApplication *fm;
public:
    void stub_process();
    QStringList stub_positionalArguments() const;
    bool stub_isFontMimeType(const QString &filepath);
};

static QString g_funcname;
void stub_show()
{
    g_funcname = __FUNCTION__;
}
void stub_activateWindow()
{
    g_funcname = __FUNCTION__;
}
}
TEST_F(TestSingleFontApplication, activateWindow)
{
    Stub s;
    s.set(ADDR(QWidget, show), stub_show);
    s.set(ADDR(QWidget, activateWindow), stub_activateWindow);

    int argc = 0;
    char **argv = nullptr;
    fm = new SingleFontApplication(argc, argv);
    fm->slotBatchInstallFonts();
    EXPECT_TRUE(g_funcname == QLatin1String("stub_show"));

    fm->activateWindow();
    EXPECT_TRUE(g_funcname == QLatin1String("stub_activateWindow"));

    QStringList fontlist;
    fontlist << "11";
    fm->installFonts(fontlist);
    EXPECT_TRUE(g_funcname == QLatin1String("stub_activateWindow"));

    fm->m_selectedFiles << "1" << "2";
    fm->onFontInstallFinished(fontlist);
    EXPECT_TRUE(fm->m_selectedFiles.isEmpty());

    Stub st;
    st.set((void (QCommandLineParser::*)(const QCoreApplication &))ADDR(QCommandLineParser, process), ADDR(TestSingleFontApplication, stub_process));
    EXPECT_TRUE(fm->parseCmdLine(true));

    EXPECT_TRUE(fm->parseCmdLine(false));

    st.set((QStringList(QCommandLineParser::*)() const)ADDR(QCommandLineParser, positionalArguments), ADDR(TestSingleFontApplication, stub_positionalArguments));
    EXPECT_FALSE(fm->parseCmdLine(false));

    st.set(ADDR(Utils, isFontMimeType), ADDR(TestSingleFontApplication, stub_isFontMimeType));
    EXPECT_TRUE(fm->parseCmdLine(false));
    EXPECT_EQ(fm->m_selectedFiles.count(), 2);
    //fm->quit();
}

void TestSingleFontApplication::stub_process()
{
    return ;
}

bool TestSingleFontApplication::stub_isFontMimeType(const QString &filepath)
{
    return true;
}

QStringList TestSingleFontApplication::stub_positionalArguments() const
{
    QStringList liststr;
    return liststr << "111" << "222";
}



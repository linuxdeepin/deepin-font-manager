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

#include "fontmanagercore.h"
#include "dcopyfilesmanager.h"


#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"

#include "signalmanager.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QSignalSpy>

const QString sysDir = QDir::homePath() + "/.local/share/fonts";

namespace {
class TestDfontmanager : public testing::Test
{

protected:
    void SetUp()
    {
        fm = FontManagerCore::instance();
    }
    void TearDown()
    {
    }
    // Some expensive resource shared by all tests.
    FontManagerCore *fm;
};

void stub_Handle()
{

}
}

TEST_F(TestDfontmanager, checkSetType)
{
    fm->setType(FontManagerCore::Install);
    EXPECT_EQ(fm->m_type, FontManagerCore::Install);
    //    delete fm;
}

TEST_F(TestDfontmanager, checkSetInstallFileList)
{
    QStringList list;
    list << "first" << "second" << "third" << "fouth";
    fm->setInstallFileList(list);
    EXPECT_EQ(fm->m_instFileList.count(), 4);
    EXPECT_EQ(true, fm->m_instFileList.contains("first"));

    list.clear();
    list << "fifth";
    fm->setInstallFileList(list);
    EXPECT_EQ(fm->m_instFileList.count(), 1);
    EXPECT_EQ(true, fm->m_instFileList.contains("fifth"));
//    delete fm;
}

TEST_F(TestDfontmanager, checkSetUnInstallFile)
{
    QStringList list;
    list << "first" << "second" << "third" << "fouth";
    fm->setUnInstallFile(list);
    EXPECT_EQ(fm->m_uninstFile.count(), 4);
    EXPECT_EQ(true, fm->m_uninstFile.contains("first"));
}


TEST_F(TestDfontmanager, checkRunInstall)
{
    Stub s;
    s.set(ADDR(FontManagerCore, handleInstall), stub_Handle);

    fm->setType(FontManagerCore::Install);
    fm->run();

    fm->setType(FontManagerCore::HalfwayInstall);
    fm->run();

    fm->setType(FontManagerCore::ReInstall);
    fm->run();
}

TEST_F(TestDfontmanager, checkRunUnInstall)
{
    Stub s;
    s.set(ADDR(FontManagerCore, handleUnInstall), stub_Handle);

    fm->setType(FontManagerCore::UnInstall);
    fm->run();

    fm->setType(FontManagerCore::DefaultNullType);
    fm->run();
}

TEST_F(TestDfontmanager, checkDoCmdInstall)
{
    Stub s;
    s.set(ADDR(FontManagerCore, doInstall), stub_Handle);
    QStringList list;

    fm->setType(FontManagerCore::Install);
    fm->doCmd(list);

    fm->setType(FontManagerCore::HalfwayInstall);
    fm->doCmd(list);

    fm->setType(FontManagerCore::ReInstall);
    fm->doCmd(list);
}

TEST_F(TestDfontmanager, checkDoCmdUnInstall)
{
    Stub s;
    s.set(ADDR(FontManagerCore, handleUnInstall), stub_Handle);

    QStringList list;
    fm->setType(FontManagerCore::UnInstall);
    fm->doCmd(list);

    fm->setType(FontManagerCore::DefaultNullType);
    fm->doCmd(list);
}

TEST_F(TestDfontmanager, checkHandleInstallAndDoInstall)
{
    Stub s;
    s.set(ADDR(DCopyFilesManager, copyFiles), stub_Handle);

    QSignalSpy spy(fm, SIGNAL(requestCancelInstall));

    fm->setType(FontManagerCore::Install);
    fm->m_instFileList << "first";
    fm->handleInstall();

    EXPECT_TRUE(spy.count() == 0);

    fm->cancelInstall();
    fm->handleInstall();

    EXPECT_TRUE(spy.count() == 0);
}

TEST_F(TestDfontmanager, checkHandleUnInstall)
{
    Stub s;
    s.set(ADDR(FontManagerCore, doCmd), stub_Handle);
    QStringList list;
    list << "first" << "endl";

    fm->setUnInstallFile(list);
    fm->handleUnInstall();
    EXPECT_EQ(true, fm->m_uninstFile.isEmpty());

}

TEST_F(TestDfontmanager, checkDoUnstall)
{
    QSignalSpy spy(fm, SIGNAL(uninstallFontFinished(QStringList)));
    QSignalSpy spys(fm, SIGNAL(uninstallFcCacheFinish()));

    QStringList list;

    QString str = QDir::homePath() + "/.local/share/fonts/Addictype";
    QString filePathOrig = QDir::homePath() + "/Desktop/1048字体/Addictype-Regular.otf";
//    QFile::copy(filePathOrig, str);
//    qDebug() << str << endl;
    QDir d;
    //新建文件夹并确认创建成功
    d.mkdir(str);
    EXPECT_EQ(true, d.exists());
    QFile::copy(filePathOrig, str + "/Addictype-Regular.otf");

    list << str + "/Addictype-Regular.otf";
    fm->doUninstall(list);
    QDir dd(str);
    EXPECT_EQ(false, dd.exists());

}

TEST_F(TestDfontmanager, checkOnInstallResultInstall)
{
    QSignalSpy spy(fm, SIGNAL(installFinished(int, const QStringList)));

    fm->m_instFileList.clear();
    fm->m_instFileList << "first";
    fm->setType(FontManagerCore::Install);
    fm->setCacheStatus(FontManagerCore::CacheNow);
    fm->onInstallResult("first", "");

    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(TestDfontmanager, checkOnInstallResultReInstall)
{
    QSignalSpy spy(fm, SIGNAL(reInstallFinished(int, const QStringList)));

    fm->m_instFileList.clear();
    fm->m_instFileList << "first";
    fm->setType(FontManagerCore::ReInstall);
    fm->setCacheStatus(FontManagerCore::CacheNow);
    fm->onInstallResult("first", "");

    EXPECT_TRUE(spy.count() == 1);

}


//TEST_F(TestDfontmanager, checkSetSystemFontCount)
//{
//    fm->setSystemFontCount(6);
//    EXPECT_EQ(6, fm->m_systemFontCount);
////    delete fm;
//}

TEST_F(TestDfontmanager, checkSetCacheStatus)
{
    fm->setCacheStatus(FontManagerCore::CacheNow);
    EXPECT_EQ(fm->m_CacheStatus, FontManagerCore::Install);
}

TEST_F(TestDfontmanager, checkCancelInstall)
{
    fm->cancelInstall();
    EXPECT_TRUE(fm->m_installCanceled);
}

TEST_F(TestDfontmanager, checkDoCache)
{
//    QSignalSpy spy(fm, SIGNAL(cacheFinish()));
    fm->doCache();
//    EXPECT_EQ(1, spy.count());
    delete fm;
}




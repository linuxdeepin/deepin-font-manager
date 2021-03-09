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

#include "views/dfinstallnormalwindow.h"
#include "views/dfontmgrmainwindow.h"

#include <gtest/gtest.h>

#include "../third-party/stub/stub.h"

#include "dfontmanager.h"
#include "globaldef.h"
#include "utils.h"
#include "dfmdbmanager.h"
#include "dfontpreviewlistdatathread.h"
#include "views/dfinstallerrordialog.h"


#include <QSignalSpy>
#include <QResizeEvent>
#include <QVBoxLayout>
#include <QTest>

#include <DApplication>
#include <DApplicationHelper>
#include <DFontSizeManager>
#include <DLog>

namespace {
class TestDFInstallNormalWindow : public testing::Test
{

protected:
    void SetUp()
    {
        iw = new DFInstallNormalWindow();
    }
    void TearDown()
    {
        delete iw;
    }
    // Some expensive resource shared by all tests.
    DFInstallNormalWindow *iw;
    QWidget *w = new QWidget;
};

QList<DFontPreviewItemData> stub_getFontModelList()
{
    QList<DFontPreviewItemData> list;

    DFontPreviewItemData data;
    data.fontInfo.filePath = "/usr/share/";
    data.fontInfo.familyName = "first";
    data.fontInfo.styleName = "second";

    list << data;
    return list;
}

DFontInfo  stub_getFontInfoError(const QString filePath)
{
//    qDebug() << filePath << endl;
    Q_UNUSED(filePath)

    DFontInfo f;
    f.isError = true;
    f.fullname = "a";
    f.filePath = "b";

    return f;
}

DFontInfo  stub_getFontInfoInstalled(const QString filePath)
{
//    qDebug() << filePath << endl;
    Q_UNUSED(filePath)

    DFontInfo f;
    f.isInstalled = true;
    f.fullname = "a";
    f.filePath = "b";

    return f;
}

DFontInfo  stub_getFontInfoNotInstalled(const QString filePath)
{
//    qDebug() << filePath << endl;
    Q_UNUSED(filePath)

    DFontInfo f;
    f.isInstalled = false;
    f.isError = false;

    f.fullname = "a";
    f.filePath = "b";

    return f;
}

bool stub_isSystemFont(DFontInfo &f)
{
    Q_UNUSED(f)

    return true;
}

bool stub_true()
{
    return true;
}


bool stub_isSystemFontFalse(DFontInfo &f)
{
    Q_UNUSED(f)

    return false;
}

void stub_respondToInstallOver()
{
    return;
}

void stub_docmd()
{
    return;
}

bool stub_ifNeedShowExceptionWindow()
{
    return true;
}

QString stub_isFontInfoExist()
{
    return "f";
}

}

TEST_F(TestDFInstallNormalWindow, checkGetAllSysfiles)
{
    Stub s;
    s.set(ADDR(DFontPreviewListDataThread, getFontModelList), stub_getFontModelList);

    iw->getAllSysfiles();
    EXPECT_TRUE(iw->m_AllSysFilesfamilyName.contains("firstsecond"));
}

TEST_F(TestDFInstallNormalWindow, checkVerifyFontFilesError)
{
    Stub s;
    s.set(ADDR(DFontInfoManager, getFontInfo), stub_getFontInfoError);
    iw->m_installFiles.append("first");
    iw->verifyFontFiles();
    EXPECT_TRUE(iw->m_damagedFiles.contains("first"));
}

TEST_F(TestDFInstallNormalWindow, checkVerifyFontFilesInstalled)
{
    Stub s;
    s.set(ADDR(DFontInfoManager, getFontInfo), stub_getFontInfoInstalled);

    Stub s1;
    s1.set(ADDR(DFInstallNormalWindow, isSystemFont), stub_isSystemFontFalse);

    iw->m_installFiles.append("first");
    iw->verifyFontFiles();
    EXPECT_TRUE(iw->m_installedFiles.contains("first"));
}

TEST_F(TestDFInstallNormalWindow, checkInlineFunction)
{

    DFontInfo info;
    info.familyName = "first";
    info.styleName = "second";
    EXPECT_TRUE(iw->getFamilyName(info) == "first");

    EXPECT_TRUE(iw->getFamilyStyleName(info) == "firstsecond");

}


TEST_F(TestDFInstallNormalWindow, checkVerifyFontFilesSystem)
{
    Stub s;
    s.set(ADDR(DFontInfoManager, getFontInfo), stub_getFontInfoNotInstalled);

    Stub s1;
    s1.set(ADDR(DFInstallNormalWindow, isSystemFont), stub_isSystemFont);

    iw->m_installFiles.append("first");
    iw->verifyFontFiles();
    EXPECT_TRUE(iw->m_systemFiles.contains("first"));
}

TEST_F(TestDFInstallNormalWindow, checkVerifyFontFilesElse)
{
    Stub s;
    s.set(ADDR(DFontInfoManager, getFontInfo), stub_getFontInfoNotInstalled);

    Stub s1;
    s1.set(ADDR(DFInstallNormalWindow, isSystemFont), stub_isSystemFontFalse);

    iw->m_installFiles.append("first");
    iw->verifyFontFiles();
    EXPECT_TRUE(iw->m_newInstallFiles.contains("first"));
    iw->verifyFontFiles();
    EXPECT_TRUE(iw->m_newInstallFiles.contains("first"));
}

TEST_F(TestDFInstallNormalWindow, checkIfNeedShowExceptionWindow)
{
    iw->m_isNeedSkipException = true;
    EXPECT_FALSE(iw->ifNeedShowExceptionWindow());

    iw->m_isNeedSkipException = false;
    iw->m_installState = DFInstallNormalWindow::InstallState::Install;
    iw->m_newInstallFiles.append("first");
    EXPECT_FALSE(iw->ifNeedShowExceptionWindow());
    iw->m_newInstallFiles.clear();

    iw->m_installState = DFInstallNormalWindow::InstallState::Install;
    iw->m_installedFiles.append("first");
    EXPECT_TRUE(iw->ifNeedShowExceptionWindow());

    iw->m_installState = DFInstallNormalWindow::InstallState::Install;
    iw->m_systemFiles.append("first");

    EXPECT_TRUE(iw->ifNeedShowExceptionWindow());

    iw->m_installState = DFInstallNormalWindow::InstallState::reinstall;
    iw->m_installedFiles.append("first");
    EXPECT_TRUE(iw->ifNeedShowExceptionWindow());

    iw->m_installState = DFInstallNormalWindow::InstallState::reinstall;
    iw->m_damagedFiles.append("first");
    EXPECT_TRUE(iw->ifNeedShowExceptionWindow());

}

TEST_F(TestDFInstallNormalWindow, checkIsSystemFont)
{
    iw->m_AllSysFilesfamilyName.clear();
    iw->m_AllSysFilesfamilyName.append("firstsecond");

    DFontInfo f;
    f.familyName = "first";
    f.styleName = "second";

    EXPECT_TRUE(iw->isSystemFont(f));
}

TEST_F(TestDFInstallNormalWindow, checkShowMessageFirst)
{
//    Stub s;
//    s.set(ADDR(DFontMgrMainWindow, respondToInstallOver), stub_respondToInstallOver);

    QSignalSpy spy0(iw->m_signalManager, SIGNAL(sendReInstallMessage(const QStringList &)));
    QSignalSpy spy1(iw->m_signalManager, SIGNAL(finishFontInstall(const QStringList &)));
    QSignalSpy spy2(iw->m_signalManager, SIGNAL(installOver(int)));

    iw->getInstallMessage = true;
    iw->getReInstallMessage = false;

    iw->checkShowMessage();
    EXPECT_TRUE(spy0.count() == 0);
    EXPECT_TRUE(spy1.count() == 1);
    EXPECT_TRUE(spy2.count() == 0);
}

TEST_F(TestDFInstallNormalWindow, checkGetNoSameFilesCount)
{
    QStringList list;
    list << "first";

    Stub s;
    s.set(ADDR(DFontInfoManager, getFontInfo), stub_getFontInfoNotInstalled);

    iw->getNoSameFilesCount(list);
}

TEST_F(TestDFInstallNormalWindow, checkResizeEvent)
{
    QResizeEvent *e = new QResizeEvent(QSize(), QSize());

    iw->resizeEvent(e);
}

TEST_F(TestDFInstallNormalWindow, checkPaintEvent)
{
    QPaintEvent *e = new QPaintEvent(QRect());

    iw->paintEvent(e);
}

TEST_F(TestDFInstallNormalWindow, checkCloseEvent)
{
    QCloseEvent *e = new QCloseEvent();

    iw->closeEvent(e);
}

TEST_F(TestDFInstallNormalWindow, checkBatchInstall)
{
    Stub s;
    s.set(ADDR(DFontManager, doCmd), stub_docmd);

    QSignalSpy spy(iw->m_signalManager, SIGNAL(finishFontInstall(const QStringList)));
    iw->m_newInstallFiles.clear();
    iw->batchInstall();
    EXPECT_TRUE(spy.count() == 1);

    Stub s1;
    s1.set(ADDR(DFInstallNormalWindow, ifNeedShowExceptionWindow), stub_ifNeedShowExceptionWindow);

    iw->m_newInstallFiles.append("first");
    iw->batchInstall();

}

TEST_F(TestDFInstallNormalWindow, checkBatchReInstall)
{
    Stub s;
    s.set(ADDR(DFontManager, doCmd), stub_docmd);

    Stub s1;
    s1.set(ADDR(DFMDBManager, isFontInfoExist), stub_isFontInfoExist);
    DFontPreviewListDataThread *dataThread = DFontPreviewListDataThread::instance();
    QSignalSpy spy(dataThread, SIGNAL(requestForceDeleteFiles(const QStringList &)));


    QStringList list;
    list << "first";

    iw->batchReInstall(list);

}

TEST_F(TestDFInstallNormalWindow, checkBatchHalfwayInstall)
{
    Stub s;
    s.set(ADDR(DFontManager, doCmd), stub_docmd);

    QSignalSpy spy(iw->m_signalManager, SIGNAL(updateInstallErrorListview(QStringList &, QStringList &, QStringList &, QStringList &)));

    QStringList list;
    list << "first";

    iw->m_newInstallFiles.append("fist");
    iw->batchHalfwayInstall(list);
    EXPECT_TRUE(spy.count() == 1);

}

TEST_F(TestDFInstallNormalWindow, checkBatchReInstallContinue)
{
    Stub s;
    s.set(ADDR(DFontManager, doCmd), stub_docmd);

    iw->m_installState = DFInstallNormalWindow::InstallState::reinstall;

    iw->m_installFiles.append("first");
    iw->batchReInstallContinue();
}

TEST_F(TestDFInstallNormalWindow, checkOnCancelInstall)
{
    Stub s;
    s.set(ADDR(DFInstallNormalWindow, onReInstallFinished), stub_docmd);

    iw->onCancelInstall();
    EXPECT_TRUE(iw->m_errCancelInstall);
}

TEST_F(TestDFInstallNormalWindow, checkOnContinueInstall)
{
    iw->onContinueInstall(QStringList());
    EXPECT_TRUE(iw->m_skipStateRecovery);
}

TEST_F(TestDFInstallNormalWindow, checkOnProgressChanged)
{
    iw->onProgressChanged("first", 0.01);

    EXPECT_TRUE(iw->m_currentFontLabel->text() == "first");
}

TEST_F(TestDFInstallNormalWindow, checkOnInstallFinished)
{
    QSignalSpy spy(iw->m_signalManager, SIGNAL(finishFontInstall(const QStringList &)));

    QStringList list;
    list << "first";

    iw->onInstallFinished(1, list);
    EXPECT_TRUE(spy.count() == 1) << spy.count();
}

TEST_F(TestDFInstallNormalWindow, checkOnReInstallFinished)
{
    QSignalSpy spy(iw->m_signalManager, SIGNAL(finishFontInstall(const QStringList &)));

    QStringList list;
    list << "first";
    iw->getInstallMessage = true;
    iw->onReInstallFinished(0, list);
    EXPECT_TRUE(spy.count() == 1) << spy.count();
}

TEST_F(TestDFInstallNormalWindow, checkSetSkipException)
{
    iw->setSkipException(true);

    EXPECT_TRUE(iw->m_isNeedSkipException);
}

TEST_F(TestDFInstallNormalWindow, checkBreakInstalltion)
{
    Stub s;
    s.set(ADDR(DFInstallErrorDialog, isVisible), stub_true);

    iw->m_pexceptionDlg = new DFInstallErrorDialog();
    iw->breakInstalltion();
}

TEST_F(TestDFInstallNormalWindow, checkKeyPressEvent)
{
    QTest::keyPress(iw, Qt::Key_Escape);
}

TEST_F(TestDFInstallNormalWindow, checkGetFamilyName)
{
    DFontInfo info;
    info.fullname = "first";

    EXPECT_TRUE("first" == iw->getFamilyName(info));
}









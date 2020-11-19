#include "views/dfquickinstallwindow.h"


#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
#include "utils.h"

#include "views/dfinstallnormalwindow.h"
#include "dfontinfomanager.h"
#include "dfontmanager.h"
#include "dfmdbmanager.h"
#include "utils.h"
#include "globaldef.h"
#include "interfaces/dfontpreviewer.h"

#include <DLog>
#include <DPalette>
#include <DPushButton>
#include <DTitlebar>
#include <DApplicationHelper>
#include <DWidgetUtil>

#include <QTest>
#include <QSignalSpy>
#include <QFontDatabase>
#include <QResizeEvent>
#include <QVBoxLayout>

namespace {
class TestDFQuickInstallWindow : public testing::Test
{

protected:
    void SetUp()
    {
        fqi = new DFQuickInstallWindow(QStringList(), w);
    }
    void TearDown()
    {
        delete fqi;
    }

    // Some expensive resource shared by all tests.
    QWidget *w = new QWidget;
    DFQuickInstallWindow *fqi;
};

//参数化测试
class TestcheckInitPreviewFont : public::testing::TestWithParam<QString>
{
public:
    QWidget *w = new QWidget;
    DFQuickInstallWindow *fqi = new DFQuickInstallWindow(QStringList(), w);
};

DFontInfo stub_getFontInfoFirst(const QString &filePath)
{
    Q_UNUSED(filePath)
    DFontInfo f;

    f.isError = false;
    f.isInstalled = true;

    f.familyName = "familyname";
    return f;
}

DFontInfo stub_getFontInfoSecond(const QString &filePath)
{
    Q_UNUSED(filePath)
    DFontInfo f;

    f.isError = false;
    f.isInstalled = false;

    return f;
}

DFontInfo stub_getFontInfoThird(const QString &filePath)
{
    Q_UNUSED(filePath)
    DFontInfo f;

    f.isError = false;
    f.isInstalled = false;
    f.styleName = "first";

    return f;
}



void stub_installFont(const QStringList &files)
{
    Q_UNUSED(files)
    return ;
}
}

TEST_F(TestDFQuickInstallWindow, checkOnFileSelectedFirst)
{
    QStringList list;
    list << "aaaaaaaaaa";
    fqi->onFileSelected(list);

    Stub s;
    s.set(ADDR(DFontInfoManager, getFontInfo), stub_getFontInfoFirst);
    fqi->onFileSelected(list);
}

TEST_F(TestDFQuickInstallWindow, checkOnFileSelectedSecond)
{
    QStringList list;
    list << "aaaaaaaaaa";
    fqi->onFileSelected(list);

    Stub s;
    s.set(ADDR(DFontInfoManager, getFontInfo), stub_getFontInfoSecond);
    fqi->onFileSelected(list);
}

TEST_F(TestDFQuickInstallWindow, checkresizeEvent)
{
    QSize s;
    QResizeEvent *e = new QResizeEvent(s, s);

    fqi->resizeEvent(e);
}


TEST_F(TestDFQuickInstallWindow, checkOnInstallBtnClicked)
{
    QSignalSpy spy(fqi, SIGNAL(quickInstall()));

    Stub s;
    s.set(ADDR(DFQuickInstallWindow, installFont), stub_installFont);

    QTest::mouseClick(fqi->m_actionBtn, Qt::LeftButton);



    EXPECT_TRUE(spy.count() == 1);
}

////installFont exec之后会崩溃
//TEST_F(TestDFQuickInstallWindow, checkInstallFont)
//{

//    QStringList list;
//    list << "a";
//    fqi->installFont(list);
//}

TEST_F(TestDFQuickInstallWindow, checkOnFontInstallFinishedFirst)
{
    QSignalSpy spy(fqi, SIGNAL(requestShowMainWindow(const QStringList &)));

    fqi->m_installFiles << "first";

    Stub s;
    s.set(ADDR(DFontInfoManager, getFontInfo), stub_getFontInfoSecond);

    fqi->onFontInstallFinished();

    EXPECT_TRUE(spy.count() == 1);
}

TEST_F(TestDFQuickInstallWindow, checkOnFontInstallFinishedSecond)
{
    QSignalSpy spy(fqi, SIGNAL(requestShowMainWindow(const QStringList &)));

    fqi->m_installFiles << "second";

    Stub s;
    s.set(ADDR(DFontInfoManager, getFontInfo), stub_getFontInfoThird);

    fqi->onFontInstallFinished();

    EXPECT_TRUE(spy.count() == 1);
}

TEST_P(TestcheckInitPreviewFont, checkInitPreviewFont)
{
    QString n =  GetParam();
    DFontInfo f;
    f.styleName = QString(n);

    fqi->InitPreviewFont(f);
}

INSTANTIATE_TEST_CASE_P(HandleTrueReturn, TestcheckInitPreviewFont, testing::Values("Italic", "Regular", "ExtraLight", "ExtraBold",
                                                                                    "DemiBold", "Bold", "Light", "Thin", "Medium", "Black"));




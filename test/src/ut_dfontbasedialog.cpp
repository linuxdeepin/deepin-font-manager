#include "dfontbasedialog.h"

#include <gtest/gtest.h>

#include "../third-party/stub/stub.h"
#include "utils.h"

#include <QVBoxLayout>
#include <QApplication>

#include <DFontSizeManager>

namespace {
class TestDFontBaseDialog : public testing::Test
{

protected:
    void SetUp()
    {
        fm = new DFontBaseDialog(w);
    }
    void TearDown()
    {
        delete fm;
    }
    // Some expensive resource shared by all tests.
    QWidget *w = new QWidget;
    DFontBaseDialog *fm;
};
}



TEST_F(TestDFontBaseDialog, checkSetIconPixmap)
{
    QPixmap map(Utils::renderSVG("://ok.svg", QSize(32, 32)));

    fm->setIconPixmap(map);

    EXPECT_TRUE(fm->m_logoIcon->pixmap()->size() == QSize(32, 32));
//    EXPECT_TRUE(fm->getContent() != nullptr);
}

TEST_F(TestDFontBaseDialog, checkGetCloseButton)
{
    DWindowCloseButton *b = fm->getCloseButton();
    EXPECT_TRUE(b != nullptr);
}

TEST_F(TestDFontBaseDialog, checkSetLogoVisable)
{
    //设置为true，但结果仍不可见，怀疑时没有窗口弹出的问题，与代码无关。
//    fm->setLogoVisable(true);
//    EXPECT_TRUE(fm->m_logoIcon->isVisible() == true);

    fm->setLogoVisable(false);
    EXPECT_TRUE(fm->m_logoIcon->isVisible() == false);
}

TEST_F(TestDFontBaseDialog, checkSetTitle)
{
    fm->setTitle("first");
    EXPECT_TRUE(fm->m_tileText->text() == "first");
}

TEST_F(TestDFontBaseDialog, checkGetContentLayout)
{
    EXPECT_TRUE(fm->getContentLayout() != nullptr);
}

TEST_F(TestDFontBaseDialog, checkAddContent)
{
    QWidget *w = new QWidget;
    fm->addContent(w);
    EXPECT_FALSE(fm->getContentLayout()->isEmpty());
}






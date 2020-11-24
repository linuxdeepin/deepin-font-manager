#include "views/fonticontext.h"


#include <DStyle>
#include <DFontSizeManager>

#include <QBitmap>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>
#include <QImage>
#include <QDebug>

#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
#include "utils.h"


namespace {
class TestFontIconText : public testing::Test
{

protected:
    void SetUp()
    {
        fit = new FontIconText("");
    }
    void TearDown()
    {
        delete fit;
    }

    // Some expensive resource shared by all tests.
    QWidget *w = new QWidget;
    FontIconText *fit;
};

class TestcheckStyleName : public::testing::TestWithParam<QString>
{
public:
    FontIconText *fit = new FontIconText("");
};


}

TEST_F(TestFontIconText, checkPaintEvent)
{
    QPaintEvent *e = new QPaintEvent(QRect());

    fit->setContent(true);
    fit->paintEvent(e);

    fit->setContent(false);
    fit->paintEvent(e);
}


//checkStyleName 函数出错 20200806
TEST_P(TestcheckStyleName, checkStyleName_Is_Normal)
{
    QString n =  GetParam();

    fit->setFontName("first", n);
}


INSTANTIATE_TEST_CASE_P(HandleTrueReturn, TestcheckStyleName, testing::Values("Italic", "Regular", "Light", "Thin", "ExtraLight",
                                                                              "ExtraBold", "Medium", "DemiBold", "Black"));







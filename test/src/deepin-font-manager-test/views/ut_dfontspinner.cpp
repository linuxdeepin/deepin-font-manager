#include "views/dfontspinner.h"


#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
#include "utils.h"

#include <QPaintEvent>
#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>

#include <DFontSizeManager>

namespace {
class TestDFontSpinner : public testing::Test
{

protected:
    void SetUp()
    {
        fs = new DFontSpinner(w);
    }
    void TearDown()
    {
        delete fs;
    }
    // Some expensive resource shared by all tests.
    QWidget *w = new QWidget;
    DFontSpinner *fs;
};
}


TEST_F(TestDFontSpinner, checkStart)
{
    fs->start();

    fs->stop();
}

TEST_F(TestDFontSpinner, checkPaintEvent)
{
    QRect f;

    QPaintEvent *paint = new QPaintEvent(f);

    fs->paintEvent(paint);
}

//TEST_F(TestDFontSpinner, checkSetBackgroundColor)
//{
//    QColor c(255, 255, 255);
//    fs->setBackgroundColor(c);

//    QPalette pal = fs->palette();
//    QColor c2 = pal.color(QPalette::Background);
//    EXPECT_TRUE(c == c2);
//}

TEST_F(TestDFontSpinner, checkChangeEvent)
{
    QEvent *e = new QEvent(QEvent::PaletteChange);
    fs->changeEvent(e);
}




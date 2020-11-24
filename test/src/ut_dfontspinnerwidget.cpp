#include "views/dfontspinnerwidget.h"


#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
#include "utils.h"

#include "views/dfontspinnerwidget.h"
#include "views/dfontspinner.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <DApplication>
#include <QDebug>
#include <DWidget>
#include <QLabel>

namespace {
class TestDFontSpinnerWidget : public testing::Test
{

protected:
    void SetUp()
    {
        fsp = new DFontSpinnerWidget(w);
    }
    void TearDown()
    {
        delete fsp;
    }
    // Some expensive resource shared by all tests.
    QWidget *w = new QWidget;
    DFontSpinnerWidget *fsp;
};
}

TEST_F(TestDFontSpinnerWidget, checkStartAndStop)
{
    fsp->spinnerStart();

    fsp->spinnerStop();
}

TEST_F(TestDFontSpinnerWidget, checkSetStyles)
{
    fsp->setStyles(DFontSpinnerWidget::SpinnerStyles::Load);
    EXPECT_TRUE(fsp->m_label->text() == "Loading fonts, please wait...");

    fsp->setStyles(DFontSpinnerWidget::SpinnerStyles::Delete);
    EXPECT_TRUE(fsp->m_label->text() == "Deleting fonts, please wait...");

    fsp->setStyles(DFontSpinnerWidget::SpinnerStyles::NoLabel);
    EXPECT_TRUE(fsp->m_label->text() == QString());
}

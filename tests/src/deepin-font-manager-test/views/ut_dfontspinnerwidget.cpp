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

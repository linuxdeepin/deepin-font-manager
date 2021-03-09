/*
 *
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

#pragma once

#include <DWidget>

#include <QLabel>

class DFontSpinner;

class DFontSpinnerWidget : public Dtk::Widget::DWidget
{
public:
    /*************************************************************************
     <Enum>      SpinnerStyles
     <Description>   加载动画样式
     <Author>
     <Value>
        <Value1>   StartupLoad      Description:启动应用加载
        <Value2>   Load             Description:安装后加载
        <Value3>   Delete           Description:删除后加载
        <Value4>   NoLabel          Description:无加载提示信息
     <Note>          null
    *************************************************************************/
    enum SpinnerStyles {
        StartupLoad,
        Load,
        Delete,
        NoLabel,
    };


public:
    explicit DFontSpinnerWidget(QWidget *parent = nullptr,  SpinnerStyles styles = SpinnerStyles::Load);
    void initUI();

    void spinnerStart();
    void spinnerStop();
    ~DFontSpinnerWidget();

    void setStyles(SpinnerStyles styles);

private:

    SpinnerStyles m_Styles;
    DFontSpinner *m_spinner = nullptr;
    QLabel *m_label = nullptr;
};

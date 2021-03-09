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

#include <QWidget>

class DFontSpinnerPrivate;

class DFontSpinner : public QWidget
{
    Q_OBJECT
public:
    explicit DFontSpinner(QWidget *parent = nullptr);
    ~DFontSpinner() override;

public Q_SLOTS:
    void start();
    void stop();

protected:
    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE;
    void changeEvent(QEvent *e) override;

private:
// d_ptr与Qobject库中一个protected权限的指针变量重名，导致会出现警告
//    DFontSpinnerPrivate *const d_ptr {nullptr};
    DFontSpinnerPrivate *const m_ptr {nullptr};
    Q_DECLARE_PRIVATE(DFontSpinner)
};

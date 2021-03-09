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

#include <DSvgRenderer>

#include <QWidget>

/*************************************************************************
 <Class>         FontIconText
 <Description>   构造函数-构造字体图标对象
 <Author>
 <Note>          null
*************************************************************************/
class FontIconText : public QWidget
{
    Q_OBJECT
public:
    explicit FontIconText(const QString picPath, QWidget *parent = nullptr);
    //设置字体名
    void setFontName(const QString &familyName, const QString &styleName);
    //设置是否为ttf字体属性
    void setContent(bool isTtf);

protected:
    //重写绘图事件
    void paintEvent(QPaintEvent *event);
signals:

public slots:
private:
    QString m_picPath;
    DTK_GUI_NAMESPACE::DSvgRenderer *render;
    QFont m_font;
    bool m_isTtf;
};

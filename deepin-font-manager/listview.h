/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
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

#ifndef LISTVIEW_H
#define LISTVIEW_H

#include <DSimpleListView>
#include "listitem.h"

DWIDGET_USE_NAMESPACE

class ListView : public DSimpleListView
{
    Q_OBJECT

public:
    ListView(QWidget *parent = nullptr);
    ~ListView();

signals:
    void closeBtnClicked(DSimpleListItem *item);

public slots:
    void handleMouseHoverChanged(DSimpleListItem* oldItem, DSimpleListItem* newItem, int columnIndex, QPoint pos);
    void handleMousePressChanged(DSimpleListItem* item, int columnIndex, QPoint pos);
    void handleMouseReleaseChanged(DSimpleListItem* item, int columnIndex, QPoint pos);

private:
    void initTheme();

protected:
    void leaveEvent(QEvent *);

private:
    ListItem *m_currentItem;
};

#endif

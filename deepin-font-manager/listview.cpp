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

#include "listview.h"
#include "listitem.h"
#include "dthememanager.h"
#include <QDebug>

ListView::ListView(QWidget *parent)
    : DSimpleListView(parent),
      m_currentItem(nullptr)
{
    // enable frame and radius.
    setFrame(true);
    setClipRadius(8);

    // set row height.
    setRowHeight(56);

    // set fixed height.
    setFixedHeight(280);

    // init theme.
    initTheme();

    connect(this, &ListView::mouseHoverChanged, this, &ListView::handleMouseHoverChanged);
    connect(this, &ListView::mousePressChanged, this, &ListView::handleMousePressChanged);
    connect(this, &ListView::mouseReleaseChanged, this, &ListView::handleMouseReleaseChanged);
    connect(DThemeManager::instance(), &DThemeManager::themeChanged, this, &ListView::initTheme);
}

ListView::~ListView()
{
}

void ListView::handleMouseHoverChanged(DSimpleListItem* oldItem, DSimpleListItem* newItem, int columnIndex, QPoint pos)
{
    if (oldItem == nullptr || !oldItem->sameAs(newItem)) {
        if (oldItem != nullptr) {
            (static_cast<ListItem *>(oldItem))->setCloseButtonStatus(CloseButtonStatus::Hide);
        }

        if (newItem != nullptr) {
            (static_cast<ListItem *>(newItem))->setCloseButtonStatus(CloseButtonStatus::Normal);
        }

        repaint();
    } else {
        if ((static_cast<ListItem *>(newItem))->isHoverCloseButton(pos)) {
            (static_cast<ListItem *>(newItem))->setCloseButtonStatus(CloseButtonStatus::Hover);
            repaint();
        } else {
            (static_cast<ListItem *>(newItem))->setCloseButtonStatus(CloseButtonStatus::Normal);
            repaint();
        }
    }

    m_currentItem = static_cast<ListItem *>(newItem);
}

void ListView::handleMousePressChanged(DSimpleListItem* item, int columnIndex, QPoint pos)
{
    if (item != nullptr) {
        if ((static_cast<ListItem *>(item))->isHoverCloseButton(pos)) {
            (static_cast<ListItem *>(item))->setCloseButtonStatus(CloseButtonStatus::Press);
            repaint();
        }
    }
}

void ListView::handleMouseReleaseChanged(DSimpleListItem* item, int columnIndex, QPoint pos)
{
    if ((static_cast<ListItem *>(item))->isHoverCloseButton(pos)) {
        (static_cast<ListItem *>(item))->setCloseButtonStatus(CloseButtonStatus::Hover);
        emit closeBtnClicked(item);
        repaint();
    } else {
        (static_cast<ListItem *>(item))->setCloseButtonStatus(CloseButtonStatus::Normal);
        repaint();
    }
}

void ListView::initTheme()
{
    if (DThemeManager::instance()->theme() == "light") {
        backgroundColor = "#FFFFFF";
        backgroundOpacity = 0.03;

        frameColor = "#000000";
        frameOpacity = 0.1;

        scrollbarColor = "#101010";
        scrollbarNormalOpacity = 0.5;
        scrollbarHoverOpacity = 0.7;
        scrollbarPressOpacity = 0.8;

        scrollbarFrameNormalOpacity = 0;
        scrollbarFrameHoverOpacity = 0;
        scrollbarFramePressOpacity = 0;
    } else {
        backgroundColor = "#ffffff";
        backgroundOpacity = 0.03;

        frameColor = "#000000";
        frameOpacity = 0;

        scrollbarColor = "#ffffff";
        scrollbarNormalOpacity = 0.2;
        scrollbarHoverOpacity = 0.4;
        scrollbarPressOpacity = 0.15;

        scrollbarFrameNormalOpacity = 0.05;
        scrollbarFrameHoverOpacity = 0.1;
        scrollbarFramePressOpacity = 0.05;
    }
}

void ListView::leaveEvent(QEvent *e)
{
    if (m_currentItem) {
        m_currentItem->setCloseButtonStatus(CloseButtonStatus::Hide);
    }

    DSimpleListView::leaveEvent(e);
}

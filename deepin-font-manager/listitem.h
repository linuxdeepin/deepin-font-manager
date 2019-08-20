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

#ifndef LISTITEM_H
#define LISTITEM_H

#include <DSimpleListItem>
#include "dfontinfomanager.h"

DWIDGET_USE_NAMESPACE

enum CloseButtonStatus {
    Hide, Normal, Hover, Press
};

enum FontStatus {
    None, Installing, Installed
};

class ListItem : public DSimpleListItem
{
    Q_OBJECT

public:
    ListItem(DFontInfo *fontInfo, QPixmap *iconPixmap, QPixmap *closeNormalPixmap, QPixmap *closeHoverPixmap, QPixmap *closePressPixmap);
    ~ListItem();

    void initTheme();

    bool sameAs(DSimpleListItem *item);
    void setFontStatus(FontStatus status);
    void drawBackground(QRect rect, QPainter *painter, int index, bool isSelect, bool isHover);
    void drawForeground(QRect rect, QPainter *painter, int column, int index, bool isSelect, bool isHover);
    DFontInfo *getFontInfo() { return m_fontInfo; }
    FontStatus getFontStatus() { return m_fontStatus; };
    QString getFilePath();
    void setCloseButtonStatus(CloseButtonStatus status);
    bool isHoverCloseButton(QPoint pos);

private:
    DFontInfo *m_fontInfo;
    FontStatus m_fontStatus;
    QPixmap *m_iconPixmap;
    QPixmap *m_closeNormalPixmap;
    QPixmap *m_closePressPixmap;
    QPixmap *m_closeHoverPixmap;
    CloseButtonStatus m_close_button_status;

    int closeButtonPadding = 35;
    QRect m_rect;

    QString m_familyColor;
};

#endif

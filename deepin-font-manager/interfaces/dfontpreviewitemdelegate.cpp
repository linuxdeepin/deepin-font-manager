// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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

#include "dfontpreviewitemdelegate.h"
#include "dfontpreviewlistview.h"
#include "globaldef.h"
#include "utils.h"

#include <DApplication>
#if QT_VERSION_MAJOR <= 5
#include <DApplicationHelper>
#endif
#include <DStyleHelper>

#include <QPainter>
#include <QCheckBox>
#include <QPainterPath>

DWIDGET_USE_NAMESPACE

const int CHECKBOX_SIZE = 20;

const int COLLECT_ICON_SIZE = 24;
const int COLLECT_ICON_RIGHT_MARGIN = 19;
const int COLLECT_ICON_TOP_MARGIN = 10;
const int COLLECT_ICON_TOP_COMPACT_MARGIN = 1;

const int FONT_NAME_HEIGHT = 30;
const int FONT_NAME_COMPACT_HEIGHT = 21;
const int FONT_NAME_LEFT_MARGIN = 50;
const int FONT_NAME_TOP_MARGIN = 3;
const int FONT_PREVIEW_LEFT_MARGIN = 50;
const int FONT_PREVIEW_RIGHT_MARGIN = COLLECT_ICON_SIZE + COLLECT_ICON_RIGHT_MARGIN;
const int FONT_PREVIEW_TOP_MARGIN = FONT_NAME_HEIGHT + FONT_NAME_TOP_MARGIN;
const int FONT_PREVIEW_TOP_COMPACT_MARGIN = FONT_NAME_COMPACT_HEIGHT + 5;
const int FONT_PREVIEW_BOTTOM_MARGIN = 10;
const int FONT_PREVIEW_BOTTOM_COMPACT_MARGIN = 5;

DFontPreviewItemDelegate::DFontPreviewItemDelegate(QAbstractItemView *parent)
    : QStyledItemDelegate(parent)
    , m_parentView(qobject_cast<DFontPreviewListView *>(parent))
{
    qDebug() << "Creating DFontPreviewItemDelegate";
    parent->viewport()->installEventFilter(this);
    qDebug() << "Event filter installed on viewport";
}

/*************************************************************************
 <Function>      paintForegroundCheckBox
 <Description>   绘制字体预览列表中的复选框
 <Author>        null
 <Input>
    <param1>     painter            Description:绘制类指针
    <param2>     option             Description:listview中的一行成员
    <param3>     itemdata           Description:对应aption行的数据
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/

void DFontPreviewItemDelegate::paintForegroundCheckBox(QPainter *painter, const QStyleOptionViewItem &option, const FontData &itemData) const
{
    // qDebug() << "Entering function: DFontPreviewItemDelegate::paintForegroundCheckBox";
    int checkBoxWidth = CHECKBOX_SIZE - 4;
    int checkBoxHeight = CHECKBOX_SIZE - 4;

    QRect rect = QRect(option.rect.x() + 25, option.rect.y() + 10, checkBoxWidth, checkBoxHeight);

#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
        // qDebug() << "Using compact mode for checkbox positioning";
        rect = QRect(option.rect.x() + 25, option.rect.y() + 6, checkBoxWidth, checkBoxHeight);
    }
#endif

    QStyleOptionButton checkBoxOption;
    checkBoxOption.state |= QStyle::State_Enabled;
    if (itemData.isEnabled()) {
        // qDebug() << "Checkbox is enabled, setting State_On";
        checkBoxOption.state |= QStyle::State_On;
    } else {
        // qDebug() << "Checkbox is disabled, setting State_Off";
        checkBoxOption.state |= QStyle::State_Off;
    }
    checkBoxOption.rect = rect;

    QCheckBox checkBox;
    DApplication::style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &checkBoxOption, painter, &checkBox);
    // qDebug() << "Exiting function: DFontPreviewItemDelegate::paintForegroundCheckBox";
}


/*************************************************************************
 <Function>      paintForegroundFontName
 <Description>   绘制字体预览列表中的字体名字
 <Author>        null
 <Input>
    <param1>     painter            Description:绘制类指针
    <param2>     option             Description:listview中的一行成员
    <param3>     itemdata           Description:对应aption行的数据
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewItemDelegate::paintForegroundFontName(QPainter *painter, const QStyleOptionViewItem &option, const FontData &itemData) const
{
    // qDebug() << "Entering function: DFontPreviewItemDelegate::paintForegroundFontName";
    QFont nameFont = painter->font();
    nameFont.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T6));
    painter->setFont(nameFont);

#if QT_VERSION_MAJOR > 5
    DGuiApplicationHelper *dAppHelper = DGuiApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
#else
    DApplicationHelper *dAppHelper = DApplicationHelper::instance();
    DPalette palette = dAppHelper->applicationPalette();
#endif
    QColor fillColor = palette.color(DPalette::TextTips);
    // SP3--禁用置灰(539)
    if (!itemData.isEnabled()) {
        // qDebug() << "Font is disabled, setting alpha to 0.6";
        fillColor.setAlphaF(0.6);
    }
    painter->setPen(QPen(fillColor));

    QRect fontNameRect = QRect(option.rect.x() + FONT_NAME_LEFT_MARGIN, option.rect.y() + FONT_NAME_TOP_MARGIN,
                               option.rect.width() - 20, FONT_NAME_HEIGHT);
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
        // qDebug() << "Using compact mode for font name height";
        fontNameRect.setHeight(FONT_NAME_COMPACT_HEIGHT);
    }
#endif

    QFontMetrics mt(nameFont);//特殊图案字体下截断字体名称/*UT000539*/
    QString elidedText = mt.elidedText(itemData.strFontName, Qt::ElideRight, option.rect.width() - 120);
    painter->drawText(fontNameRect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);
    // qDebug() << "Exiting function: DFontPreviewItemDelegate::paintForegroundFontName";
}

/*************************************************************************
 <Function>      paintForegroundCollectIcon
 <Description>   绘制字体预览列表中的收藏图标
 <Author>        null
 <Input>
    <param1>     painter            Description:绘制类指针
    <param2>     option             Description:listview中的一行成员
    <param3>     itemdata           Description:对应aption行的数据
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewItemDelegate::paintForegroundCollectIcon(QPainter *painter, const QStyleOptionViewItem &option, const FontData &itemData) const
{
    // qDebug() << "Entering function: DFontPreviewItemDelegate::paintForegroundCollectIcon";
    DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
    QString strImgPrefix = (DGuiApplicationHelper::DarkType == appHelper->themeType()) ? QString("dark_") : QString("");

    QString iconStatus;
    IconStatus status = itemData.getHoverState();
    if (IconHover == status) {
        // qDebug() << "Icon hover state detected";
        iconStatus = QString("hover");
    } else if (IconPress == status) {
        // qDebug() << "Icon press state detected";
        iconStatus = QString("press");
        strImgPrefix = "";
    } else {
        // qDebug() << "Icon normal state detected";
        iconStatus = QString("normal");
    }

    //fix bug-14120 -> clear hover state, if mouse leaved
//    iconStatus = (false == option.state.testFlag(QStyle::State_MouseOver)) ? QString("normal") : iconStatus;

    QString strImageSrc;
    QPixmap pixmap;
    if (itemData.isCollected()) {
        // qDebug() << "Font is collected, using collection icon";
        strImageSrc = QString("://%1collection_%2.svg").arg(strImgPrefix).arg(iconStatus);
        pixmap = Utils::renderSVG(strImageSrc, QSize(COLLECT_ICON_SIZE, COLLECT_ICON_SIZE));
    } else {
        // qDebug() << "Font is not collected, using uncollection icon";
        strImageSrc = QString("://%1uncollection_%2.svg").arg(strImgPrefix).arg(iconStatus);
        pixmap = Utils::renderSVG(strImageSrc, QSize(COLLECT_ICON_SIZE, COLLECT_ICON_SIZE));
    }

    QRect collectIconRealRect = QRect(option.rect.right() - COLLECT_ICON_SIZE - COLLECT_ICON_RIGHT_MARGIN,
                                      option.rect.top() + COLLECT_ICON_TOP_MARGIN,
                                      COLLECT_ICON_SIZE, COLLECT_ICON_SIZE);
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
        // qDebug() << "Using compact mode for collect icon positioning";
        collectIconRealRect = QRect(option.rect.right() - COLLECT_ICON_SIZE - COLLECT_ICON_RIGHT_MARGIN,
                                              option.rect.top() + COLLECT_ICON_TOP_COMPACT_MARGIN,
                                              COLLECT_ICON_SIZE, COLLECT_ICON_SIZE);
    }
#endif


    // 获取系统活动色
    QColor bgColor = Dtk::Gui::DGuiApplicationHelper::instance()->applicationPalette().highlight().color();

    // 针对鼠标按下时进行处理
    if (IconPress == status) {
        // qDebug() << "Processing icon press state with color transformation";
        QImage originImage = pixmap.toImage();  // 将原始图片转为image
        int iWidth = originImage.width();
        int iHeight = originImage.height();

        QImage tempImage(iWidth, iHeight, QImage::Format_ARGB32); // 创建临时image

        // 对每一个像素点进行设置RGBA
        for (int w = 0; w < iWidth; ++w) {
            for (int h = 0; h < iHeight; ++h) {
                bgColor.setAlpha(qAlpha(originImage.pixel(w, h)));    // 替换RGB
                tempImage.setPixel(w, h, bgColor.rgba());             // 设置色值
            }
        }

        painter->drawPixmap(collectIconRealRect, QPixmap::fromImage(tempImage));    // 使用新图片绘制
    } else {
        // qDebug() << "Using original pixmap for drawing";
        painter->drawPixmap(collectIconRealRect, pixmap);   // 使用原始图片绘制
    }

    // qDebug() << "Exiting function: DFontPreviewItemDelegate::paintForegroundCollectIcon";
}

/*************************************************************************
 <Function>      adjustPreviewRect
 <Description>   调整预览区域
 <Author>        null
 <Input>
    <param1>     bgRect            Description:调整前的区域
 <Return>        QRect             Dscription:调整后的区域
 <Note>          null
*************************************************************************/

QRect DFontPreviewItemDelegate::adjustPreviewRect(const QRect bgRect) const
{
    // qDebug() << "Entering function: DFontPreviewItemDelegate::adjustPreviewRect";
    QRect fontPreviewRect;
    int fontRectWidth = bgRect.width() - FONT_PREVIEW_LEFT_MARGIN - FONT_PREVIEW_RIGHT_MARGIN;
    fontPreviewRect = QRect(bgRect.x() + FONT_PREVIEW_LEFT_MARGIN, bgRect.y() + FONT_PREVIEW_TOP_MARGIN,
                            fontRectWidth, bgRect.height() - FONT_PREVIEW_TOP_MARGIN - FONT_PREVIEW_BOTTOM_MARGIN);

#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
        // qDebug() << "Using compact mode for preview rect adjustment";
        fontPreviewRect = QRect(bgRect.x() + FONT_PREVIEW_LEFT_MARGIN, bgRect.y() + FONT_PREVIEW_TOP_COMPACT_MARGIN,
                                fontRectWidth, bgRect.height() - FONT_PREVIEW_TOP_COMPACT_MARGIN - FONT_PREVIEW_BOTTOM_COMPACT_MARGIN);
    }
#endif
    // qDebug() << "Exiting function: DFontPreviewItemDelegate::adjustPreviewRect";
    return fontPreviewRect;
}

/*************************************************************************
 <Function>      adjustPreviewFontBaseLinePoint
 <Description>   根据预览区域和文字显示效果获取基准点
 <Author>        null
 <Input>
    <param1>     fontPreviewRect               Description:字体预览区域范围
    <param2>     previewFontMetrics            Description:字体显示效果
 <Return>        QPoint                        Description:显示基准点
 <Note>          null
*************************************************************************/
QPoint DFontPreviewItemDelegate::adjustPreviewFontBaseLinePoint(const QRect &fontPreviewRect, const QFontMetrics &previewFontMetrics) const
{
    // qDebug() << "Entering function: DFontPreviewItemDelegate::adjustPreviewFontBaseLinePoint";
    Q_UNUSED(previewFontMetrics);
    /* 部分不规则的字体无法获取到有效QFontMetrics::height(),即 QFontMetrics::ascent(), QFontMetrics::descent()无效. */
//    int baseLineY = 0;
//    if (previewFontMetrics.ascent() == previewFontMetrics.descent()) {
//        baseLineY = fontPreviewRect.bottom() - fontPreviewRect.height() / 2;
//    } else {
//        baseLineY = fontPreviewRect.bottom() - (fontPreviewRect.height() - previewFontMetrics.height()) / 2;
//    }
    /* Bug#20555 调整descent值，用来调整预览效果 UT000591 */
    int commonFontDescent = fontPreviewRect.height() / 5;
    int baseLineX = fontPreviewRect.x();
    int baseLineY = fontPreviewRect.bottom() - commonFontDescent;

    // qDebug() << "Exiting function: DFontPreviewItemDelegate::adjustPreviewFontBaseLinePoint";
    return QPoint(baseLineX, baseLineY);
}

/*************************************************************************
 <Function>      paintForegroundPreviewFont
 <Description>   绘制字体预览列表中的预览内容
 <Author>        null
 <Input>
    <param1>     painter                 Description:绘制类指针
    <param2>     option                  Description:listview中的一行成员
    <param3>     itemdata                Description:对应aption行的数据
    <param4>     fontPixelSize           Description:预览效果的像素大小
    <param5>     fontPreviewText         Description:要显示的预览内容

 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewItemDelegate::paintForegroundPreviewFont(QPainter *painter, const QStyleOptionViewItem &option,
                                                          const QString &familyName, const QString &styleName, bool isEnabled,
                                                          int fontPixelSize, QString &fontPreviewText) const
{
    // qDebug() << "Entering function: DFontPreviewItemDelegate::paintForegroundPreviewFont";
    QFont previewFont;

    previewFont.setFamily(familyName);
    previewFont.setStyleName(styleName);
    previewFont.setPixelSize(fontPixelSize);
    painter->setFont(previewFont);

    if (painter->fontInfo().family().isEmpty()) {
        qWarning() << "Empty font family, skipping preview";
        return;
    }

    QRect fontPreviewRect = adjustPreviewRect(option.rect);
//    painter->setPen(QPen(option.palette.color(DPalette::Text)));
    //SP3--禁用置灰(539)
    QColor color(option.palette.color(DPalette::Text));
    if (!isEnabled) {
        // qDebug() << "Font preview is disabled, setting alpha to 0.6";
        color.setAlphaF(0.6);
    }
    painter->setPen(QPen(color));

    QFontMetrics fontMetric(previewFont);

    QString elidedText = fontMetric.elidedText(fontPreviewText, Qt::ElideRight, fontPreviewRect.width());

    QPoint baseLinePoint = adjustPreviewFontBaseLinePoint(fontPreviewRect, fontMetric);
    /* 使用baseline规则绘制预览文字，这样不用考虑特殊字体 UT000591 */
    painter->drawText(baseLinePoint.x(), baseLinePoint.y(), elidedText);
    // qDebug() << "Exiting function: DFontPreviewItemDelegate::paintForegroundPreviewFont";
}

/*************************************************************************
 <Function>      paintBackground
 <Description>   绘制字体预览列表的背景
 <Author>        null
 <Input>
    <param1>     painter            Description:绘制类指针
    <param2>     option             Description:listview中的一行成员
    <param3>     index              Description:字体列表的索引
 <Return>        null               Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewItemDelegate::paintBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // qDebug() << "Entering function: DFontPreviewItemDelegate::paintBackground";
    DPalette::ColorGroup colorGroup = option.state.testFlag(QStyle::State_Enabled) ? DPalette::Normal : DPalette::Disabled;
    if (colorGroup == DPalette::Normal && false == option.state.testFlag(QStyle::State_Active)) {
        // qDebug() << "Setting color group to Inactive";
        colorGroup = DPalette::Inactive;
    }

    QRect bgRect = QRect(option.rect.x() + 10, option.rect.y(), option.rect.width() - 20, option.rect.height() - 2);

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);

    if (0 == index.row()) {
        int radius = 8;
        path.addRoundedRect(bgRect, radius, radius);
        path.addRect(QRect(bgRect.x(), bgRect.y() + bgRect.height() / 2, bgRect.width(), bgRect.height() / 2));
    } else {
        path.addRect(bgRect);
    }

    if (option.state.testFlag(QStyle::State_Selected)) {
        // qDebug() << "Item is selected";
        //如果是通过tab获取到的焦点，绘制tab的选中效果
        if (m_parentView->getIsTabFocus() == true) {
            // qDebug() << "Using tab focus background";
            paintTabFocusBackground(painter, option, bgRect);
        } else {
            // qDebug() << "Using normal selected background";
            //不是通过tab获取到的焦点的情况，再去绘制默认的选中状态，避免的tab选中状态时四个角会出现直角的问题
            DStyleHelper styleHelper;
            QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), DPalette::ToolTipText);

            fillColor.setAlphaF(0.2);
            painter->setBrush(QBrush(fillColor));
            painter->fillPath(path, fillColor);
        }
    } else {
        // qDebug() << "Item is not selected, using default background";
#if QT_VERSION_MAJOR > 5
        DPalette pa = DGuiApplicationHelper::instance()->applicationPalette();
#else
        DPalette pa = DApplicationHelper::instance()->palette(m_parentView);
#endif
        DStyleHelper styleHelper;
        QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::ItemBackground);
        painter->setBrush(QBrush(fillColor));
        painter->fillPath(path, fillColor);
    }
    // qDebug() << "Exiting function: DFontPreviewItemDelegate::paintBackground";
}

/*************************************************************************
 <Function>      paintTabFocusBackground
 <Description>   绘制tab选中后的背景效果
 <Author>        null
 <Input>
    <param1>     painter            Description:绘制类指针
    <param2>     option             Description:listview中的一行成员
    <param3>     bgRect           Description:绘制区域
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewItemDelegate::paintTabFocusBackground(QPainter *painter, const QStyleOptionViewItem &option, const QRect &bgRect) const
{
    // qDebug() << "Entering function: DFontPreviewItemDelegate::paintTabFocusBackground";
    //初始化绘制高亮色区域路径
    QPainterPath path;
    painter->setRenderHint(QPainter::Antialiasing, true);
    const int radius = 8;
    setPaintPath(bgRect, path, 0, 0, radius);

    //初始化绘制窗口色区域路径
    QPainterPath path2;
    setPaintPath(bgRect, path2, 2, 2, 6);

    //初始化绘制背景色区域路径
    QPainterPath path3;
    setPaintPath(bgRect, path3, 3, 3, 6);

    DPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
                              ? DPalette::Normal : DPalette::Disabled;
    if (cg == DPalette::Normal && !(option.state & QStyle::State_Active)) {
        cg = DPalette::Inactive;
    }

    QColor fillColor = option.palette.color(cg, DPalette::Highlight);
    painter->setBrush(QBrush(fillColor));
    painter->fillPath(path, painter->brush());

    QColor fillColor2 = option.palette.color(cg, DPalette::Window);
    painter->setBrush(QBrush(fillColor2));
    painter->fillPath(path2, painter->brush());

    DStyleHelper styleHelper;
    QColor fillColor3 = styleHelper.getColor(static_cast<const QStyleOption *>(&option), DPalette::ToolTipText);
    fillColor3.setAlphaF(0.2);
    painter->setBrush(QBrush(fillColor3));
    painter->fillPath(path3, painter->brush());
    // qDebug() << "Exiting function: DFontPreviewItemDelegate::paintTabFocusBackground";
}

/*************************************************************************
 <Function>      setPaintPath
 <Description>   得到需要绘制区域的路径
 <Author>        null
 <Input>
    <param1>     bgRect          Description:listview一项的区域范围
    <param2>     path            Description:需要绘制区域的路径
    <param3>     xDifference     Description:x方向需要变化的数值
    <param4>     yDifference     Description:y方向需要变化的数值
    <param5>     radius          Description:圆弧半径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewItemDelegate::setPaintPath(const QRect &bgRect, QPainterPath &path, const int xDifference, const int yDifference, const int radius) const
{
    // qDebug() << "Entering function: DFontPreviewItemDelegate::setPaintPath";
    QPoint path_bottomRight(bgRect.bottomRight().x() - xDifference, bgRect.bottomRight().y() - yDifference);
    QPoint path_topRight(bgRect.topRight().x() - xDifference, bgRect.topRight().y() + yDifference);
    QPoint path_topLeft(bgRect.topLeft().x() + xDifference, bgRect.topLeft().y() + yDifference);
    QPoint path_bottomLeft(bgRect.bottomLeft().x() + xDifference, bgRect.bottomLeft().y() - yDifference);
    path.moveTo(path_bottomRight - QPoint(0, 10));
    path.lineTo(path_topRight + QPoint(0, 10));
    path.arcTo(QRect(QPoint(path_topRight - QPoint(radius * 2, 0)), QSize(radius * 2, radius * 2)), 0, 90);
    path.lineTo(path_topLeft + QPoint(10, 0));
    path.arcTo(QRect(QPoint(path_topLeft), QSize(radius * 2, radius * 2)), 90, 90);
    path.lineTo(path_bottomLeft - QPoint(0, 10));
    path.arcTo(QRect(QPoint(path_bottomLeft - QPoint(0, radius * 2)), QSize(radius * 2, radius * 2)), 180, 90);
    path.lineTo(path_bottomRight - QPoint(10, 0));
    path.arcTo(QRect(QPoint(path_bottomRight - QPoint(radius * 2, radius * 2)), QSize(radius * 2, radius * 2)), 270, 90);
    // qDebug() << "Exiting function: DFontPreviewItemDelegate::setPaintPath";
}


/*************************************************************************
 <Function>    paint
 <Description>   代理的绘画函数
 <Author>        null
 <Input>
    <param1>     painter         Description:绘制类指针
    <param2>     option          Description:listview中的一行成员
    <param3>     index           Description:option的索引
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    // qDebug() << "Entering function: DFontPreviewItemDelegate::paint";
    if (index.isValid()) {
        // qDebug() << "Index is valid, proceeding with painting";
        painter->save();
        painter->setRenderHint(QPainter::TextAntialiasing, true);

        FontData fontData = index.data(Qt::DisplayRole).value<FontData>();
        int fontPixelSize = (index.data(FontSizeRole).isNull()) ? FTM_DEFAULT_PREVIEW_FONTSIZE : index.data(FontSizeRole).toInt();
        fontPixelSize = (fontPixelSize <= 0) ? FTM_DEFAULT_PREVIEW_FONTSIZE : fontPixelSize;
        FontDelegateData dData = index.data(FontFamilyStylePreviewRole).value<FontDelegateData>();
        QString fontPreviewContent = index.data(FontPreviewRole).toString().isEmpty()
                                     ? dData.ownPreview : index.data(FontPreviewRole).toString();
        if ((fontPreviewContent.isEmpty() || 0 == fontPixelSize)) {
            qWarning() << "Empty preview content or invalid font size, skipping paint";
            painter->restore();
            return;
        }

        paintBackground(painter, option, index);
        paintForegroundCheckBox(painter, option, fontData);
        paintForegroundFontName(painter, option, fontData);
        paintForegroundCollectIcon(painter, option, fontData);
        paintForegroundPreviewFont(painter, option, dData.familyName,
                                   dData.styleName, fontData.isEnabled(),
                                   fontPixelSize, fontPreviewContent);
        painter->restore();
    } else {
        // qDebug() << "Index is invalid, delegating to parent paint";
        QStyledItemDelegate::paint(painter, option, index);
    }
    // qDebug() << "Exiting function: DFontPreviewItemDelegate::paint";
}

/*************************************************************************
 <Function>      sizeHint
 <Description>   获取当前listview一项尺寸的大小
 <Author>        null
 <Input>
    <param1>     option            Description:listview中的一行成员
    <param2>     index            Description:lsiview中的该字体的索引
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
QSize DFontPreviewItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    // qDebug() << "Entering function: DFontPreviewItemDelegate::sizeHint";
    FontData data = index.data(Qt::DisplayRole).value<FontData>();
    int fontSize = (false == index.data(FontSizeRole).isNull()) ? index.data(FontSizeRole).toInt() : FTM_DEFAULT_PREVIEW_FONTSIZE;

    int itemHeight = FTM_PREVIEW_ITEM_HEIGHT;
#ifdef DTKWIDGET_CLASS_DSizeMode
    if (DGuiApplicationHelper::instance()->sizeMode() == DGuiApplicationHelper::SizeMode::CompactMode) {
        // qDebug() << "Using compact mode for item height";
        itemHeight = FTM_PREVIEW_ITEM_COMPACT_HEIGHT;
    }
#endif

    if (fontSize > 30) {
        // qDebug() << "Font size is larger than 30, adjusting item height";
        itemHeight += static_cast<int>(((fontSize - 30) + 1) * 1.5);
    }

    // qDebug() << "Exiting function: DFontPreviewItemDelegate::sizeHint";
    return QSize(option.rect.width(), itemHeight);
}

/*************************************************************************
 <Function>      eventFilter
 <Description>   事件过滤器
 <Author>        null
 <Input>
    <param1>     object            Description:unused
    <param2>     event             Description:进入时间过滤器的事件
 <Return>        bool              Description:事件是否处理
 <Note>          null
*************************************************************************/
bool DFontPreviewItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    // qDebug() << "Entering function: DFontPreviewItemDelegate::eventFilter";
    Q_UNUSED(object)
    if (event->type() == QEvent::HoverLeave) {
        // qDebug() << "Processing HoverLeave event";
        DFontPreviewListView *listview = qobject_cast<DFontPreviewListView *>(parent());
        listview->clearHoverState();
        listview->clearPressState(DFontPreviewListView::ClearType::PreviousClear);
    }

    return false;
}

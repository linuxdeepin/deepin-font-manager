#include "dfontpreviewitemdelegate.h"
#include "dfontpreviewlistview.h"
#include "globaldef.h"
#include "utils.h"

#include <QPainter>

#include <DApplication>
#include <DApplicationHelper>
#include <DStyleHelper>
#include <DPalette>
#include <DLog>
#include <DCheckBox>
#include <DLabel>
#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE

const int CHECKBOX_SIZE = 20;

const int COLLECT_ICON_SIZE = 24;
const int COLLECT_ICON_RIGHT_MARGIN = 19;
const int COLLECT_ICON_TOP_MARGIN = 10;

const int FONT_NAME_HEIGHT = 30;
const int FONT_NAME_LEFT_MARGIN = 50;
const int FONT_NAME_TOP_MARGIN = 3;
const int FONT_PREVIEW_LEFT_MARGIN = 50;
const int FONT_PREVIEW_RIGHT_MARGIN = COLLECT_ICON_SIZE + COLLECT_ICON_RIGHT_MARGIN;
const int FONT_PREVIEW_TOP_MARGIN = FONT_NAME_HEIGHT + FONT_NAME_TOP_MARGIN;
const int FONT_PREVIEW_BOTTOM_MARGIN = 10;

DFontPreviewItemDelegate::DFontPreviewItemDelegate(QAbstractItemView *parent)
    : QStyledItemDelegate(parent)
    , m_parentView(qobject_cast<DFontPreviewListView *>(parent))
{
    parent->viewport()->installEventFilter(this);
}

void DFontPreviewItemDelegate::paintForegroundCheckBox(QPainter *painter, const QStyleOptionViewItem &option, const FontData &itemData) const
{
    int checkBoxWidth = CHECKBOX_SIZE - 4;
    int checkBoxHeight = CHECKBOX_SIZE - 4;

    QRect rect = QRect(option.rect.x() + 25, option.rect.y() + 10, checkBoxWidth, checkBoxHeight);

    QStyleOptionButton checkBoxOption;
    checkBoxOption.state |= QStyle::State_Enabled;
    checkBoxOption.state |= itemData.isEnabled() ? QStyle::State_On : QStyle::State_Off;
    checkBoxOption.rect = rect;

    DCheckBox checkBox;
    DApplication::style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &checkBoxOption, painter, &checkBox);
}

void DFontPreviewItemDelegate::paintForegroundFontName(QPainter *painter, const QStyleOptionViewItem &option, const FontData &itemData) const
{
    QFont nameFont = painter->font();
    nameFont.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T6));
    painter->setFont(nameFont);

    DStyleHelper styleHelper;
    DPalette pa = DApplicationHelper::instance()->palette(m_parentView);
    QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::TextTips);
    //SP3--禁用置灰(539)
    if (!itemData.isEnabled()) {
        fillColor.setAlphaF(0.6);
    }
    painter->setPen(QPen(fillColor));

    QRect fontNameRect = QRect(option.rect.x() + FONT_NAME_LEFT_MARGIN, option.rect.y() + FONT_NAME_TOP_MARGIN,
                               option.rect.width() - 20, FONT_NAME_HEIGHT);

    QFontMetrics mt(nameFont);//特殊图案字体下截断字体名称/*UT000539*/
    QString elidedText = mt.elidedText(itemData.strFontName, Qt::ElideRight, option.rect.width() - 120, Qt::TextShowMnemonic);
    painter->drawText(fontNameRect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);
}

void DFontPreviewItemDelegate::paintForegroundCollectIcon(QPainter *painter, const QStyleOptionViewItem &option, const FontData &itemData) const
{
    DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
    QString strImgPrefix = (DGuiApplicationHelper::DarkType == appHelper->themeType()) ? QString("dark_") : QString("");

    QString iconStatus = QString("press");
    IconStatus status = itemData.getHoverState();
    if (IconHover == status) {
        iconStatus = QString("hover");
    } else if (IconPress == status) {
        iconStatus = QString("press");
        strImgPrefix = "";
    } else {
        iconStatus = QString("normal");
    }

    //fix bug-14120 -> clear hover state, if mouse leaved
//    iconStatus = (false == option.state.testFlag(QStyle::State_MouseOver)) ? QString("normal") : iconStatus;

    QString strImageSrc;
    QPixmap pixmap;
    if (itemData.isCollected()) {
        strImageSrc = QString("://%1collection_%2.svg").arg(strImgPrefix).arg(iconStatus);
        pixmap = Utils::renderSVG(strImageSrc, QSize(COLLECT_ICON_SIZE, COLLECT_ICON_SIZE));
    } else {
        strImageSrc = QString("://%1uncollection_%2.svg").arg(strImgPrefix).arg(iconStatus);
        pixmap = Utils::renderSVG(strImageSrc, QSize(COLLECT_ICON_SIZE, COLLECT_ICON_SIZE));
    }

    QRect collectIconRealRect = QRect(option.rect.right() - COLLECT_ICON_SIZE - COLLECT_ICON_RIGHT_MARGIN,
                                      option.rect.top() + COLLECT_ICON_TOP_MARGIN,
                                      COLLECT_ICON_SIZE, COLLECT_ICON_SIZE);
    painter->drawPixmap(collectIconRealRect, pixmap);

}

QRect DFontPreviewItemDelegate::adjustPreviewRect(const QRect bgRect) const
{
    QRect fontPreviewRect;
    int fontRectWidth = bgRect.width() - FONT_PREVIEW_LEFT_MARGIN - FONT_PREVIEW_RIGHT_MARGIN;
    fontPreviewRect = QRect(bgRect.x() + FONT_PREVIEW_LEFT_MARGIN, bgRect.y() + FONT_PREVIEW_TOP_MARGIN,
                            fontRectWidth, bgRect.height() - FONT_PREVIEW_TOP_MARGIN - FONT_PREVIEW_BOTTOM_MARGIN);
    return fontPreviewRect;
}

QPoint DFontPreviewItemDelegate::adjustPreviewFontBaseLinePoint(const QRect &fontPreviewRect, const QFontMetrics &previewFontMetrics) const
{
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

    return QPoint(baseLineX, baseLineY);
}

void DFontPreviewItemDelegate::paintForegroundPreviewFont(QPainter *painter, const QStyleOptionViewItem &option, const DFontPreviewItemData &itemData, int fontPixelSize, QString &fontPreviewText) const
{
    QFont previewFont;
    previewFont.setFamily(itemData.fontInfo.familyName);
    previewFont.setStyleName(itemData.fontInfo.styleName);
    previewFont.setPixelSize(fontPixelSize);
    painter->setFont(previewFont);

    if (painter->fontInfo().family().isEmpty())
        return;

    QRect fontPreviewRect = adjustPreviewRect(option.rect);
//    painter->setPen(QPen(option.palette.color(DPalette::Text)));
    //SP3--禁用置灰(539)
    QColor color(option.palette.color(DPalette::Text));
    if (!itemData.fontData.isEnabled())
        color.setAlphaF(0.6);
    painter->setPen(QPen(color));

    QFontMetrics fontMetric(previewFont);

    QString elidedText = fontMetric.elidedText(fontPreviewText, Qt::ElideRight, fontPreviewRect.width(), Qt::TextShowMnemonic);

    QPoint baseLinePoint = adjustPreviewFontBaseLinePoint(fontPreviewRect, fontMetric);
    /* 使用baseline规则绘制预览文字，这样不用考虑特殊字体 UT000591 */
    painter->drawText(baseLinePoint.x(), baseLinePoint.y(), elidedText);
}

void DFontPreviewItemDelegate::paintBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    DPalette::ColorGroup colorGroup = option.state.testFlag(QStyle::State_Enabled) ? DPalette::Normal : DPalette::Disabled;
    if (colorGroup == DPalette::Normal && false == option.state.testFlag(QStyle::State_Active)) {
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
        //如果是通过tab获取到的焦点，绘制tab的选中效果
        if (m_parentView->getIsTabFocus() == true) {
            paintTabFocusBackground(painter, option, bgRect);
        } else {
            //不是通过tab获取到的焦点的情况，再去绘制默认的选中状态，避免的tab选中状态时四个角会出现直角的问题
            DStyleHelper styleHelper;
            QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), DPalette::ToolTipText);

            fillColor.setAlphaF(0.2);
            painter->setBrush(QBrush(fillColor));
            painter->fillPath(path, fillColor);
        }
    } else {
        DPalette pa = DApplicationHelper::instance()->palette(m_parentView);
        DStyleHelper styleHelper;
        QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::ItemBackground);
        painter->setBrush(QBrush(fillColor));
        painter->fillPath(path, fillColor);
    }
}

//绘制tab选中后的背景效果
void DFontPreviewItemDelegate::paintTabFocusBackground(QPainter *painter, const QStyleOptionViewItem &option, const QRect &bgRect) const
{
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

    DPalette::ColorGroup cg = option.state & QStyle::State_Enabled
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
}

/**
*  @brief  得到需要绘制区域的路径
*  @param[in]  bgRect listview一项的区域范围
*  @param[in]  path 需要绘制区域的路径
*  @param[in]  xDifference x方向需要变化的数值
*  @param[in]  yDifference y方向需要变化的数值
*  @param[in]  radius  圆弧半径
*/
void DFontPreviewItemDelegate::setPaintPath(const QRect &bgRect, QPainterPath &path, const int xDifference, const int yDifference, const int radius) const
{
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
}

void DFontPreviewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->save();
        painter->setRenderHint(QPainter::TextAntialiasing, true);

        FontData fontData = index.data(Qt::DisplayRole).value<FontData>();
        DFontPreviewItemData itemData = m_parentView->getFontData(fontData);
        if (itemData.fontInfo.filePath.isEmpty())
            return;
        int fontPixelSize = (index.data(FontSizeRole).isNull()) ? FTM_DEFAULT_PREVIEW_FONTSIZE : index.data(FontSizeRole).toInt();
        fontPixelSize = (fontPixelSize <= 0) ? FTM_DEFAULT_PREVIEW_FONTSIZE : fontPixelSize;
        QString fontPreview = itemData.fontInfo.defaultPreview;
        if (fontPreview.isEmpty()) {
            if (itemData.fontInfo.previewLang == FONT_LANG_CHINESE) {
                fontPreview = FTM_DEFAULT_PREVIEW_CN_TEXT;
            } else if (itemData.fontInfo.previewLang & FONT_LANG_ENGLISH) {
                fontPreview = FTM_DEFAULT_PREVIEW_EN_TEXT;
            } else {
                fontPreview = FTM_DEFAULT_PREVIEW_DIGIT_TEXT;
            }
        }

        QString fontPreviewContent = index.data(FontPreviewRole).toString().isEmpty() ? fontPreview : index.data(FontPreviewRole).toString();
        if ((fontPreviewContent.isEmpty() || 0 == fontPixelSize) && itemData.fontData.strFontName.isEmpty()) {
            painter->restore();
            return;
        }

        paintBackground(painter, option, index);
        paintForegroundCheckBox(painter, option, fontData);
        paintForegroundFontName(painter, option, fontData);
        paintForegroundCollectIcon(painter, option, fontData);
        paintForegroundPreviewFont(painter, option, itemData, fontPixelSize, fontPreviewContent);
        painter->restore();
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }

    /* Bug#21463 UT000591 */
    QFont::cleanup();
}

QSize DFontPreviewItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    FontData data = index.data(Qt::DisplayRole).value<FontData>();
    int fontSize = (false == index.data(FontSizeRole).isNull()) ? index.data(FontSizeRole).toInt() : FTM_DEFAULT_PREVIEW_FONTSIZE;

    int itemHeight = FTM_PREVIEW_ITEM_HEIGHT;
    if (fontSize > 30) {
        itemHeight += static_cast<int>(((fontSize - 30) + 1) * 1.5);
    }

    return QSize(option.rect.width(), itemHeight);
}

bool DFontPreviewItemDelegate::eventFilter(QObject *object, QEvent *event)
{
    Q_UNUSED(object)
    if (event->type() == QEvent::HoverLeave) {
        DFontPreviewListView *listview = qobject_cast<DFontPreviewListView *>(parent());
        listview->clearHoverState();
    }

    return false;
}

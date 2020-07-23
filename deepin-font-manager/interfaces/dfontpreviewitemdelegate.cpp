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

void DFontPreviewItemDelegate::paintForegroundCheckBox(QPainter *painter, const QStyleOptionViewItem &option, const DFontPreviewItemData &itemData) const
{
    int checkBoxWidth = CHECKBOX_SIZE - 4;
    int checkBoxHeight = CHECKBOX_SIZE - 4;

    QRect rect = QRect(option.rect.x() + 25, option.rect.y() + 10, checkBoxWidth, checkBoxHeight);

    QStyleOptionButton checkBoxOption;
    checkBoxOption.state |= QStyle::State_Enabled;
    checkBoxOption.state |= (itemData.isEnabled == true) ? QStyle::State_On : QStyle::State_Off;
    checkBoxOption.rect = rect;

    DCheckBox checkBox;
    DApplication::style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &checkBoxOption, painter, &checkBox);
}

void DFontPreviewItemDelegate::paintForegroundFontName(QPainter *painter, const QStyleOptionViewItem &option, const DFontPreviewItemData &itemData) const
{
    QFont nameFont = painter->font();
    nameFont.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T6));
    painter->setFont(nameFont);

    DStyleHelper styleHelper;
    DPalette pa = DApplicationHelper::instance()->palette(m_parentView);
    QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::TextTips);
    //SP3--禁用置灰(539)
    if (!itemData.isEnabled) {
        fillColor.setAlphaF(0.6);
    }
    painter->setPen(QPen(fillColor));

    QRect fontNameRect = QRect(option.rect.x() + FONT_NAME_LEFT_MARGIN, option.rect.y() + FONT_NAME_TOP_MARGIN,
                               option.rect.width() - 20, FONT_NAME_HEIGHT);

    QFontMetrics mt(nameFont);//特殊图案字体下截断字体名称/*UT000539*/
    QString elidedText = mt.elidedText(itemData.strFontName, Qt::ElideRight, option.rect.width() - 120, Qt::TextShowMnemonic);
    painter->drawText(fontNameRect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);
}

void DFontPreviewItemDelegate::paintForegroundCollectIcon(QPainter *painter, const QStyleOptionViewItem &option, const DFontPreviewItemData &itemData) const
{
    DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
    QString strImgPrefix = (DGuiApplicationHelper::DarkType == appHelper->themeType()) ? QString("dark_") : QString("");

    QString iconStatus = QString("press");
    if (IconHover == itemData.collectIconStatus) {
        iconStatus = QString("hover");
    } else if (IconPress == itemData.collectIconStatus) {
        iconStatus = QString("press");
        strImgPrefix = "";
    } else {
        iconStatus = QString("normal");
    }

    //fix bug-14120 -> clear hover state, if mouse leaved
//    iconStatus = (false == option.state.testFlag(QStyle::State_MouseOver)) ? QString("normal") : iconStatus;

    QString strImageSrc;
    QPixmap pixmap;
    if (itemData.isCollected) {
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

QFont DFontPreviewItemDelegate::adjustPreviewFont(const int appFontId, const QString &fontFamilyName, const QString &fontStyleName, const int &fontSize) const
{

    QString familyName = QFontDatabase::applicationFontFamilies(appFontId).isEmpty() ? fontFamilyName : QFontDatabase::applicationFontFamilies(appFontId).first();
    QFont font;
    font.setFamily(familyName);
    font.setStyleName(fontStyleName);
    font.setPixelSize(fontSize);
//    font.setItalic(fontStyleName.contains("Italic"));
    //设置字体 ut000794
//    setfont(font, fontStyleName);
    return font;
}

void DFontPreviewItemDelegate::setfont(QFont &font, QString fontStyleName) const
{
    if (fontStyleName.contains("Regular")) {
        font.setWeight(QFont::Normal);
    } else if (fontStyleName.contains("Bold")) {
        font.setWeight(QFont::Bold);
    } else if (fontStyleName.contains("Light")) {
        font.setWeight(QFont::Light);
    } else if (fontStyleName.contains("Thin")) {
        font.setWeight(QFont::Thin);
    } else if (fontStyleName.contains("ExtraLight")) {
        font.setWeight(QFont::ExtraLight);
    } else if (fontStyleName.contains("ExtraBold")) {
        font.setWeight(QFont::ExtraBold);
    } else if (fontStyleName.contains("Medium")) {
        font.setWeight(QFont::Medium);
    } else if (fontStyleName.contains("DemiBold")) {
        font.setWeight(QFont::DemiBold);
    } else if (fontStyleName.contains("Black")) {
        font.setWeight(QFont::Black);
    } else if (fontStyleName.contains("AnyStretch")) {
        font.setStretch(QFont::AnyStretch);
    } else if (fontStyleName.contains("UltraCondensed")) {
        font.setStretch(QFont::UltraCondensed);
    } else if (fontStyleName.contains("ExtraCondensed")) {
        font.setStretch(QFont::ExtraCondensed);
    } else if (fontStyleName.contains("Condensed")) {
        font.setStretch(QFont::Condensed);
    } else if (fontStyleName.contains("SemiCondensed")) {
        font.setStretch(QFont::SemiCondensed);
    } else if (fontStyleName.contains("Unstretched")) {
        font.setStretch(QFont::Unstretched);
    } else if (fontStyleName.contains("SemiExpanded")) {
        font.setStretch(QFont::SemiExpanded);
    } else if (fontStyleName.contains("Expanded")) {
        font.setStretch(QFont::Expanded);
    } else if (fontStyleName.contains("ExtraExpanded")) {
        font.setStretch(QFont::ExtraExpanded);
    } else if (fontStyleName.contains("UltraExpanded")) {
        font.setStretch(QFont::UltraExpanded);
    }

}

void DFontPreviewItemDelegate::paintForegroundPreviewFont(QPainter *painter, const QStyleOptionViewItem &option, const DFontPreviewItemData &itemData, int fontPixelSize, QString &fontPreviewText) const
{
    QFont previewFont = adjustPreviewFont(itemData.appFontId, itemData.fontInfo.familyName, itemData.fontInfo.styleName, fontPixelSize);
    previewFont.setPixelSize(fontPixelSize);
    painter->setFont(previewFont);

    if (painter->fontInfo().family().isEmpty())
        return;

    QRect fontPreviewRect = adjustPreviewRect(option.rect);
//    painter->setPen(QPen(option.palette.color(DPalette::Text)));
    //SP3--禁用置灰(539)
    QColor color(option.palette.color(DPalette::Text));
    if (!itemData.isEnabled)
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
    QPainterPath path;
    painter->setRenderHint(QPainter::Antialiasing, true);
    const int radius = 8;

    path.moveTo(bgRect.bottomRight() - QPoint(0, radius));
    path.lineTo(bgRect.topRight() + QPoint(0, radius));
    path.arcTo(QRect(QPoint(bgRect.topRight() - QPoint(radius * 2, 0)), QSize(radius * 2, radius * 2)), 0, 90);
    path.lineTo(bgRect.topLeft() + QPoint(radius, 0));
    path.arcTo(QRect(QPoint(bgRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
    path.lineTo(bgRect.bottomLeft() - QPoint(0, radius));
    path.arcTo(QRect(QPoint(bgRect.bottomLeft() - QPoint(0, radius * 2)), QSize(radius * 2, radius * 2)), 180, 90);
    path.lineTo(bgRect.bottomLeft() + QPoint(radius, 0));
    path.arcTo(QRect(QPoint(bgRect.bottomRight() - QPoint(radius * 2, radius * 2)), QSize(radius * 2, radius * 2)), 270, 90);

    QPainterPath path2;
    QPoint path2_bottomRight(bgRect.bottomRight().x() - 2, bgRect.bottomRight().y() - 2);
    QPoint path2_topRight(bgRect.topRight().x() - 2, bgRect.topRight().y() + 2);
    QPoint path2_topLeft(bgRect.topLeft().x() + 2, bgRect.topLeft().y() + 2);
    QPoint path2_bottomLeft(bgRect.bottomLeft().x() + 2, bgRect.bottomLeft().y() - 2);
    path2.moveTo(path2_bottomRight - QPoint(0, 6));
    path2.lineTo(path2_topRight + QPoint(0, 6));
    path2.arcTo(QRect(QPoint(path2_topRight - QPoint(6 * 2, 0)), QSize(6 * 2, 6 * 2)), 0, 90);
    path2.lineTo(path2_topLeft + QPoint(6, 0));
    path2.arcTo(QRect(QPoint(path2_topLeft), QSize(6 * 2, 6 * 2)), 90, 90);
    path2.lineTo(path2_bottomLeft - QPoint(0, 6));
    path2.arcTo(QRect(QPoint(path2_bottomLeft - QPoint(0, 6 * 2)), QSize(6 * 2, 6 * 2)), 180, 90);
    path2.lineTo(path2_bottomRight - QPoint(6, 0));
    path2.arcTo(QRect(QPoint(path2_bottomRight - QPoint(6 * 2, 6 * 2)), QSize(6 * 2, 6 * 2)), 270, 90);

    QPainterPath path3;
    QPoint path3_bottomRight(bgRect.bottomRight().x() - 3, bgRect.bottomRight().y() - 3);
    QPoint path3_topRight(bgRect.topRight().x() - 3, bgRect.topRight().y() + 3);
    QPoint path3_topLeft(bgRect.topLeft().x() + 3, bgRect.topLeft().y() + 3);
    QPoint path3_bottomLeft(bgRect.bottomLeft().x() + 3, bgRect.bottomLeft().y() - 3);
    path3.moveTo(path3_bottomRight - QPoint(0, 10));
    path3.lineTo(path3_topRight + QPoint(0, 10));
    path3.arcTo(QRect(QPoint(path3_topRight - QPoint(6 * 2, 0)), QSize(6 * 2, 6 * 2)), 0, 90);
    path3.lineTo(path3_topLeft + QPoint(10, 0));
    path3.arcTo(QRect(QPoint(path3_topLeft), QSize(6 * 2, 6 * 2)), 90, 90);
    path3.lineTo(path3_bottomLeft - QPoint(0, 10));
    path3.arcTo(QRect(QPoint(path3_bottomLeft - QPoint(0, 6 * 2)), QSize(6 * 2, 6 * 2)), 180, 90);
    path3.lineTo(path3_bottomRight - QPoint(10, 0));
    path3.arcTo(QRect(QPoint(path3_bottomRight - QPoint(6 * 2, 6 * 2)), QSize(6 * 2, 6 * 2)), 270, 90);


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

void DFontPreviewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->save();
        painter->setRenderHint(QPainter::TextAntialiasing, true);

        DFontPreviewItemData itemData = index.data(Qt::DisplayRole).value<DFontPreviewItemData>();
        int fontPixelSize = (index.data(Dtk::UserRole + 2).isNull()) ? itemData.iFontSize : index.data(Dtk::UserRole + 2).toInt();
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
        QString fontPreviewContent = index.data(Dtk::UserRole + 1).toString().isEmpty() ? fontPreview : index.data(Dtk::UserRole + 1).toString();

        if ((fontPreviewContent.isEmpty() || 0 == fontPixelSize) && itemData.strFontName.isEmpty()) {
            painter->restore();
            return;
        }

        paintBackground(painter, option, index);
        paintForegroundCheckBox(painter, option, itemData);
        paintForegroundFontName(painter, option, itemData);
        paintForegroundCollectIcon(painter, option, itemData);
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
    DFontPreviewItemData data = index.data(Qt::DisplayRole).value<DFontPreviewItemData>();
    int fontSize = (false == index.data(Dtk::UserRole + 2).isNull()) ? index.data(Dtk::UserRole + 2).toInt() : data.iFontSize;

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

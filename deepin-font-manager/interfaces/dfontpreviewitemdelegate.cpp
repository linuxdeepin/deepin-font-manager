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

DWIDGET_USE_NAMESPACE

const int CHECKBOX_SIZE = 20;

const int COLLECT_ICON_SIZE = 24;
const int COLLECT_ICON_RIGHT_MARGIN = 15;
const int COLLECT_ICON_TOP_MARGIN = 10;

const int FONT_NAME_HEIGHT = 27;
const int FONT_NAME_LEFT_MARGIN = 50;
const int FONT_NAME_TOP_MARGIN = 3;

const int FONT_PREVIEW_LEFT_MARGIN = 50;
const int FONT_PREVIEW_RIGHT_MARGIN = COLLECT_ICON_SIZE + COLLECT_ICON_RIGHT_MARGIN;
const int FONT_PREVIEW_TOP_MARGIN = 27;
const int FONT_PREVIEW_BOTTOM_MARGIN = 10;


DFontPreviewItemDelegate::DFontPreviewItemDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
    , m_parentView(parent)
{
}

void DFontPreviewItemDelegate::paintForegroundCheckBox(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    int checkBoxWidth = CHECKBOX_SIZE - 4;
    int checkBoxHeight = CHECKBOX_SIZE - 4;

    DFontPreviewItemData itemData = index.data(Qt::DisplayRole).value<DFontPreviewItemData>();
    QRect rect = QRect(option.rect.x() + 25, option.rect.y() + 10, checkBoxWidth, checkBoxHeight);

    QStyleOptionButton checkBoxOption;
    checkBoxOption.state |= QStyle::State_Enabled;
    checkBoxOption.state |= (itemData.isEnabled == true) ? QStyle::State_On : QStyle::State_Off;
    checkBoxOption.rect = rect;

    DCheckBox checkBox;
    DApplication::style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &checkBoxOption, painter, &checkBox);
}

void DFontPreviewItemDelegate::paintForegroundFontName(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QFont nameFont = painter->font();
    nameFont.setPixelSize(DFontSizeManager::instance()->fontPixelSize(DFontSizeManager::T6));
    painter->setFont(nameFont);
    DStyleHelper styleHelper;
    DPalette pa = DApplicationHelper::instance()->palette(m_parentView);
    QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::TextTips);
    painter->setPen(QPen(fillColor));

    DFontPreviewItemData itemData = index.data(Qt::DisplayRole).value<DFontPreviewItemData>();
    QRect fontNameRect = QRect(option.rect.x() + FONT_NAME_LEFT_MARGIN, option.rect.y() + FONT_NAME_TOP_MARGIN,
                               option.rect.width() - 20, FONT_NAME_HEIGHT);
    painter->drawText(fontNameRect, Qt::AlignLeft | Qt::AlignVCenter, itemData.strFontName);
}

void DFontPreviewItemDelegate::paintForegroundCollectIcon(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
    QString strImgPrefix = (DGuiApplicationHelper::DarkType == appHelper->themeType()) ? QString("dark_") : QString("");

    DFontPreviewItemData itemData = index.data(Qt::DisplayRole).value<DFontPreviewItemData>();
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
        strImageSrc = QString(":/images/%1collection_%2.svg").arg(strImgPrefix).arg(iconStatus);
        pixmap = Utils::renderSVG(strImageSrc, QSize(COLLECT_ICON_SIZE, COLLECT_ICON_SIZE));
    } else {
        strImageSrc = QString(":/images/%1uncollection_%2.svg").arg(strImgPrefix).arg(iconStatus);
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
    /* 调整descent值，用来调整预览效果 UT000591 */
    int commonFontDescent = 4;
    int baseLineX = fontPreviewRect.x();
    int baseLineY = fontPreviewRect.bottom() - commonFontDescent;
    return QPoint(baseLineX, baseLineY);
}

QFont DFontPreviewItemDelegate::adjustPreviewFont(const QString &fontFamilyName, const QString &fontStyleName, const int &fontSize) const
{
    QFont font = QFont(fontFamilyName);
    font.setPixelSize(fontSize);
    font.setItalic(fontStyleName.contains("Italic"));

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
    }
    return font;
}

void DFontPreviewItemDelegate::paintForegroundPreviewContent(QPainter *painter, const QString &content, const QRect &fontPreviewRect, const QFont &previewFont) const
{
    QFontMetrics fontMetric(previewFont);
    QString elidedText = fontMetric.elidedText(content, Qt::ElideRight, fontPreviewRect.width(), Qt::TextShowMnemonic);
    QPoint baseLinePoint = adjustPreviewFontBaseLinePoint(fontPreviewRect, fontMetric);
    /* 使用baseline规则绘制预览文字，这样不用考虑特殊字体 UT000591 */
    //    painter->drawText(fontPreviewRect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);
    painter->drawText(baseLinePoint.x(), baseLinePoint.y(), elidedText);
}

void DFontPreviewItemDelegate::paintForegroundPreviewFont(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    DFontPreviewItemData itemData = index.data(Qt::DisplayRole).value<DFontPreviewItemData>();
    QRect fontPreviewRect = adjustPreviewRect(option.rect);
    QString fontPreviewContent = index.data(Dtk::UserRole + 1).isNull() ? itemData.strFontPreview : index.data(Dtk::UserRole + 1).toString();
    int fontPixelSize = (index.data(Dtk::UserRole + 2).isNull()) ? itemData.iFontSize : index.data(Dtk::UserRole + 2).toInt();

    painter->setPen(QPen(option.palette.color(DPalette::Text)));
    if (itemData.isPreviewEnabled) {
        QFont previewFont = adjustPreviewFont(itemData.fontInfo.familyName, itemData.fontInfo.styleName, fontPixelSize);
        painter->setFont(previewFont);
        paintForegroundPreviewContent(painter, fontPreviewContent, fontPreviewRect, previewFont);
    } else {
        QFont previewFont;
        previewFont.setPixelSize(fontPixelSize);
        painter->setFont(previewFont);
        paintForegroundPreviewContent(painter, fontPreviewContent, fontPreviewRect, previewFont);
    }
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
        DStyleHelper styleHelper;
        QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), DPalette::ToolTipText);
        fillColor.setAlphaF(0.2);
        painter->setBrush(QBrush(fillColor));
        painter->fillPath(path, fillColor);
    } else {
        DPalette pa = DApplicationHelper::instance()->palette(m_parentView);
        DStyleHelper styleHelper;
        QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::ItemBackground);
        painter->setBrush(QBrush(fillColor));
        painter->fillPath(path, fillColor);
    }
}

void DFontPreviewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        DFontPreviewItemData itemData = index.data(Qt::DisplayRole).value<DFontPreviewItemData>();
        int fontPixelSize = (index.data(Dtk::UserRole + 2).isNull()) ? itemData.iFontSize : index.data(Dtk::UserRole + 2).toInt();
        QString fontPreviewContent = index.data(Dtk::UserRole + 1).isNull() ? itemData.strFontPreview : index.data(Dtk::UserRole + 1).toString();
        if (fontPreviewContent.isEmpty() || 0 == fontPixelSize) {
            return;
        }

        paintBackground(painter, option, index);
        paintForegroundCheckBox(painter, option, index);
        paintForegroundFontName(painter, option, index);
        paintForegroundCollectIcon(painter, option, index);
        paintForegroundPreviewFont(painter, option, index);

        painter->restore();
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
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

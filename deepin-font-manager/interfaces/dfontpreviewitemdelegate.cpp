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
const int COLLECT_ICON_SIZE = 25;

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
    QRect fontNameRect = QRect(option.rect.x() + 50, option.rect.y() + 5, option.rect.width() - 20, 20);
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
    iconStatus = (false == option.state.testFlag(QStyle::State_MouseOver)) ? QString("normal") : iconStatus;

    QPixmap pixmap;
    if (itemData.isCollected) {
        QString strImageSrc = QString(":/images/%1collection_%2.svg").arg(strImgPrefix).arg(iconStatus);
        pixmap = Utils::renderSVG(strImageSrc, QSize(COLLECT_ICON_SIZE, COLLECT_ICON_SIZE));
    } else {
        QString strImageSrc = QString(":/images/%1uncollection_%2.svg").arg(strImgPrefix).arg(iconStatus);
        pixmap = Utils::renderSVG(strImageSrc, QSize(COLLECT_ICON_SIZE, COLLECT_ICON_SIZE));
    }

    QRect collectIconRealRect = QRect(option.rect.right() - 35 + 2, option.rect.top() + 10 - 3, COLLECT_ICON_SIZE, COLLECT_ICON_SIZE);
    painter->drawPixmap(collectIconRealRect, pixmap);
}

QRect DFontPreviewItemDelegate::adjustPreviewRect(const QString &fontName, const bool &isEnable, const QRect bgRect) const
{
    QRect fontPreviewRect;
    if (!fontName.compare("Noto Sans Tibetan-Bold") && isEnable == true) {
        fontPreviewRect = QRect(bgRect.x() + 50, bgRect.y() + 37, bgRect.width() - 50 - COLLECT_ICON_SIZE - 15, bgRect.height() - 27);
    } else if (!fontName.compare("Noto Serif Tibetan-Regular") && isEnable == true) {
        fontPreviewRect = QRect(bgRect.x() + 50, bgRect.y() + 37, bgRect.width() - 50 - COLLECT_ICON_SIZE - 15, bgRect.height() - 25);
    } else if (!fontName.compare("Noto Serif Tibetan-Bold") && isEnable == true) {
        fontPreviewRect = QRect(bgRect.x() + 50, bgRect.y() + 37, bgRect.width() - 50 - COLLECT_ICON_SIZE - 15, bgRect.height() - 21);
    } else if (!fontName.compare("Noto Sans Tibetan-Regular") && isEnable == true) {
        fontPreviewRect = QRect(bgRect.x() + 50, bgRect.y() + 37, bgRect.width() - 50 - COLLECT_ICON_SIZE - 15, bgRect.height() - 25);
    } else if (!fontName.compare("Noto Serif Myanmar-Regular") && isEnable == true) {
        fontPreviewRect = QRect(bgRect.x() + 50, bgRect.y() + 37, bgRect.width() - 50 - COLLECT_ICON_SIZE - 15, bgRect.height() - 25);
    } else if (!fontName.compare("Noto Serif Myanmar-Bold") && isEnable == true) {
        fontPreviewRect = QRect(bgRect.x() + 50, bgRect.y() + 40, bgRect.width() - 50 - COLLECT_ICON_SIZE - 15, bgRect.height() - 20);
    } else if (!fontName.compare("Noto Nastaliq Urdu-Bold") && isEnable == true) {
        fontPreviewRect = QRect(bgRect.x() + 50, bgRect.y() + 20, bgRect.width() - 50 - COLLECT_ICON_SIZE - 15, bgRect.height() - 20);
    } else if (!fontName.compare("Noto Nastaliq Urdu-Regular") && isEnable == true) {
        fontPreviewRect = QRect(bgRect.x() + 50, bgRect.y() + 20, bgRect.width() - 50 - COLLECT_ICON_SIZE - 15, bgRect.height() - 20);
    } else {
        fontPreviewRect = QRect(bgRect.x() + 50, bgRect.y() + 30, bgRect.width() - 50 - COLLECT_ICON_SIZE - 15, bgRect.height() - 26);
    }
    return fontPreviewRect;
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

void DFontPreviewItemDelegate::paintForegroundPreviewFont(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    DFontPreviewItemData itemData = index.data(Qt::DisplayRole).value<DFontPreviewItemData>();
    QRect fontPreviewRect = adjustPreviewRect(itemData.strFontName, itemData.isEnabled, option.rect);

    QString fontPreviewContent = index.data(Dtk::UserRole + 1).isNull() ? itemData.strFontPreview : index.data(Dtk::UserRole + 1).toString();
    int fontPixelSize = (index.data(Dtk::UserRole + 2).isNull()) ? itemData.iFontSize : index.data(Dtk::UserRole + 2).toInt();

    if (itemData.isPreviewEnabled) {
        QFont previewFont = adjustPreviewFont(itemData.fontInfo.familyName, itemData.fontInfo.styleName, fontPixelSize);
        QFontMetrics fontMetric(previewFont);
        QString previewText = Utils::convertToPreviewString(itemData.fontInfo.filePath, fontPreviewContent);
        QString elidedText = fontMetric.elidedText(previewText, Qt::ElideRight, option.rect.width() - 50 - COLLECT_ICON_SIZE - 15, Qt::TextShowMnemonic);
        //绘制预览字体
        painter->setPen(QPen(option.palette.color(DPalette::Text)));
        painter->setFont(previewFont);
        painter->drawText(fontPreviewRect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);
    } else {
        //禁用字体时使用系统默认字体显示
        QFont previewFont;
        previewFont.setPixelSize(fontPixelSize);
        QFontMetrics fontMetric(previewFont);
        QString elidedText = fontMetric.elidedText(fontPreviewContent, Qt::ElideRight, option.rect.width() - 50 - COLLECT_ICON_SIZE - 15, Qt::TextShowMnemonic);
        //绘制预览字体
        painter->setPen(QPen(option.palette.color(DPalette::Text)));
        painter->setFont(previewFont);
        painter->drawText(fontPreviewRect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);
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
//        path.moveTo(bgRect.bottomRight() - QPointF(0, radius));
//        path.lineTo(bgRect.topRight() + QPointF(0, radius));
//        path.arcTo(QRectF(QPointF(bgRect.topRight() - QPointF(radius * 2, 0)), QSize(radius * 2, radius * 2)), 0, 90);
//        path.lineTo(bgRect.topLeft() + QPointF(radius, 0));
//        path.arcTo(QRectF(QPointF(bgRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);

//        path.lineTo(bgRect.bottomLeft());
//        path.quadTo(bgRect.bottomLeft(), bgRect.bottomLeft());
//        path.lineTo(bgRect.bottomRight());
//        path.quadTo(bgRect.bottomRight(), bgRect.bottomRight());
    } else {
        path.addRect(bgRect);
//        path.moveTo(bgRect.topRight());
//        path.lineTo(bgRect.topLeft());
//        path.quadTo(bgRect.topLeft(), bgRect.topLeft());
//        path.lineTo(bgRect.bottomLeft());
//        path.quadTo(bgRect.bottomLeft(), bgRect.bottomLeft());
//        path.lineTo(bgRect.bottomRight());
//        path.quadTo(bgRect.bottomRight(), bgRect.bottomRight());
//        path.lineTo(bgRect.topRight());
//        path.quadTo(bgRect.topRight(), bgRect.topRight());
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
        if ( fontPreviewContent.isEmpty() || 0 == fontPixelSize) {
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

QSize DFontPreviewItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    QVariant varDisplay = index.data(Qt::DisplayRole);

    DFontPreviewItemData data = varDisplay.value<DFontPreviewItemData>();
    int iFontSize = data.iFontSize;

    QVariant varFontSize = index.data(Dtk::UserRole + 2);
    if (!varFontSize.isNull()) {
        iFontSize = varFontSize.toInt();
    }

    int itemHeight = FTM_PREVIEW_ITEM_HEIGHT;
    if (iFontSize > 30) {
        itemHeight += static_cast<int>(((iFontSize - 30) + 1) * 1.5);
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

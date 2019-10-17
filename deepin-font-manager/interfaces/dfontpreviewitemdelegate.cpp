#include "dfontpreviewitemdelegate.h"
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

DFontPreviewItemDelegate::DFontPreviewItemDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
    , m_parentView(parent)
{
}

void DFontPreviewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        QVariant varFontPreviewText = index.data(Dtk::UserRole + 1);
        QVariant varFontSize = index.data(Dtk::UserRole + 2);
        QVariant varDisplay = index.data(Qt::DisplayRole);

        DFontPreviewItemData data = varDisplay.value<DFontPreviewItemData>();

        QString strFontPreview = data.strFontPreview;
        int iFontSize = data.iFontSize;

        if (!varFontPreviewText.isNull()) {
            strFontPreview = varFontPreviewText.toString();
        }

        if (!varFontSize.isNull()) {
            iFontSize = varFontSize.toInt();
        }

        QStyleOptionViewItem viewOption(option);  //用来在视图中画一个item

        DPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? DPalette::Normal : DPalette::Disabled;
        if (cg == DPalette::Normal && !(option.state & QStyle::State_Active)) {
            cg = DPalette::Inactive;
        }

        QRect bgRect;
        bgRect.setX(option.rect.x()+10);
        bgRect.setY(option.rect.y());
        bgRect.setWidth(option.rect.width()-20);
        bgRect.setHeight(option.rect.height());

        QPainterPath path;

        if (0 == index.row())
        {
            int radius = 8;
            path.moveTo(bgRect.bottomRight() - QPointF(0, radius));
            path.lineTo(bgRect.topRight() + QPointF(0, radius));
            path.arcTo(QRectF(QPointF(bgRect.topRight() - QPointF(radius * 2, 0)), QSize(radius * 2, radius * 2)), 0, 90);
            path.lineTo(bgRect.topLeft() + QPointF(radius, 0));
            path.arcTo(QRectF(QPointF(bgRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);

            path.lineTo(bgRect.bottomLeft());
            path.quadTo(bgRect.bottomLeft(), bgRect.bottomLeft());
            path.lineTo(bgRect.bottomRight());
            path.quadTo(bgRect.bottomRight(), bgRect.bottomRight());
        }
        else {
            path.moveTo(bgRect.topRight());
            path.lineTo(bgRect.topLeft());
            path.quadTo(bgRect.topLeft(), bgRect.topLeft());
            path.lineTo(bgRect.bottomLeft());
            path.quadTo(bgRect.bottomLeft(), bgRect.bottomLeft());
            path.lineTo(bgRect.bottomRight());
            path.quadTo(bgRect.bottomRight(), bgRect.bottomRight());
            path.lineTo(bgRect.topRight());
            path.quadTo(bgRect.topRight(), bgRect.topRight());
        }

        if (option.state & QStyle::State_Selected) {
            DStyleHelper styleHelper;
            QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), DPalette::ToolTipText);
            fillColor.setAlphaF(0.2);
            painter->setBrush(QBrush(fillColor));
            painter->fillPath(path, fillColor);
        }
        else {
            DPalette pa = DApplicationHelper::instance()->palette(m_parentView);
            DStyleHelper styleHelper;
            QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::ItemBackground);
            painter->setBrush(QBrush(fillColor));
            painter->fillPath(path, fillColor);
        }

        int checkBoxSize = 20;
        int collectIconSize = 25;

        DCheckBox checkBox;
        //绘制checkbox
        QStyleOptionButton checkBoxOption;
        bool checked = data.isEnabled;
        checkBoxOption.state |= QStyle::State_Enabled;
        //根据值判断是否选中
        if (checked) {
            checkBoxOption.state |= QStyle::State_On;
        } else {
            checkBoxOption.state |= QStyle::State_Off;
        }

        QRect checkboxRealRect = QRect(bgRect.left() + 15, bgRect.top() + 10, checkBoxSize-4, checkBoxSize-4);
        checkBoxOption.rect = checkboxRealRect;
        DApplication::style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &checkBoxOption, painter,
                                             &checkBox);

        QString nameFontFamily = Utils::loadFontFamilyFromFiles(":/images/SourceHanSansCN-Medium.ttf");
        QFont nameFont(nameFontFamily);
        nameFont.setPixelSize(14);
        painter->setFont(nameFont);
        DStyleHelper styleHelper;
        DPalette pa = DApplicationHelper::instance()->palette(m_parentView);
        QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::TextTips);
        painter->setPen(QPen(fillColor));

        QRect fontNameRect = QRect(bgRect.left() + 50 - 2, checkboxRealRect.top()-5, bgRect.width() - 15 - 50, checkboxRealRect.height()+10);
        painter->drawText(fontNameRect, Qt::AlignLeft | Qt::AlignVCenter, data.strFontName);

        QString strImgPrefix = "";
        DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
        if (DGuiApplicationHelper::DarkType == appHelper->themeType()) {
            strImgPrefix = "dark_";
        }

        QString strStatus = QString("press");
        switch (data.collectIconStatus) {
        case IconHover: {
            strStatus = QString("hover");
        } break;
        case IconPress: {
            strStatus = QString("press");
            strImgPrefix = "";
        } break;
        default: {
            strStatus = QString("normal");
        } break;
        }

        QPixmap pixmap;
        if (data.isCollected) {
            QString strImageSrc = QString(":/images/%1collection_%2.svg").arg(strImgPrefix).arg(strStatus);
            pixmap = Utils::renderSVG(strImageSrc, QSize(collectIconSize, collectIconSize));
        } else {
            QString strImageSrc = QString(":/images/%1uncollection_%2.svg").arg(strImgPrefix).arg(strStatus);
            pixmap = Utils::renderSVG(strImageSrc, QSize(collectIconSize, collectIconSize));
        }

        QRect collectIconRealRect = QRect(bgRect.right() - 35 + 2, bgRect.top() + 10 - 3,
                                          collectIconSize, collectIconSize);
        painter->drawPixmap(collectIconRealRect, pixmap);

        QRect fontPreviewRect = QRect(fontNameRect.left(), bgRect.top() + 26, bgRect.width() - 50 - collectIconSize - 15,
                                bgRect.height() - 26);
        if (data.isPreviewEnabled) {

            QFont preivewFont(data.fontInfo.familyName);
            preivewFont.setPixelSize(iFontSize);
            QString styleName = data.fontInfo.styleName;

            if (styleName.contains("Italic")) {
                preivewFont.setItalic(true);
            }

            if (styleName.contains("Regular")) {
                preivewFont.setWeight(QFont::Normal);
            } else if (styleName.contains("Bold")) {
                preivewFont.setWeight(QFont::Bold);
            } else if (styleName.contains("Light")) {
                preivewFont.setWeight(QFont::Light);
            } else if (styleName.contains("Thin")) {
                preivewFont.setWeight(QFont::Thin);
            } else if (styleName.contains("ExtraLight")) {
                preivewFont.setWeight(QFont::ExtraLight);
            } else if (styleName.contains("ExtraBold")) {
                preivewFont.setWeight(QFont::ExtraBold);
            } else if (styleName.contains("Medium")) {
                preivewFont.setWeight(QFont::Medium);
            } else if (styleName.contains("DemiBold")) {
                preivewFont.setWeight(QFont::DemiBold);
            } else if (styleName.contains("Black")) {
                preivewFont.setWeight(QFont::Black);
            }

            QFontMetrics fontMetric(preivewFont);
            QString previewText = Utils::convertToPreviewString(data.fontInfo.filePath, strFontPreview);
            QString elidedText = fontMetric.elidedText(previewText,
                                                       Qt::ElideRight,
                                                       bgRect.width() - 50 - collectIconSize - 15,
                                                       Qt::TextShowMnemonic);
            //绘制预览字体
            painter->setPen(QPen(option.palette.color(DPalette::Text)));
            painter->setFont(preivewFont);
            painter->drawText(fontPreviewRect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);
        }
        else {

            //禁用字体时使用系统默认字体显示
            QFont preivewFont;
            preivewFont.setPixelSize(iFontSize);
            QFontMetrics fontMetric(preivewFont);
            QString previewText = strFontPreview;
            QString elidedText = fontMetric.elidedText(previewText,
                                                       Qt::ElideRight,
                                                       bgRect.width() - 50 - collectIconSize - 15,
                                                       Qt::TextShowMnemonic);
            //绘制预览字体
            painter->setPen(QPen(option.palette.color(DPalette::Text)));
            painter->setFont(preivewFont);
            painter->drawText(fontPreviewRect, Qt::AlignLeft | Qt::AlignVCenter, elidedText);
        }
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
        itemHeight += static_cast<int>(((iFontSize-30)+1)*1.5);
    }
    return QSize(option.rect.width(), itemHeight);
}

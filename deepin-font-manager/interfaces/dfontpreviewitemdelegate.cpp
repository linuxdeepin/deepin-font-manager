#include "dfontpreviewitemdelegate.h"
#include "utils.h"

#include <QPainter>

#include <DApplication>
#include <DPalette>
#include <DLog>
#include <DCheckBox>

#define FTM_PREVIEW_ITEM_HEIGHT 130

#define FTM_IS_USE_ROUND_CORNER true

DFontPreviewItemDelegate::DFontPreviewItemDelegate(QAbstractItemView *parent)
    : DStyledItemDelegate(parent)
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

        QRect rect;
        rect.setX(option.rect.x());
        rect.setY(option.rect.y());
        rect.setWidth(option.rect.width());
        rect.setHeight(option.rect.height());

        if (FTM_IS_USE_ROUND_CORNER) {
            QPainterPath path;
            const int radius = 10;

            path.moveTo(rect.bottomRight() - QPointF(0, radius));
            path.lineTo(rect.topRight() + QPointF(0, radius));
            path.arcTo(QRectF(QPointF(rect.topRight() - QPointF(radius * 2, 0)), QSize(radius * 2, radius * 2)), 0, 90);
            path.lineTo(rect.topLeft() + QPointF(radius, 0));
            path.arcTo(QRectF(QPointF(rect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
            path.lineTo(rect.bottomLeft() - QPointF(0, radius));
            path.arcTo(QRectF(QPointF(rect.bottomLeft() - QPointF(0, radius * 2)), QSize(radius * 2, radius * 2)), 180, 90);
            path.lineTo(rect.bottomLeft() + QPointF(radius, 0));
            path.arcTo(QRectF(QPointF(rect.bottomRight() - QPointF(radius * 2, radius * 2)), QSize(radius * 2, radius * 2)), 270, 90);

            if (option.state & QStyle::State_Selected) {
                QColor fillColor = option.palette.color(DPalette::Background);
                painter->fillPath(path, fillColor);
            }

        } else {

            QPainterPath path;
            path.moveTo(rect.topRight());
            path.lineTo(rect.topLeft());
            path.quadTo(rect.topLeft(), rect.topLeft());
            path.lineTo(rect.bottomLeft());
            path.quadTo(rect.bottomLeft(), rect.bottomLeft());
            path.lineTo(rect.bottomRight());
            path.quadTo(rect.bottomRight(), rect.bottomRight());
            path.lineTo(rect.topRight());
            path.quadTo(rect.topRight(), rect.topRight());

            if (option.state & QStyle::State_Selected) {
                QColor fillColor = option.palette.color(DPalette::Background);
                painter->fillPath(path, fillColor);
            }
        }

        int checkBoxSize = 14;
        int collectIconSize = 22;
        //绘制数据位置
        QRect checkboxRect = QRect(rect.left() + 15, rect.top() + 8, checkBoxSize, checkBoxSize);
        QRect fontNameRect = QRect(rect.left() + 50, rect.top() + 5, rect.width() - 50 - 33, 20);
        QRect collectIconRect =
            QRect(rect.right() - 33, rect.top() + 10, collectIconSize, collectIconSize);
        QRect fontPreviewRect;

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

        checkBoxOption.rect = checkboxRect;
        DApplication::style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &checkBoxOption, painter,
                                             &checkBox);

        QFont nameFont;
        nameFont.setPixelSize(14);
        painter->setPen(QPen(option.palette.color(DPalette::Text)));
        painter->setFont(nameFont);
        painter->drawText(fontNameRect, Qt::AlignLeft | Qt::AlignTop, data.strFontName);

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
        QString strElidedText = fontMetric.elidedText(strFontPreview, Qt::ElideRight,
                                                      rect.width() - 50 - 33, Qt::TextShowMnemonic);

        int yOffset = 0;
        if (iFontSize > 55) {
            yOffset = 20;
        }

        fontPreviewRect = QRect(rect.left() + 50, rect.top() + yOffset, rect.width() - 50,
                                rect.height() - yOffset);
        //绘制预览字体
        painter->setPen(QPen(option.palette.color(DPalette::Text)));
        painter->setFont(preivewFont);
        painter->drawText(fontPreviewRect, Qt::AlignLeft | Qt::AlignVCenter, strElidedText);

        QString strStatus = QString("press");
//        switch (data.collectIconStatus) {
//        case IconHover: {
//            strStatus = QString("hover");
//        } break;
//        case IconPress: {
//            strStatus = QString("press");
//        } break;
//        default: {
//            strStatus = QString("normal");
//        } break;
//        }

        QPixmap pixmap;
        if (data.isCollected) {
            QString strCollectionImageSrc = QString(":/images/collection_%1.svg").arg(strStatus);
            if (IconPress == data.collectIconStatus) {
                qDebug() << strCollectionImageSrc << endl;
            }
            pixmap =
                Utils::renderSVG(strCollectionImageSrc, QSize(collectIconSize, collectIconSize));
        } else {
            QString strUnCollectionImageSrc = QString(":/images/uncollection_%1.svg").arg(strStatus);
            if (IconPress == data.collectIconStatus) {
                qDebug() << strUnCollectionImageSrc << endl;
            }
            pixmap =
                Utils::renderSVG(strUnCollectionImageSrc, QSize(collectIconSize, collectIconSize));
        }

        painter->drawPixmap(collectIconRect, pixmap);

        painter->restore();
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize DFontPreviewItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QSize(option.rect.width(), FTM_PREVIEW_ITEM_HEIGHT);
}

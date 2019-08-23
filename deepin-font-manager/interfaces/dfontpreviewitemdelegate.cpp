#include "dfontpreviewitemdelegate.h"
#include "utils.h"

#include <QCheckBox>
#include <QEvent>
#include <QImageReader>
#include <QMouseEvent>
#include <QPainter>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QSvgGenerator>

#include <DApplication>
#include <DLog>

DTK_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

DFontPreviewItemDelegate::DFontPreviewItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void DFontPreviewItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->save();

        QVariant variant = index.data(Qt::DisplayRole);
        DFontPreviewItemData data = variant.value<DFontPreviewItemData>();

        QStyleOptionViewItem viewOption(option);  //用来在视图中画一个item

        QRectF rect;
        rect.setX(option.rect.x());
        rect.setY(option.rect.y());
        rect.setWidth(option.rect.width());
        rect.setHeight(option.rect.height());

        //绘制数据位置
        QRect checkboxRect = QRect(rect.left() + 15, rect.top() + 8, 13, 13);
        QRect fontNameRect = QRect(rect.left() + 50, rect.top() + 5, rect.width() - 50 - 33, 20);
        QRect collectIconRect = QRect(rect.right() - 33, rect.top() + 10, 16, 16);
        QRect fontPreviewRect =
            QRect(rect.left() + 50, rect.top() + 20, rect.width(), rect.height());

        QCheckBox checkBox;
        //绘制checkbox
        QStyleOptionButton checkBoxOption;
        bool checked = data.bEnabled;
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

        QFont nameFont("SourceHanSansSC-Medium");
        nameFont.setPixelSize(14);
        painter->setPen(QPen(Qt::black));
        painter->setFont(nameFont);
        painter->drawText(fontNameRect, Qt::AlignLeft, data.strFontName);

        QFont preivewFont(data.strFontName);
        preivewFont.setPixelSize(30);
        painter->setPen(QPen(Qt::black));
        painter->setFont(preivewFont);
        painter->drawText(fontPreviewRect, Qt::AlignLeft, data.strFontPreview);

        QString strStatus = QString("normal");
        QString strCollectionImageSrc = QString(":/images/collection_%1.svg").arg(strStatus);
        QString strUnCollectionImageSrc = QString(":/images/uncollection_%1.svg").arg(strStatus);

        QPixmap pixmap;
        if (data.bCollected) {
            pixmap = Utils::renderSVG(strCollectionImageSrc, QSize(16, 16));
        } else {
            pixmap = Utils::renderSVG(strUnCollectionImageSrc, QSize(16, 16));
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
    return QSize(option.rect.width(), 72);
}

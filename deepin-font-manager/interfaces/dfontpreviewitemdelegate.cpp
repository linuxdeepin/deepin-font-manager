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

        QVariant varFontName = index.data(Qt::UserRole);
        QVariant varFontPreviewText = index.data(Qt::UserRole + 1);
        QVariant variant = index.data(Qt::DisplayRole);

        DFontPreviewItemData data = variant.value<DFontPreviewItemData>();

        QString strFontName = data.strFontName;
        QString strFontPreview = data.strFontPreview;

        if (!varFontName.isNull()) {
            strFontName = varFontName.toString();
        }

        if (!varFontPreviewText.isNull()) {
            strFontPreview = varFontPreviewText.toString();
        }

        QStyleOptionViewItem viewOption(option);  //用来在视图中画一个item

        QRectF rect;
        rect.setX(option.rect.x());
        rect.setY(option.rect.y());
        rect.setWidth(option.rect.width());
        rect.setHeight(option.rect.height());

        int checkBoxSize = 14;
        int collectIconSize = 22;
        //绘制数据位置
        QRect checkboxRect = QRect(rect.left() + 15, rect.top() + 8, checkBoxSize, checkBoxSize);
        QRect fontNameRect = QRect(rect.left() + 50, rect.top() + 5, rect.width() - 50 - 33, 20);
        QRect collectIconRect =
            QRect(rect.right() - 33, rect.top() + 10, collectIconSize, collectIconSize);
        QRect fontPreviewRect =
            QRect(rect.left() + 50, rect.top() + 20, rect.width() - 50, rect.height());

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

        QFont nameFont;
        nameFont.setPixelSize(14);
        painter->setPen(QPen(Qt::black));
        painter->setFont(nameFont);
        painter->drawText(fontNameRect, Qt::AlignLeft, strFontName);

        QFont preivewFont(data.pFontInfo->familyName);
        preivewFont.setPixelSize(30);
        QString styleName = data.pFontInfo->styleName;

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

        //        qDebug() << "rect.width():" << rect.width() << endl;
        QFontMetrics fm(preivewFont);
        //        qDebug() << "fm.width(strFontPreview):" << fm.width(strFontPreview) << endl;
        QString strElidedText =
            fm.elidedText(strFontPreview, Qt::ElideRight, rect.width() - 100, Qt::TextShowMnemonic);

        //绘制预览字体
        painter->setPen(QPen(Qt::black));
        painter->setFont(preivewFont);
        painter->drawText(fontPreviewRect, Qt::AlignLeft, strElidedText);

        QString strStatus = QString("normal");
        QString strCollectionImageSrc = QString(":/images/collection_%1.svg").arg(strStatus);
        QString strUnCollectionImageSrc = QString(":/images/uncollection_%1.svg").arg(strStatus);

        QPixmap pixmap;
        if (data.bCollected) {
            pixmap =
                Utils::renderSVG(strCollectionImageSrc, QSize(collectIconSize, collectIconSize));
        } else {
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
    return QSize(option.rect.width(), 72);
}

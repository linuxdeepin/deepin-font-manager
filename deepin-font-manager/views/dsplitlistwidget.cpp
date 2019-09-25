#include "dsplitlistwidget.h"
#include "globaldef.h"
#include "utils.h"

#include <QPainter>
#include <QMouseEvent>

#include <DStyleHelper>
#include <DApplication>
#include <DApplicationHelper>
#include <DLog>

#define FTM_SPLIT_TOP_SPACE_TAG "_space_"
#define FTM_SPLIT_TOP_SPLIT_TAG "_split_"
#define FTM_SPLIT_LINE_INDEX    6

DNoFocusDelegate::DNoFocusDelegate(QAbstractItemView *parent)
    :DStyledItemDelegate(parent)
    , m_parentView(parent)
{
}

//用于去除选中项的边框
void DNoFocusDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    if (index.isValid()) {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        QVariant varDisplay = index.data(Qt::DisplayRole);
        QString strTitle = varDisplay.value<QString>();

        QStyleOptionViewItem viewOption(option);  //用来在视图中画一个item

        DPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? DPalette::Normal : DPalette::Disabled;
        if (cg == DPalette::Normal && !(option.state & QStyle::State_Active)) {
            cg = DPalette::Inactive;
        }

        if (strTitle.startsWith(FTM_SPLIT_TOP_SPACE_TAG)) {
            //用于ListView顶部空白
        }
        else if (strTitle.startsWith(FTM_SPLIT_TOP_SPLIT_TAG)) {

            QRect lineRect;
            lineRect.setX(option.rect.x()+10);
            lineRect.setY(option.rect.y()+option.rect.height()-2);
            lineRect.setWidth(option.rect.width()-20);
            lineRect.setHeight(2);

            //绘制分割线
            DPalette pa = DApplicationHelper::instance()->palette(m_parentView);
            DStyleHelper styleHelper;
            QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::ItemBackground);
            painter->fillRect(lineRect, fillColor);
        }
        else {

            QRect rect;
            rect.setX(option.rect.x());
            rect.setY(option.rect.y());
            rect.setWidth(option.rect.width());
            rect.setHeight(option.rect.height());

            QRect paintRect = QRect(rect.left()+10, rect.top(), rect.width()-20, rect.height());

            QPainterPath path;
            const int radius = 8;

            path.moveTo(paintRect.bottomRight() - QPoint(0, radius));
            path.lineTo(paintRect.topRight() + QPoint(0, radius));
            path.arcTo(QRect(QPoint(paintRect.topRight() - QPoint(radius * 2, 0)), QSize(radius * 2, radius * 2)), 0, 90);
            path.lineTo(paintRect.topLeft() + QPoint(radius, 0));
            path.arcTo(QRect(QPoint(paintRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
            path.lineTo(paintRect.bottomLeft() - QPoint(0, radius));
            path.arcTo(QRect(QPoint(paintRect.bottomLeft() - QPoint(0, radius * 2)), QSize(radius * 2, radius * 2)), 180, 90);
            path.lineTo(paintRect.bottomLeft() + QPoint(radius, 0));
            path.arcTo(QRect(QPoint(paintRect.bottomRight() - QPoint(radius * 2, radius * 2)), QSize(radius * 2, radius * 2)), 270, 90);

            if (option.state & QStyle::State_Selected) {
                QColor fillColor = option.palette.color(cg, DPalette::Highlight);
                painter->setBrush(QBrush(fillColor));
                painter->fillPath(path, painter->brush());
            }

            //绘制标题
            QRect fontNameRect = QRect(rect.left()+20, rect.top()+(rect.height()-20)/2, rect.width()-20, 20);

            QString fontFamilyName = Utils::loadFontFamilyFromFiles(":/images/SourceHanSansCN-Medium.ttf");
            QFont nameFont(fontFamilyName);
            nameFont.setWeight(QFont::Medium);
            nameFont.setPixelSize(14);
            painter->setFont(nameFont);

            if (option.state & QStyle::State_Selected) {
                painter->setPen(QPen(option.palette.color(DPalette::HighlightedText)));
                painter->drawText(fontNameRect, Qt::AlignLeft | Qt::AlignVCenter, strTitle);
            }
            else {
                painter->setPen(QPen(option.palette.color(DPalette::Text)));
                painter->drawText(fontNameRect, Qt::AlignLeft | Qt::AlignVCenter, strTitle);
            }
        }

        painter->restore();
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize DNoFocusDelegate::sizeHint(const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    Q_UNUSED(index)

    int rowIndex = index.row();
    if (0 == rowIndex) {
        return QSize(option.rect.width(), 10);
    }
    else if (FTM_SPLIT_LINE_INDEX == rowIndex) {
        return QSize(option.rect.width(), 24);
    }
    else {
        return QSize(option.rect.width(), 36);
    }
}

DSplitListWidget::DSplitListWidget(QWidget *parent)
    : DListView(parent)
{
    //去除选中项的边框
    this->setItemDelegate(new DNoFocusDelegate(this));
    this->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
    this->setAutoScroll(false);

    initListData();
}

DSplitListWidget::~DSplitListWidget() {}

void DSplitListWidget::initListData()
{
    m_titleStringList << DApplication::translate("Category", "All Fonts")
                      << DApplication::translate("Category", "System Fonts")
                      << DApplication::translate("Category", "User Fonts")
                      << DApplication::translate("Category", "My Favorite")
                      << DApplication::translate("Category", "Active Fonts")
                      << DApplication::translate("Category", "Chinese")
                      << DApplication::translate("Category", "Monospace");

    for(int i=0; i<m_titleStringList.size(); i++)
    {
        QString titleString = m_titleStringList.at(i);
        m_titleStringIndexMap.insert(titleString, i);
    }

    m_categoryItemModell = new QStandardItemModel;

    int iTitleIndex = 0;
    for (int i = 0; i < m_titleStringList.size()+2; i++) {
        QStandardItem *item = new QStandardItem;
        if (0 == i) {
            item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
            item->setData(QVariant::fromValue(QString(FTM_SPLIT_TOP_SPACE_TAG)), Qt::DisplayRole);
        }
        else if (FTM_SPLIT_LINE_INDEX == i) {
            item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
            item->setData(QVariant::fromValue(QString(FTM_SPLIT_TOP_SPLIT_TAG)), Qt::DisplayRole);
        }
        else {
            QString titleString = m_titleStringList.at(iTitleIndex++);
            item->setData(QVariant::fromValue(titleString), Qt::DisplayRole);
        }

        m_categoryItemModell->appendRow(item);
    }

    this->setModel(m_categoryItemModell);

    //设置默认选中
    QModelIndex index = m_categoryItemModell->index(1, 0);
    selectionModel()->select(index, QItemSelectionModel::Select);
}

void DSplitListWidget::mouseReleaseEvent(QMouseEvent *event)
{
    DListView::mouseReleaseEvent(event);

    QPoint selectionPoint = event->pos();

    QModelIndex modelIndex = indexAt(selectionPoint);

    if (modelIndex.row() <=0 || FTM_SPLIT_LINE_INDEX == modelIndex.row()){
        return;
    }

    QStandardItem *item = m_categoryItemModell->item(modelIndex.row());
    QVariant varUserData = item->data(Qt::DisplayRole).value<QVariant>();
    qDebug() << "varUserData" << varUserData << endl;
    int realIndex = m_titleStringIndexMap.value(varUserData.toString());

    emit onListWidgetItemClicked(realIndex);
}

#include "dinstallerrorlistview.h"
#include "globaldef.h"

#include <QPainter>
#include <QMouseEvent>

DInstallErrorListDelegate::DInstallErrorListDelegate(QAbstractItemView *parent)
    :DStyledItemDelegate(parent)
{
}

//用于去除选中项的边框
void DInstallErrorListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
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
            painter->fillPath(path, fillColor);
        }

        //绘制标题
        QRect fontNameRect = QRect(rect.left()+20, rect.top(), rect.width()-20, rect.height());

        QFont nameFont;
        nameFont.setPixelSize(14);
        painter->setFont(nameFont);

        if (option.state & QStyle::State_Selected) {
            painter->setPen(QPen(option.palette.color(DPalette::Text)));
            painter->drawText(fontNameRect, Qt::AlignLeft | Qt::AlignVCenter, strTitle);
        }
        else {
            painter->setPen(QPen(option.palette.color(DPalette::Text)));
            painter->drawText(fontNameRect, Qt::AlignLeft | Qt::AlignVCenter, strTitle);
        }

        painter->restore();
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize DInstallErrorListDelegate::sizeHint(const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    Q_UNUSED(index)

    return QSize(option.rect.width(), 38);
}




DWIDGET_USE_NAMESPACE

DInstallErrorListView::DInstallErrorListView(QWidget *parent)
    : DListView(parent)
{
    QWidget *topSpaceWidget = new QWidget;
    topSpaceWidget->setFixedSize(this->width(), 10);
    this->addHeaderWidget(topSpaceWidget);

    setMouseTracking(true);

    initErrorListData();
    initDelegate();
    initConnections();
}

DInstallErrorListView::~DInstallErrorListView()
{
}

void DInstallErrorListView::initErrorListData()
{
}

void DInstallErrorListView::initDelegate()
{
//    m_fontPreviewItemDelegate = new DFontPreviewItemDelegate(this);
//    this->setItemDelegate(m_fontPreviewItemDelegate);

//    m_fontPreviewProxyModel = new DFontPreviewProxyModel(this);
//    m_fontPreviewProxyModel->setSourceModel(m_fontPreviewItemModel);
//    m_fontPreviewProxyModel->setDynamicSortFilter(true);
//    this->setModel(m_fontPreviewProxyModel);
}

void DInstallErrorListView::initConnections()
{
//    connect(this, &DInstallErrorListView::onClickEnableButton, this,
//            &DInstallErrorListView::onListViewItemEnableBtnClicked);
//    connect(this, &DInstallErrorListView::onClickCollectionButton, this,
//            &DInstallErrorListView::onListViewItemCollectionBtnClicked);
//    connect(this, &DInstallErrorListView::onShowContextMenu, this,
//            &DInstallErrorListView::onListViewShowContextMenu, Qt::ConnectionType::QueuedConnection);
}

void DInstallErrorListView::mouseMoveEvent(QMouseEvent *event)
{
    DListView::mouseMoveEvent(event);

    QPoint mousePos = event->pos();

    QModelIndex rowModelIndex = indexAt(event->pos());
    QRect rect = rectForIndex(rowModelIndex);

    int collectIconSize = 22;
    QRect collectIconRect =
        QRect(rect.right() - 33, rect.top() + 10, collectIconSize, collectIconSize);

//    DFontPreviewItemData itemData =
//        qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(rowModelIndex));

//    if (collectIconRect.contains(mousePos)) {
//        itemData.collectIconStatus = IconHover;
//    }
//    else {
//        itemData.collectIconStatus = IconNormal;
//    }
//    m_fontPreviewProxyModel->setData(rowModelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
}

void DInstallErrorListView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_bLeftMouse = true;
    } else {
        m_bLeftMouse = false;
    }

    DListView::mousePressEvent(event);

//    QPoint mousePos = event->pos();

    QModelIndex rowModelIndex = indexAt(event->pos());
    QRect rect = rectForIndex(rowModelIndex);

    int collectIconSize = 22;
    QRect collectIconRect =
        QRect(rect.right() - 33, rect.top() + 10, collectIconSize, collectIconSize);

//    DFontPreviewItemData itemData =
//        qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(rowModelIndex));

//    if (collectIconRect.contains(mousePos)) {
//        itemData.collectIconStatus = IconPress;
//    }
//    else {
//        itemData.collectIconStatus = IconNormal;
//    }
//    m_fontPreviewProxyModel->setData(rowModelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
}

void DInstallErrorListView::mouseReleaseEvent(QMouseEvent *event)
{
    DListView::mouseReleaseEvent(event);

//    QPoint selectionPoint = event->pos();

//    QModelIndex rowModelIndex = indexAt(selectionPoint);
//    m_currModelIndex = rowModelIndex;

//    DFontPreviewItemData itemData =
//        qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(rowModelIndex));

//    itemData.collectIconStatus = IconNormal;
//    m_fontPreviewProxyModel->setData(rowModelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);

//    if (selectionPoint.x() < 50) {
//        //触发启用/禁用字体
//        emit onClickEnableButton(rowModelIndex);
//    } else if ((selectionPoint.x() > (this->width() - 50)) &&
//               (selectionPoint.x() < this->width())) {
//        //触发收藏/取消收藏
//        emit onClickCollectionButton(rowModelIndex);
//    }
}

void DInstallErrorListView::setSelection(const QRect &rect,
                                        QItemSelectionModel::SelectionFlags command)
{
    DListView::setSelection(rect, command);

    QPoint selectionPoint(rect.x(), rect.y());
    QModelIndex modelIndex = indexAt(selectionPoint);
    m_currModelIndex = modelIndex;

}

void DInstallErrorListView::setModel(QAbstractItemModel *model)
{
    m_fontPreviewItemModel = qobject_cast<QStandardItemModel *>(model);
    DListView::setModel(model);
}

void DInstallErrorListView::setRightContextMenu(QMenu *rightMenu)
{
    m_rightMenu = rightMenu;
}

QModelIndex DInstallErrorListView::currModelIndex()
{
    return m_currModelIndex;
}

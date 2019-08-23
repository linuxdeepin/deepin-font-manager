#include "dfontpreviewlistview.h"
#include "dfontpreviewitemdelegate.h"

#include <DLog>
#include <DMenu>

DWIDGET_USE_NAMESPACE

DFontPreviewListView::DFontPreviewListView(QWidget *parent)
    : DListView(parent)
{
    initConnections();
}

void DFontPreviewListView::initConnections()
{
    connect(this, &DFontPreviewListView::onClickEnableButton, this,
            &DFontPreviewListView::onListViewItemEnableBtnClicked);
    connect(this, &DFontPreviewListView::onClickCollectionButton, this,
            &DFontPreviewListView::onListViewItemCollectionBtnClicked);
    connect(this, &DFontPreviewListView::onShowContextMenu, this,
            &DFontPreviewListView::onListViewShowContextMenu);
}

void DFontPreviewListView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_bLeftMouse = true;
    } else {
        m_bLeftMouse = false;
    }
    DListView::mousePressEvent(event);
}

void DFontPreviewListView::setSelection(const QRect &rect,
                                        QItemSelectionModel::SelectionFlags command)
{
    Q_UNUSED(command)

    QPoint selectionPoint(rect.x(), rect.y());
    QModelIndex modelIndex = indexAt(selectionPoint);
    qDebug() << modelIndex.row() << endl;
    m_currModelIndex = modelIndex;

    if (m_bLeftMouse) {
        if (selectionPoint.x() < 50) {
            //触发启用/禁用字体
            emit onClickEnableButton(modelIndex);
        } else if ((selectionPoint.x() > (this->width() - 50)) &&
                   (selectionPoint.x() < this->width())) {
            //触发收藏/取消收藏
            emit onClickCollectionButton(modelIndex);
        }
        return;
    } else {
        emit onShowContextMenu(modelIndex);
    }
}

void DFontPreviewListView::setModel(QAbstractItemModel *model)
{
    m_fontPreviewItemModel = qobject_cast<QStandardItemModel *>(model);
    DListView::setModel(model);
}

void DFontPreviewListView::onListViewItemEnableBtnClicked(QModelIndex index)
{
    DFontPreviewItemData itemData =
        qvariant_cast<DFontPreviewItemData>(m_fontPreviewItemModel->data(index));
    itemData.bEnabled = !itemData.bEnabled;

    m_fontPreviewItemModel->setData(index, QVariant::fromValue(itemData), Qt::DisplayRole);
}

void DFontPreviewListView::onListViewItemCollectionBtnClicked(QModelIndex index)
{
    DFontPreviewItemData itemData =
        qvariant_cast<DFontPreviewItemData>(m_fontPreviewItemModel->data(index));
    itemData.bCollected = !itemData.bCollected;

    m_fontPreviewItemModel->setData(index, QVariant::fromValue(itemData), Qt::DisplayRole);
}

void DFontPreviewListView::onListViewShowContextMenu(QModelIndex index)
{
    Q_UNUSED(index)

    DMenu *rightMenu = new DMenu(this);
#warning need internationalization
    QString strAddFont = QString("添加字体");
    QString strEnableFont = QString("启用字体");
    QString strDeleteFont = QString("删除字体");
    QString strCollectFont = QString("收藏");
    QString strDisplayFontInfo = QString("显示信息");
    QString strShowFontFileInExplorer = QString("在文件管理器中显示");

    QAction *addFontAction = rightMenu->addAction(strAddFont);
    QAction *enableFontAction = rightMenu->addAction(strEnableFont);
    QAction *deletFontAction = rightMenu->addAction(strDeleteFont);
    QAction *collectFontAction = rightMenu->addAction(strCollectFont);
    QAction *displayFontInfoAction = rightMenu->addAction(strDisplayFontInfo);
    QAction *showFontFileInExplorerAction = rightMenu->addAction(strShowFontFileInExplorer);

    connect(addFontAction, SIGNAL(triggered(bool)), this, SLOT(onClickAddFontRightMenu()));
    connect(enableFontAction, SIGNAL(triggered(bool)), this, SLOT(onClickEnableFontRightMenu()));
    connect(deletFontAction, SIGNAL(triggered(bool)), this, SLOT(onClickDeleteFontRightMenu()));
    connect(collectFontAction, SIGNAL(triggered(bool)), this, SLOT(onClickCollectFontRightMenu()));
    connect(displayFontInfoAction, SIGNAL(triggered(bool)), this,
            SLOT(onClickDisplayFontInfoRightMenu()));
    connect(showFontFileInExplorerAction, SIGNAL(triggered(bool)), this,
            SLOT(onClickShowFontFileRightMenu()));

    //在当前鼠标位置显示
    rightMenu->exec(QCursor::pos());
}

void DFontPreviewListView::onClickAddFontRightMenu()
{
    emit onAddFont(m_currModelIndex);
}

void DFontPreviewListView::onClickEnableFontRightMenu()
{
    emit onEnableFont(m_currModelIndex);
}

void DFontPreviewListView::onClickDeleteFontRightMenu()
{
    emit onDeleteFont(m_currModelIndex);
}

void DFontPreviewListView::onClickCollectFontRightMenu()
{
    emit onCollectFont(m_currModelIndex);
}

void DFontPreviewListView::onClickDisplayFontInfoRightMenu()
{
    emit onDisplayFontInfo(m_currModelIndex);
}

void DFontPreviewListView::onClickShowFontFileRightMenu()
{
    emit onShowFontFile(m_currModelIndex);
}

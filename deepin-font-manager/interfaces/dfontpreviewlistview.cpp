#include "dfontpreviewlistview.h"
#include "dfontinfomanager.h"
#include "dfontpreviewitemdelegate.h"
#include "globaldef.h"
#include "dfmxmlwrapper.h"
#include "views/dfontmgrmainwindow.h"

#include <DLog>
#include <DMenu>
#include <DGuiApplicationHelper>
#include <DApplication>
#include <DMessageManager>

#include <QFontDatabase>
#include <QSet>

DWIDGET_USE_NAMESPACE

bool DFontPreviewListView::misdelete = false;

DFontPreviewListView::DFontPreviewListView(QWidget *parent)
    : QListView(parent)
    , m_bLoadDataFinish(false)
    , m_parentWidget(parent)
    , m_fontPreviewItemModel(new QStandardItemModel())
    , m_dataThread(nullptr)
{
    qRegisterMetaType<DFontPreviewItemData>("DFontPreviewItemData");
    qRegisterMetaType<QList<DFontPreviewItemData>>("QList<DFontPreviewItemData> &");
    qRegisterMetaType<QItemSelection>("QItemSelection");
    m_fontPreviewItemModel->setColumnCount(1);
    setFrameShape(QFrame::NoFrame);
    connect(this, &DFontPreviewListView::itemsSelected, this, &DFontPreviewListView::selectFonts);
    connect(this, &DFontPreviewListView::itemSelected, this, &DFontPreviewListView::selectFont);
    //    connect(this, &DFontPreviewListView::itemAdded, this, &DFontPreviewListView::onItemAdded);
    connect(this, &DFontPreviewListView::multiItemsAdded, this, &DFontPreviewListView::onMultiItemsAdded);
    connect(this, &DFontPreviewListView::itemRemoved, this, &DFontPreviewListView::onItemRemoved);
    connect(this, &DFontPreviewListView::itemRemovedFromSys, this, &DFontPreviewListView::onItemRemovedFromSys);
    connect(this, &DFontPreviewListView::requestUpdateModel, this, &DFontPreviewListView::updateModel);
    DFontMgrMainWindow *mw = qobject_cast<DFontMgrMainWindow *>(m_parentWidget);
    if (mw)
        connect(this, &DFontPreviewListView::requestShowSpinner, mw, &DFontMgrMainWindow::onShowSpinner, Qt::BlockingQueuedConnection);

    connect(m_signalManager, &SignalManager::cancelDel, this, &DFontPreviewListView::cancelDel);
    /*切换listview后，scrolltotop UT000539*/
    connect(m_signalManager, &SignalManager::changeView, this, &DFontPreviewListView::viewChanged);

    m_timerOfFreshed = new QTimer();//scrollTo after deleter
    connect(m_timerOfFreshed, &QTimer::timeout, this, &DFontPreviewListView::isNeedScrollToBottom);

    m_dataThread = DFontPreviewListDataThread::instance(this);

    //    setAutoScroll(true);
    setMouseTracking(true);
    setUpdatesEnabled(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    if (mw)
        connect(mw, &DFontMgrMainWindow::requestDeleted, this, [ = ](const QStringList files) {
        qDebug() << " requestDeleted";
        Q_EMIT requestDeleted(files);
    });
    initFontListData();

    initDelegate();
    initConnections();
}

DFontPreviewListView::~DFontPreviewListView()
{
}

void DFontPreviewListView::initFontListData()
{
    emit onLoadFontsStatus(0);
    //qDebug() << "main thread id = " << QThread::currentThreadId();
    //    connect(m_dataThread, SIGNAL(resultReady()), this, SLOT(onFinishedDataLoad()));
}

bool DFontPreviewListView::isListDataLoadFinished()
{
    return m_bLoadDataFinish;
}

void DFontPreviewListView::isNeedScrollToBottom()
{
    if (this->count() == m_fontPreviewProxyModel->rowCount()) {
        if (!isAtListviewBottom() && m_bListviewAtButtom) {
            scrollToBottom();
        }
        m_timerOfFreshed->stop();
    }
}

void DFontPreviewListView::refreshFontListData(const QStringList &installFont)
{
    qDebug() << __func__ << "S" << endl;
    m_dataThread->refreshFontListData(false, installFont);

    QList<DFontPreviewItemData> diffFontInfoList = m_dataThread->getDiffFontModelList();
    for (int i = 0; i < diffFontInfoList.size(); ++i) {
        DFontPreviewItemData itemData = diffFontInfoList.at(i);
        QString filePath = itemData.fontInfo.filePath;
        Q_EMIT itemAdded(itemData);
        m_dataThread->addPathWatcher(filePath);

        /* Bug#16821 UT000591  添加字体后需要加入到Qt的字体数据库中，否则无法使用*/
        QFontDatabase::addApplicationFont(filePath);
    }
    Q_EMIT itemsSelected(installFont);
}

void DFontPreviewListView::onFinishedDataLoad()
{
    qDebug() << "onFinishedDataLoad thread id = " << QThread::currentThreadId();
    QList<DFontPreviewItemData> fontInfoList = m_dataThread->getFontModelList();
    //qDebug() << fontInfoList.size();
    for (int i = 0; i < fontInfoList.size(); ++i) {
        DFontPreviewItemData itemData = fontInfoList.at(i);
        QString filePath = itemData.fontInfo.filePath;
        QFileInfo filePathInfo(filePath);
        //如果字体文件已经不存在，则从t_manager表中删除
        if (!filePathInfo.exists()) {
            //删除字体之前启用字体，防止下次重新安装后就被禁用
            enableFont(itemData.fontInfo.filePath);
            DFMDBManager::instance()->deleteFontInfo(itemData);
            continue;
        } else {
            //add it to file system watcher
            m_dataThread->addPathWatcher(filePath);
        }

        Q_EMIT itemAdded(itemData);
    }
    DFMDBManager::instance()->commitDeleteFontInfo();
    enableFonts();

    m_bLoadDataFinish = true;
    emit onLoadFontsStatus(1);
}

void DFontPreviewListView::onItemAdded(const DFontPreviewItemData &itemData)
{
    QStandardItemModel *sourceModel = qobject_cast<QStandardItemModel *>(m_fontPreviewProxyModel->sourceModel());
    QStandardItem *item = new QStandardItem;
    item->setData(QVariant::fromValue(itemData), Qt::DisplayRole);
    sourceModel->appendRow(item);
}

void DFontPreviewListView::onMultiItemsAdded(QList<DFontPreviewItemData> &data)
{
    if (data.isEmpty())
        return;
    QStandardItemModel *sourceModel = qobject_cast<QStandardItemModel *>(m_fontPreviewProxyModel->sourceModel());
    int rows = sourceModel->rowCount();
    //    qDebug() << __FUNCTION__ << data.size() << rows;

    int i = 0;
    qDebug() << "start" << endl;
    bool res = sourceModel->insertRows(rows, data.size());
    if (!res) {
        qDebug() << __FUNCTION__ << "insertRows fail";
        return;
    }

    qDebug() << __FUNCTION__ << "rows = " << sourceModel->rowCount();
    for (DFontPreviewItemData &itemData : data) {
        QModelIndex index = sourceModel->index(rows + i,   0);

        int appFontId = QFontDatabase::addApplicationFont(itemData.fontInfo.filePath);
        itemData.appFontId = appFontId;

        res = sourceModel->setData(index, QVariant::fromValue(itemData), Qt::DisplayRole);
        if (!res)
            qDebug() << __FUNCTION__ << "setData fail";
        i++;
    }
}

void DFontPreviewListView::onItemRemoved(const DFontPreviewItemData &itemData)
{

    /*UT000539 删除后刷新选中*/
    if (m_fontPreviewProxyModel == nullptr)
        return;

    QFontDatabase::removeApplicationFont(itemData.appFontId);
    deleteFontModelIndex(itemData.fontInfo.filePath);

    /*UT000539 刷新删除后选中状态*/
    if (m_selectAfterDel != -1) {
        m_bListviewAtButtom = isAtListviewBottom();
        m_bListviewAtTop = isAtListviewTop();
        DFontPreviewProxyModel *filterModel = this->getFontPreviewProxyModel();
        if (m_bListviewAtButtom && !m_bListviewAtTop) {
            QModelIndex modelIndex = filterModel->index(m_selectAfterDel - 1, 0);
            setCurrentIndex(modelIndex);
        } else if (m_selectAfterDel == filterModel->rowCount()) {
            QModelIndex modelIndex = filterModel->index(m_selectAfterDel - 1, 0);
            setCurrentIndex(modelIndex);
        } else {
            QModelIndex modelIndex = filterModel->index(m_selectAfterDel, 0);
            setCurrentIndex(modelIndex);
        }
        isSelectedNow = true;
    }
}

void DFontPreviewListView::onItemRemovedFromSys(const DFontPreviewItemData &itemData)
{
    if (m_fontPreviewProxyModel == nullptr)
        return;

    qDebug() << __FUNCTION__ << ", path " << itemData.fontInfo.filePath << QThread::currentThreadId();
    QFontDatabase::removeApplicationFont(itemData.appFontId);
    deleteFontModelIndex(itemData.fontInfo.filePath, true);

    if (m_selectAfterDel != -1) {
        m_bListviewAtButtom = isAtListviewBottom();
        m_bListviewAtTop = isAtListviewTop();
        DFontPreviewProxyModel *filterModel = this->getFontPreviewProxyModel();
        if (m_bListviewAtButtom && !m_bListviewAtTop) {
            QModelIndex modelIndex = filterModel->index(m_selectAfterDel - 1, 0);
            setCurrentIndex(modelIndex);
        } else if (m_selectAfterDel == filterModel->rowCount()) {
            QModelIndex modelIndex = filterModel->index(m_selectAfterDel - 1, 0);
            setCurrentIndex(modelIndex);
        } else {
            QModelIndex modelIndex = filterModel->index(m_selectAfterDel, 0);
            setCurrentIndex(modelIndex);
        }
        isSelectedNow = true;
    }
//    QItemSelectionModel *selection_model = selectionModel();
//    //如果是删除多项，就要清空选中。
//    selection_model->clearSelection();
//    m_bListviewAtButtom = isAtListviewBottom();
//    m_bListviewAtTop = isAtListviewTop();
//    if (currModelIndex().row() == this->count()) {
//        QModelIndex modelIndex = m_fontPreviewItemModel->index(this->count() - 1, 0);
//        selection_model->select(modelIndex, QItemSelectionModel::Select);
//    } else  if (m_bListviewAtButtom && !m_bListviewAtTop) {
//        QModelIndex modelIndexbottom = m_fontPreviewItemModel->index(currModelIndex().row() - 1, 0);
//        selection_model->select(modelIndexbottom, QItemSelectionModel::Select);
//    } else {
//        selection_model->select(currModelIndex(), QItemSelectionModel::Select);
//    }


}

void DFontPreviewListView::updateCurrentFontGroup(int currentFontGroup)
{
    switch (currentFontGroup) {
    case 0: {
        m_currentFontGroup = FontGroup::AllFont;
        break;
    }
    case 1: {
        m_currentFontGroup = FontGroup::SysFont;
        break;
    }
    case 2: {
        m_currentFontGroup = FontGroup::UserFont;
        break;
    }
    case 3: {
        m_currentFontGroup = FontGroup::CollectFont;
        break;
    }
    case 4: {
        m_currentFontGroup = FontGroup::ActiveFont;
        break;
    }
    case 5: {
        m_currentFontGroup = FontGroup::ChineseFont;
        break;
    }
    case 6: {
        m_currentFontGroup = FontGroup::EqualWidthFont;
        break;
    }
    default:
        break;
    }

}

void DFontPreviewListView::initDelegate()
{
    m_fontPreviewItemDelegate = new DFontPreviewItemDelegate(this);
    this->setItemDelegate(m_fontPreviewItemDelegate);

    m_fontPreviewProxyModel = new DFontPreviewProxyModel();
    m_fontPreviewProxyModel->setSourceModel(m_fontPreviewItemModel);
    m_fontPreviewProxyModel->setDynamicSortFilter(true);

    QListView::setModel(m_fontPreviewProxyModel);
}

void DFontPreviewListView::initConnections()
{
    connect(this, &DFontPreviewListView::onClickEnableButton, this,
            &DFontPreviewListView::onListViewItemEnableBtnClicked);
    connect(this, &DFontPreviewListView::onClickCollectionButton, this,
            &DFontPreviewListView::onListViewItemCollectionBtnClicked);
    //    connect(this, &DFontPreviewListView::onShowContextMenu, this,
    //            &DFontPreviewListView::onListViewShowContextMenu, Qt::ConnectionType::QueuedConnection);

    connect(m_signalManager, &SignalManager::currentFontGroup, this, &DFontPreviewListView::updateCurrentFontGroup);

    /*UT000539 设置focus状态、设置选中状态 */
    connect(m_signalManager, &SignalManager::refreshFocus, this, &DFontPreviewListView::refreshFocuses);
}

int DFontPreviewListView::count() const
{
    return model()->rowCount(rootIndex());
}

void DFontPreviewListView::cancelDel()
{
    m_selectAfterDel = -1;
}

void DFontPreviewListView::viewChanged()
{
    scrollToTop();
    isSelectedNow = false;
    m_currentSelectedRow = -1;
}

void DFontPreviewListView::refreshFocuses(int count)
{
    if (selectionModel()->selectedIndexes().isEmpty())
        return;
    m_isJustInstalled = true;
    QTimer::singleShot(50, [ = ]() {
        setFocus(Qt::MouseFocusReason);
        if (1 == count) {
            setCurrentSelected(selectionModel()->selectedIndexes().first().row());
            scrollTo(currentIndex());
        } else if (count > 1) {
//                m_currentSelectedRow = -1;
            if (selectedIndexes().count() > 0) {
                setCurrentSelected(selectedIndexes().first().row());
            }
            isSelectedNow = false;
            if (selectionModel()->selectedIndexes().count() > 1) {
                scrollTo(selectionModel()->selectedIndexes().first());
            }
        }
    });
}

void DFontPreviewListView::updateModel()
{
    m_bListviewAtButtom = isAtListviewBottom();
    m_bListviewAtTop = isAtListviewTop();
    DFontMgrMainWindow *mw = qobject_cast<DFontMgrMainWindow *>(m_parentWidget);

//    QStringList uninstallFonts = selectedFonts(nullptr, nullptr);

    int rowCnt = m_fontPreviewItemModel->rowCount();
    m_fontPreviewItemModel->removeRows(0, rowCnt);
    for (int i = rowCnt; i >= 0; i--) {
        QStandardItem *item = m_fontPreviewItemModel->takeItem(i);
        if (item)
            delete item;
    }

    m_fontPreviewItemModel->clear();
    delete m_fontPreviewItemModel;
    delete m_fontPreviewProxyModel;

    m_fontPreviewItemModel = new QStandardItemModel;
    m_fontPreviewItemModel->setColumnCount(1);
    m_fontPreviewProxyModel = new DFontPreviewProxyModel;
    m_fontPreviewProxyModel->setSourceModel(m_fontPreviewItemModel);
    setModel(m_fontPreviewProxyModel);

    m_fontPreviewProxyModel->setFilterKeyColumn(0);

    if (mw)
        m_fontPreviewProxyModel->setFilterGroup(mw->currentFontGroup());
    QFontDatabase::removeAllApplicationFonts();

    QList<DFontPreviewItemData> modelist = m_dataThread->getFontModelList();
    onMultiItemsAdded(modelist);
    emit m_signalManager->fontSizeRequestToSlider();//设置预览大小

    /*UT000539 刷新删除后选中状态*/
    if (m_selectAfterDel != -1) {
        DFontPreviewProxyModel *filterModel = this->getFontPreviewProxyModel();
        if (m_bListviewAtButtom && !m_bListviewAtTop) {
            QModelIndex modelIndex = filterModel->index(m_selectAfterDel - 1, 0);
            setCurrentIndex(modelIndex);
        } else if (m_selectAfterDel == filterModel->rowCount()) {
            QModelIndex modelIndex = filterModel->index(m_selectAfterDel - 1, 0);
            setCurrentIndex(modelIndex);
        } else {
            QModelIndex modelIndex = filterModel->index(m_selectAfterDel, 0);
            setCurrentIndex(modelIndex);
        }
        isSelectedNow = true;
    }

    if (m_bListviewAtButtom)
        m_timerOfFreshed->start(2);
}

QRect DFontPreviewListView::getCollectionIconRect(QRect visualRect)
{
    int collectIconSize = 22 + 10;
    return QRect(visualRect.right() - 10 - 33, visualRect.top() + 10 - 5, collectIconSize, collectIconSize);
}

// ut000442 对选中字体的索引按照row从大到小进行排序，为了在我的收藏界面和已激活界面进行操作时
// 涉及到删除listview中item时从后往前删，避免在删除过程中索引出错的问题。
void DFontPreviewListView::sortModelIndexList(QModelIndexList &sourceList)
{
    QModelIndex temp;
    bool flag;//是否交换的标志
    for (int i = 0; i < sourceList.count() - 1; i++) {

        flag = false;
        for (int j = sourceList.count() - 1; j > i; j--) { //选出该趟排序的最大值向前移动

            if (sourceList[j].row() > sourceList[j - 1].row()) {
                temp = sourceList[j];
                sourceList[j] = sourceList[j - 1];
                sourceList[j - 1] = temp;
                flag = true;    //只要有发生了交换，flag就置为true
            }
        }
        // 判断标志位是否为false，如果为false，说明后面的元素已经有序，就直接return
        if (!flag)
            break;
    }
}

void DFontPreviewListView::deleteFontModelIndex(const QString &filePath, bool isFromSys)
{
    Q_UNUSED(isFromSys);
    qDebug() << __FUNCTION__ << "m_fontPreviewProxyModel rowcount = " << m_fontPreviewProxyModel->rowCount();
    if (m_fontPreviewItemModel && m_fontPreviewItemModel->rowCount() == 0) {
        return;
    }

    for (int i = 0; i < m_fontPreviewProxyModel->sourceModel()->rowCount(); i++) {
        QModelIndex modelIndex = m_fontPreviewProxyModel->sourceModel()->index(i, 0);
        QVariant varModel = m_fontPreviewProxyModel->sourceModel()->data(modelIndex, Qt::DisplayRole);

        DFontPreviewItemData itemData = varModel.value<DFontPreviewItemData>();

        if (itemData.fontInfo.filePath == filePath) {
            m_fontPreviewProxyModel->sourceModel()->removeRow(i);
            break;
        }
    }
}

bool DFontPreviewListView::isDeleting()
{
    DFontMgrMainWindow *mw = qobject_cast<DFontMgrMainWindow *>(m_parentWidget);
    if (mw)
        return mw->isDeleting();
    return false;
}

void DFontPreviewListView::selectFonts(const QStringList &fileList)
{
    //    Q_EMIT DFontManager::instance()->batchInstall("onlyprogress", 98);
    if (fileList.isEmpty())
        return;

    QItemSelection selection;
    qDebug() << __FUNCTION__ << " fileList size " << fileList.size() << ", row count " << getFontPreviewProxyModel()->rowCount();
    for (int i = 0; i < getFontPreviewProxyModel()->rowCount(); ++i) {
        QModelIndex index = getFontPreviewProxyModel()->index(i, 0);
        DFontPreviewItemData itemData =
            qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(index));
        //        qDebug() << __FUNCTION__ << itemData.fontInfo.filePath;
        if (fileList.contains(itemData.fontInfo.filePath)) {
            QModelIndex left = m_fontPreviewProxyModel->index(index.row(), 0);
            QModelIndex right = m_fontPreviewProxyModel->index(index.row(), m_fontPreviewProxyModel->columnCount() - 1);
            QItemSelection sel(left, right);
            selection.merge(sel, QItemSelectionModel::Select);
        }
    }

    qDebug() << " selection size " << selection.size();

    QItemSelectionModel *selection_model = selectionModel();
    if (selection.size() > 0)  {
        selection_model->reset();
        selection_model->select(selection, QItemSelectionModel::Select);
    }

    QModelIndex cur = currModelIndex();
//    if (cur.isValid())
//        scrollTo(cur);

    if (selection.size() == 1)
        setCurrentIndex(cur);

    //    DFontMgrMainWindow *mw = qobject_cast<DFontMgrMainWindow *>(m_parentWidget);
    //    if (mw)
    //        Q_EMIT mw->requestUpdatePreview();

    Q_EMIT SignalManager::instance()->requestInstallAdded();
    /*用于安装后刷新聚焦、安装后focus for ctrl+a UT000539*/
    Q_EMIT m_signalManager->refreshFocus(fileList.size());

    //    Q_EMIT DFontManager::instance()->batchInstall("onlyprogress", 100);
}

void DFontPreviewListView::selectFont(const QString &file)
{
    QItemSelection selection;

    for (int i = 0; i < getFontPreviewProxyModel()->rowCount(); ++i) {
        QModelIndex index = getFontPreviewProxyModel()->index(i, 0);
        DFontPreviewItemData itemData =
            qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(index));
        //        qDebug() << __FUNCTION__ << itemData.fontInfo.filePath;
        if (!file.compare(itemData.fontInfo.filePath)) {
            QModelIndex left = m_fontPreviewProxyModel->index(index.row(), 0);
            QModelIndex right = m_fontPreviewProxyModel->index(index.row(), m_fontPreviewProxyModel->columnCount() - 1);
            QItemSelection sel(left, right);
            selection.merge(sel, QItemSelectionModel::Select);
        }
    }

    qDebug() << " selection size " << selection.size();

    QItemSelectionModel *selection_model = selectionModel();
    if (selection.size() > 0)  {
        selection_model->reset();
        selection_model->select(selection, QItemSelectionModel::Select);
    }

    QModelIndex cur = currModelIndex();

//    if (cur.isValid())
//        scrollTo(cur);

    selection_model->reset();
    setCurrentIndex(cur);
    //    DFontMgrMainWindow *mw = qobject_cast<DFontMgrMainWindow *>(m_parentWidget);
    //    if (mw)
    //        Q_EMIT mw->requestUpdatePreview();

}

QMutex *DFontPreviewListView::getMutex()
{
    return &m_mutex;
}

void DFontPreviewListView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    //    qDebug() << __FUNCTION__ << selected << " deselected " << deselected;
    QListView::selectionChanged(selected, deselected);
}

void DFontPreviewListView::mouseMoveEvent(QMouseEvent *event)
{
    if (m_fontPreviewItemModel && m_fontPreviewItemModel->rowCount() == 0) {
        return;
    }

    if (!m_fontPreviewProxyModel) {
        return;
    }

    QPoint clickPoint = event->pos();

    QModelIndex modelIndex = indexAt(clickPoint);
    QRect rect = visualRect(modelIndex);

    QRect collectIconRect = getCollectionIconRect(rect);

    DFontPreviewItemData itemData = qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(modelIndex));
    clearHoverState();///*UT000539*/
    if (collectIconRect.contains(clickPoint)) {
        //        if (itemData.collectIconStatus != IconHover) {
        itemData.collectIconStatus = IconHover;
        m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
        //        }
        //        qDebug() << "+++++++++++++" << itemData.strFontName << endl;
        m_hoverModelIndex = modelIndex;
    } else {
        if (itemData.collectIconStatus != IconNormal) {
            itemData.collectIconStatus =  IconNormal;
            m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
        }
    }

    QListView::mouseMoveEvent(event);
}

void DFontPreviewListView::mousePressEvent(QMouseEvent *event)
{

    QListView::mousePressEvent(event);// 获取鼠标在点击窗体上的坐标
    QPoint clickPoint = event->pos();
    QModelIndex modelIndex = indexAt(clickPoint);
    if (event->button() == Qt::LeftButton) {
        m_bLeftMouse = true;
        m_bRightMous = false;
        /*UT000539*/
        if (QApplication::keyboardModifiers() == Qt::ShiftModifier) {
            if (-1 != m_currentSelectedRow) {
                if (modelIndex.row() == -1) {
                    return;
                } else if (m_currentSelectedRow < modelIndex.row()) {
                    selectionModel()->clear();
                    for (int i = m_currentSelectedRow; i <= modelIndex.row(); i++) {
                        QModelIndex modelIndex1 = m_fontPreviewProxyModel->index(i, 0);
                        selectionModel()->select(modelIndex1, QItemSelectionModel::Select);
                    }
                } else if (m_currentSelectedRow > modelIndex.row()) {
                    selectionModel()->clear();
                    for (int i = modelIndex.row(); i <= m_currentSelectedRow; i++) {
                        QModelIndex modelIndex1 = m_fontPreviewProxyModel->index(i, 0);

                        selectionModel()->select(modelIndex1, QItemSelectionModel::Select);
                    }
                } else {
                    selectionModel()->clear();
                    selectionModel()->select(modelIndex, QItemSelectionModel::Select);
                }
            } else {
                selectionModel()->clear();
                for (int i = 0; i <= modelIndex.row(); i++) {
                    QModelIndex modelIndex1 = m_fontPreviewProxyModel->index(i, 0);
                    selectionModel()->select(modelIndex1, QItemSelectionModel::Select);
                }
            }
        } else {
            m_currentSelectedRow = modelIndex.row();
            isSelectedNow = true;
        }
    } else {
        m_bLeftMouse = false;
        if (event->button() == Qt::RightButton) {
            m_bRightMous = true;
            //DMenu *rightMenu = m_rightMenu;
            //在当前鼠标位置显示 `
            if (!modelIndex.isValid()) {
                return;
            }
            if (!m_rightMenu->isVisible()) {
                connect(m_rightMenu, &QMenu::aboutToHide, this, [ = ] {
                    clearPressState();
                });
                if (!this->selectedIndexes().contains(modelIndex)) {
                    this->setCurrentIndex(modelIndex);
                }
                DFontPreviewItemData itemData = qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(modelIndex));
                itemData.collectIconStatus =  IconNormal;
                m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
                Q_EMIT m_signalManager->refreshFocus(this->selectedIndexes().count());
                m_rightMenu->exec(QCursor::pos());
                return;
            }
        } else {
            m_bRightMous = false;
        }

    }

    if (m_fontPreviewItemModel && m_fontPreviewItemModel->rowCount() == 0) {
        return;
    }

    QRect rect = visualRect(modelIndex);

    m_curRect = rect;

    //    selectionModel()->clear();
    //    selectionModel()->selectedIndexes().append(modelIndex);

    QRect collectIconRect = getCollectionIconRect(rect);
    int checkBoxSize = 20 + 10;
    QRect checkboxRealRect = QRect(rect.left() + 25, rect.top() + 10 - 5, checkBoxSize, checkBoxSize);

    if (m_bLeftMouse && (collectIconRect.contains(clickPoint) || checkboxRealRect.contains(clickPoint))) {
        m_bClickCollectionOrEnable = true;
    } else {
        m_bClickCollectionOrEnable = false;
    }

    DFontPreviewItemData itemData = qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(modelIndex));

    if (collectIconRect.contains(clickPoint)) {
        if (itemData.collectIconStatus != IconPress) {
            if (Qt::LeftButton == event->button()) {//取消press状态/*UT000539*/
                itemData.collectIconStatus = IconPress;
            }
            m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
        }
        m_pressModelIndex = modelIndex;
    } else if (itemData.collectIconStatus != IconNormal) {
        itemData.collectIconStatus = IconNormal;
        m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
    }

    if (m_bClickCollectionOrEnable) {
        return;
    }

    //    mouseMoveEvent(event)


}
void DFontPreviewListView::mouseReleaseEvent(QMouseEvent *event)
{

    QListView::mouseReleaseEvent(event);

    if (m_fontPreviewItemModel && m_fontPreviewItemModel->rowCount() == 0) {
        return;
    }
    if (Qt::MidButton == event->button()) {
        return;
    }

    QPoint clickPoint = event->pos();

    QModelIndex modelIndex = indexAt(clickPoint);

    QModelIndexList indexList;
    indexList << modelIndex;
    m_currModelIndex = modelIndex;

    DFontPreviewItemData itemData =
        qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(modelIndex));

    if (itemData.collectIconStatus != IconNormal) {
        itemData.collectIconStatus = IconNormal;
        m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
    }

    QRect rect = visualRect(modelIndex);

    int checkBoxSize = 20 + 10;
    QRect checkboxRealRect = QRect(rect.left() + 25, rect.top() + 10 - 5, checkBoxSize, checkBoxSize);
    QRect collectIconRect = getCollectionIconRect(rect);

    if (event->button() == Qt::LeftButton) {/*539 排除右键点击效果*/
        if (checkboxRealRect.contains(clickPoint)) {
            //触发启用/禁用字体
            //        emit onClickEnableButton(indexList, !itemData.isEnabled);
            if (m_currentFontGroup != FontGroup::ActiveFont) {
                if (itemData.isCanDisable)
                    onListViewItemEnableBtnClicked(indexList, !itemData.isEnabled);
            } else {
                if (itemData.isCanDisable)
                    onListViewItemEnableBtnClicked(indexList, !itemData.isEnabled, true);
            }


        } else if (collectIconRect.contains(clickPoint)) {
            //触发收藏/取消收藏
            //        emit onClickCollectionButton(modelIndex);
            if (m_currentFontGroup != FontGroup::CollectFont) {
                onListViewItemCollectionBtnClicked(indexList, !itemData.isCollected);
            } else {
                onListViewItemCollectionBtnClicked(indexList, !itemData.isCollected, true);
            }

        }
    }

    //ut000442 bug20343 鼠标点击右侧爱心时不移动，松开时再次获取此时鼠标位置的item，再去判断鼠标点击
    //点还在不在爱心上，这样就能正确显示相应效果。

    //我的搜藏界面,index点击之后会涉及到index的变化,所以需要重新获取一遍
    if (m_currentFontGroup == FontGroup::CollectFont) {
        modelIndex = currModelIndex();
        rect = visualRect(modelIndex);
        checkboxRealRect = QRect(rect.left() + 25, rect.top() + 10 - 5, checkBoxSize, checkBoxSize);
        collectIconRect = getCollectionIconRect(rect);
    }


    DFontPreviewItemData m_NextItemData =
        qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(modelIndex));

    if (collectIconRect.contains(clickPoint)) {
        if (m_NextItemData.collectIconStatus != IconHover) {
            m_NextItemData.collectIconStatus = IconHover;
            m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(m_NextItemData), Qt::DisplayRole);
        }
        m_hoverModelIndex = modelIndex;
    } else if (m_NextItemData.collectIconStatus != IconNormal) {
        m_NextItemData.collectIconStatus = IconNormal;
        m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(m_NextItemData), Qt::DisplayRole);
    }


    m_bClickCollectionOrEnable = false;
}

void DFontPreviewListView::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
}

void DFontPreviewListView::setSelection(const QRect &rect,
                                        QItemSelectionModel::SelectionFlags command)
{
    QPoint clickPoint(rect.x(), rect.y());
    QModelIndex modelIndex = indexAt(clickPoint);
    m_currModelIndex = modelIndex;

    if (!m_bLeftMouse && m_bRightMous) {
        emit onShowContextMenu(modelIndex);
    }

    if (m_bClickCollectionOrEnable) {
        return;
    }
    QListView::setSelection(rect, command);
}

void DFontPreviewListView::setModel(QAbstractItemModel *model)
{
//    m_fontPreviewItemModel = qobject_cast<QStandardItemModel *>(model);
    QListView::setModel(model);
}

void DFontPreviewListView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    //    qDebug() << __FUNCTION__ << currentIndex() << "begin" << start << " to " << end;
    selectionModel()->setCurrentIndex(parent, QItemSelectionModel::NoUpdate);
    QListView::rowsAboutToBeRemoved(parent, start, end);
    //    qDebug() << __FUNCTION__ << currentIndex() << "end";
}

void DFontPreviewListView::keyPressEvent(QKeyEvent *event)
{
    if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_End)) {
        scrollToBottom();
    } else if ((event->modifiers() == Qt::ControlModifier) && (event->key() == Qt::Key_Home)) {
        scrollToTop();
    } else {
        if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down) {
            QModelIndexList list = selectedIndexes();
            if (QApplication::keyboardModifiers() == Qt::ShiftModifier || list.count() < 1 || QApplication::keyboardModifiers() == Qt::CTRL) {
                QListView::keyPressEvent(event);
                return;
            }
            if (event->key() == Qt::Key_Up) {
                if (isAtListviewTop()) {
                    if (list.last().row() == 0) {
                        QModelIndex modelIndex = m_fontPreviewProxyModel->index(this->count() - 1, 0);
                        setCurrentIndex(modelIndex);
                        return;
                    }
                } else {
                    if (list.last().row() == 0) {
                        scrollToTop();
                        return;
                    }
                }
            } else if (event->key() == Qt::Key_Down) {
                if (isAtListviewBottom()) {
                    if (list.last().row() == this->count() - 1) {
                        QModelIndex modelIndex = m_fontPreviewProxyModel->index(0, 0);
                        setCurrentIndex(modelIndex);
                        return;
                    }
                } else {
                    if (list.last().row() == this->count() - 1) {
                        scrollToBottom();
                    }
                }
            }
        }
        QListView::keyPressEvent(event);
    }
}

void DFontPreviewListView::enableFont(const QString &filePath)
{
    if (!m_enableFontList.contains(filePath))
        m_enableFontList << filePath;
}

void DFontPreviewListView::disableFont(const QString &filePath)
{
    if (!m_disableFontList.contains(filePath))
        m_disableFontList << filePath;
}

void DFontPreviewListView::enableFonts()
{
    if (m_enableFontList.isEmpty())
        return;
    qDebug() << __FUNCTION__ << m_enableFontList.size();

    QString fontConfigPath = DFMXmlWrapper::m_fontConfigFilePath;
    bool isCreateSuccess = DFMXmlWrapper::createFontConfigFile(fontConfigPath);

    if (!isCreateSuccess) {
        return;
    }

    DFMXmlWrapper::deleteNodeWithTextList(fontConfigPath, "pattern", m_enableFontList);
    m_enableFontList.clear();
}

void DFontPreviewListView::disableFonts()
{
    if (m_disableFontList.isEmpty())
        return;
    qDebug() << __FUNCTION__ << m_disableFontList.size();

    QString fontConfigPath = DFMXmlWrapper::m_fontConfigFilePath;
    bool isCreateSuccess = DFMXmlWrapper::createFontConfigFile(fontConfigPath);

    if (!isCreateSuccess) {
        return;
    }

    DFMXmlWrapper::addPatternNodesWithTextList(fontConfigPath, "rejectfont", m_disableFontList);
    m_disableFontList.clear();
}
//设置收藏页面删除后的选中
void DFontPreviewListView::toSetCurrentIndex(QModelIndexList &itemIndexesNew)
{
    m_bListviewAtButtom = isAtListviewBottom();
    m_bListviewAtTop = isAtListviewTop();
    int i = itemIndexesNew.last().row();
    DFontPreviewProxyModel *filterModel = this->getFontPreviewProxyModel();
    if ((m_bListviewAtButtom && !m_bListviewAtTop)/* || m_bListviewAtButtom*/) {
        QModelIndex modelIndex = filterModel->index(i - 1, 0);
        setCurrentIndex(modelIndex);
        //            qDebug() << "modelIndex+++++++++++++++++++++++++" << modelIndex << endl;
    } else if (itemIndexesNew.last().row() == filterModel->rowCount()) {

        QModelIndex modelIndex = filterModel->index(itemIndexesNew.last().row() - 1, 0);
        setCurrentIndex(modelIndex);
    } else {
        setCurrentIndex(itemIndexesNew.last());
        //            qDebug() << "modelIndex+++++++++++++++++++++++++" << itemIndexesNew.last() << endl;
    }
}

//设置激活页面删除后的选中
void DFontPreviewListView::toSetCurrentIndex(QModelIndexList &itemIndexesNew, int count, int size)
{
    m_bListviewAtButtom = isAtListviewBottom();
    m_bListviewAtTop = isAtListviewTop();
    int i = itemIndexesNew.last().row();

    DFontPreviewProxyModel *filterModel = this->getFontPreviewProxyModel();
    if ((m_bListviewAtButtom && !m_bListviewAtTop)/* || m_bListviewAtButtom*/) {
        if (count == 0) {//激活页面禁用字体未成功
            QModelIndex modelIndex = filterModel->index(i, 0);
            setCurrentIndex(modelIndex);
        } else {
            QModelIndex modelIndex = filterModel->index(i - 1, 0);
            setCurrentIndex(modelIndex);
        }
    } else if (m_bListviewAtTop && !m_bListviewAtButtom) {
        QModelIndex modelIndex = filterModel->index(i, 0);
        if (modelIndex.isValid()) {
            setCurrentIndex(modelIndex);
        } else {
            QModelIndex modelIndexUp = filterModel->index(i - 1, 0);
            setCurrentIndex(modelIndexUp);
        }
    } else {
        if (i < size) {
            QModelIndex modelIndex = filterModel->index(i, 0);
            setCurrentIndex(modelIndex);
        } else if (i == size) {
            if (count == 0) {
                QModelIndex modelIndex = filterModel->index(size, 0);
                setCurrentIndex(modelIndex);
            } else {
                QModelIndex modelIndex = filterModel->index(size - 1, 0);
                setCurrentIndex(modelIndex);
            }
        }
    }
}

bool DFontPreviewListView::isAtListviewBottom()
{
    if (this->verticalScrollBar()->value() >= this->verticalScrollBar()->maximum() - 50) {
        return true;
    } else {
        return false;
    }
}

bool DFontPreviewListView::isAtListviewTop()
{
    if (this->verticalScrollBar()->value() <= this->verticalScrollBar()->minimum() + 50) {
        return true;
    } else {
        return false;
    }
}

QString DFontPreviewListView::getPreviewTextWithSize(int *fontSize)
{
    DFontMgrMainWindow *mw = qobject_cast<DFontMgrMainWindow *>(m_parentWidget);
    if (mw)
        return mw->getPreviewTextWithSize(fontSize);
    if (fontSize != nullptr)
        *fontSize = FTM_DEFAULT_PREVIEW_FONTSIZE;
    return QString(DApplication::translate("Font", "Don't let your dreams be dreams"));
}

void DFontPreviewListView::setCurrentSelected(int indexRow)
{
    m_currentSelectedRow = indexRow;
}

void DFontPreviewListView::onListViewItemEnableBtnClicked(const QModelIndexList &itemIndexes, bool setValue, bool isFromActiveFont)
{
    bool needShowTips = false;
    int count = 0;
    int size = this->count();
    QMutexLocker locker(&m_mutex);
    QString fontName;
    QModelIndexList itemIndexesNew = itemIndexes;

    sortModelIndexList(itemIndexesNew);

    //    for (int i = 0; i < itemIndexes.count(); i++) {
    //        qDebug() << itemIndexes[i].row() << endl;
    //        itemIndexesNew.append(itemIndexes[itemIndexes.count() - 1 - i]);
    //    }

    QList<DFontPreviewItemData> modelist = m_dataThread->getFontModelList();

    for (QModelIndex &index : itemIndexesNew) {
        //        DFontPreviewItemData itemData =
        //            qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(itemIndexes[0]));
        DFontPreviewItemData itemData =
            qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(index));
        QFileInfo fi(itemData.fontInfo.filePath);
        if (!fi.exists())
            continue;
//        itemData.isEnabled = setValue;

        //        qDebug() << __FUNCTION__ << "familyName" << itemData.fontInfo.familyName << endl;

        int idx = modelist.indexOf(itemData);
        if (setValue) {
            enableFont(itemData.fontInfo.filePath);
            itemData.isEnabled = setValue;
        } else {
            if (!itemData.isCanDisable) {
                needShowTips = true;
            } else {
                if (index == itemIndexes[0]) {
                    fontName = itemData.strFontName;
                }
                disableFont(itemData.fontInfo.filePath);
                itemData.isEnabled = setValue;
                count++;
            }
        }
        m_dataThread->updateItemStatus(idx, itemData);//更新状态
        DFMDBManager::instance()->updateFontInfo(itemData, "isEnabled");

        m_fontPreviewProxyModel->setData(index, QVariant::fromValue(itemData), Qt::DisplayRole);

    }

    DFMDBManager::instance()->commitUpdateFontInfo();

    if (setValue) {
        enableFonts();
        return;
    } else {
        disableFonts();
    }

    if (isFromActiveFont == true) {
        toSetCurrentIndex(itemIndexesNew, count, size);
    }


    QString message;
    if (needShowTips) {
        //不可禁用字体
        if (count >= 1) {
            Q_EMIT rowCountChanged();
        }
//        message = DApplication::translate("MessageManager", "Some fonts are not allowed to be disabled");
        message = DApplication::translate("MessageManager", "部分系统字体不允许被禁用");
    } else {
        if (count == 1) {
            message = QString("%1 %2").arg(fontName).arg(DApplication::translate("MessageManager", "deactivated"));
        } else if (count > 1) {
            message = DApplication::translate("MessageManager", "The fonts have been deactivated");
        }
        Q_EMIT rowCountChanged();
    }
    if (count > 0)//禁用字体大于零
        DMessageManager::instance()->sendMessage(this->m_parentWidget, QIcon("://ok.svg"), message);
}

void DFontPreviewListView::onListViewItemCollectionBtnClicked(const QModelIndexList &index, bool setValue, bool isFromCollectFont)
{
    QMutexLocker locker(&m_mutex);
    QModelIndexList itemIndexesNew = index;
    sortModelIndexList(itemIndexesNew);
    QList<DFontPreviewItemData> modelist = m_dataThread->getFontModelList();
    for (QModelIndex &index : itemIndexesNew) {
        DFontPreviewItemData itemData =
            qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(index));
        itemData.isCollected = setValue;
        //        DFMDBManager::instance()->updateFontInfoByFontId(itemData.strFontId, "isCollected", QString::number(itemData.isCollected));
        DFMDBManager::instance()->updateFontInfo(itemData, "isCollected");
        int idx = modelist.indexOf(itemData);
        m_dataThread->updateItemStatus(idx, itemData);//更新状态
        m_fontPreviewProxyModel->setData(index, QVariant::fromValue(itemData), Qt::DisplayRole);
    }
    if (isFromCollectFont == true) {
        //      ut000442 bug 22350.通过快捷键访问时，有可能选中字体为空，添加相关判断，为空直接返回
        if (itemIndexesNew.count() == 0) {
            return;
        }
        toSetCurrentIndex(itemIndexesNew);
    }

    DFMDBManager::instance()->commitUpdateFontInfo();

    Q_EMIT rowCountChanged();
}

void DFontPreviewListView::onListViewShowContextMenu(const QModelIndex &index)
{
    Q_UNUSED(index)

    DMenu *rightMenu = m_rightMenu;

    //在当前鼠标位置显示
    connect(rightMenu, &QMenu::aboutToHide, this, [ = ] {
        clearPressState();
    });
    rightMenu->exec(QCursor::pos());
}

void DFontPreviewListView::setRightContextMenu(QMenu *rightMenu)
{
    m_rightMenu = rightMenu;
}

QModelIndex DFontPreviewListView::currModelIndex()
{
    int min = -1;
    QModelIndex minIndex;
    for (QModelIndex &index : selectedIndexes()) {
        if (min < 0 || min > index.row()) {
            min = index.row();
            minIndex = index;
        }
    }

    if (minIndex.isValid())
        m_currModelIndex = minIndex;

    return m_currModelIndex;
}

DFontPreviewItemData DFontPreviewListView::currModelData()
{
    QVariant varModel = m_fontPreviewProxyModel->data(currModelIndex(), Qt::DisplayRole);
    DFontPreviewItemData itemData = varModel.value<DFontPreviewItemData>();

    return itemData;
}

DFontPreviewProxyModel *DFontPreviewListView::getFontPreviewProxyModel()
{
    return m_fontPreviewProxyModel;
}

void DFontPreviewListView::clearPressState()
{
    if (!m_pressModelIndex.isValid())
        return;

    DFontPreviewItemData pressData =
        qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(m_pressModelIndex));
    //    qDebug() << " restore press item " << pressData.strFontName;
    pressData.collectIconStatus = IconNormal;
    m_fontPreviewProxyModel->setData(m_pressModelIndex, QVariant::fromValue(pressData), Qt::DisplayRole);
    m_pressModelIndex = QModelIndex();
}

void DFontPreviewListView::clearHoverState()
{
    if (!m_hoverModelIndex.isValid())
        return;

    DFontPreviewItemData itemData =
        qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(m_hoverModelIndex));
    //    qDebug() << " restore hover item " << itemData.strFontName;
    itemData.collectIconStatus = IconNormal;
    m_fontPreviewProxyModel->setData(m_hoverModelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
    m_hoverModelIndex = QModelIndex();
}

void DFontPreviewListView::updateChangedFile(const QString &path)
{
    qDebug() << __FUNCTION__ << path << " begin ";
    QMutexLocker locker(&m_mutex);
    changeFontFile(path);

    Q_EMIT rowCountChanged();
    qDebug() << __FUNCTION__ << path << " end ";
}

void DFontPreviewListView::updateChangedDir(const QString &path)
{
    //no different between "share" or "fonts" dir
    //    qDebug() << __FUNCTION__ << path << " begin ";
    Q_UNUSED(path);
    QMutexLocker locker(&m_mutex);
    QList<DFontPreviewItemData> fontInfoList = m_dataThread->getFontModelList();
//    qDebug() << __FUNCTION__ << fontInfoList.size();
    for (int i = 0; i < fontInfoList.size(); ++i) {
        DFontPreviewItemData itemData = fontInfoList.at(i);
        QFileInfo filePathInfo(itemData.fontInfo.filePath);
        //如果字体文件已经不存在，则从t_manager表中删除
        if (!filePathInfo.exists()) {
            //            qDebug() << __FUNCTION__ << " begin to delete font " << itemData.fontInfo.filePath;
            //删除字体之前启用字体，防止下次重新安装后就被禁用
            enableFont(itemData.fontInfo.filePath);
            DFMDBManager::instance()->deleteFontInfo(itemData);
            Q_EMIT itemRemovedFromSys(itemData);
            if (!misdelete) {
                m_dataThread->removeFontData(itemData);
            }
            m_dataThread->removePathWatcher(filePathInfo.filePath());
        }
    }
    DFMDBManager::instance()->commitDeleteFontInfo();
    enableFonts();

    Q_EMIT rowCountChanged();
    //    qDebug() << __FUNCTION__ << path << " end ";
}

void DFontPreviewListView::deleteFontFiles(const QStringList &files, bool force)
{
    if (force) {
        for (const QString &path : files) {
            bool del = QFile::remove(path);
            qDebug() << __FUNCTION__ << " force delete file " << path << del;
        }
    }

    deleteCurFonts(files);
}

void DFontPreviewListView::deleteCurFonts(const QStringList &files)
{
    qDebug() << __FUNCTION__ << " before delete " << m_dataThread->getFontModelList().size() << m_fontPreviewProxyModel->rowCount()  << m_fontPreviewProxyModel->sourceModel()->rowCount();
    QList<DFontPreviewItemData> fontInfoList = m_dataThread->getFontModelList();
    qDebug() << fontInfoList.size() << __FUNCTION__ << files.size();
    int delCnt = 0;
    int total = fontInfoList.size();
    for (int i = 0; i < total; ++i) {
        DFontPreviewItemData itemData = fontInfoList.at(i);
        QString filePath = itemData.fontInfo.filePath;
        QFileInfo filePathInfo(filePath);
        //如果字体文件已经不存在，则从t_manager表中删除
        if (files.contains(filePath)) {
            //删除字体之前启用字体，防止下次重新安装后就被禁用
            enableFont(itemData.fontInfo.filePath);
            DFMDBManager::instance()->deleteFontInfo(itemData);
//            Q_EMIT itemRemoved(itemData);
            delCnt++;
            emit SignalManager::instance()->updateUninstallDialog(itemData.fontInfo.filePath.split("/").last(), delCnt, files.size());
            m_dataThread->removeFontData(itemData);
            m_dataThread->removePathWatcher(filePath);
        }
    }
    DFMDBManager::instance()->commitDeleteFontInfo();
    //    emit SignalManager::instance()->updateUninstallDialog(QString("test"), delCnt, files.size());
//    QTimer::singleShot(50, [ = ]() {
//        emit SignalManager::instance()->closeUninstallDialog();
//    });

    misdelete = false;
    enableFonts();

    Q_EMIT requestUpdateModel();
    m_dataThread->onAutoDirWatchers();

    Q_EMIT rowCountChanged();
    Q_EMIT deleteFinished();
}

void DFontPreviewListView::changeFontFile(const QString &path, bool force)
{
    QFileInfo fi(path);
    bool isDir = fi.isDir();
    if (force) {
        bool del = QFile::remove(path);
        qDebug() << __FUNCTION__ << " force delete file " << path << del;
    }
    QList<DFontPreviewItemData> fontInfoList = m_dataThread->getFontModelList();
    qDebug() << fontInfoList.size() << __FUNCTION__ << path;
    for (int i = 0; i < fontInfoList.size(); ++i) {
        DFontPreviewItemData itemData = fontInfoList.at(i);
        QString filePath = itemData.fontInfo.filePath;
        QFileInfo filePathInfo(filePath);
        //如果字体文件已经不存在，则从t_manager表中删除
        if ((!isDir && (filePath == path) && (!filePathInfo.exists())) || (isDir && filePath.startsWith(path) && !filePathInfo.exists())) {
            //删除字体之前启用字体，防止下次重新安装后就被禁用
            enableFont(itemData.fontInfo.filePath);
            DFMDBManager::instance()->deleteFontInfo(itemData);
            Q_EMIT itemRemoved(itemData);
            if (!misdelete) {
                m_dataThread->removeFontData(itemData);
            }
            m_dataThread->removePathWatcher(filePath);
            if (!isDir)
                break;
        }
    }
    DFMDBManager::instance()->commitDeleteFontInfo();
    enableFonts();
    //    DFontInfoManager::instance()->removeFontInfo();

    if (isDir) {
        if (!QFileInfo(QDir::homePath() + "/.local/share/fonts").exists()) {
            m_dataThread->removePathWatcher(QDir::homePath() + "/.local/share/fonts");
        } else if (!QFileInfo(QDir::homePath() + "/.local/share/").exists()) {
            m_dataThread->removePathWatcher(QDir::homePath() + "/.local/share/");
        }
    }
}

QStringList DFontPreviewListView::selectedFonts(int *deleteCnt, int *systemCnt)
{
    QModelIndexList list = selectedIndexes();
    QStringList ret;
    int deleteNum = 0;
    int systemNum = 0;
    for (QModelIndex &index : list) {
        QVariant varModel = m_fontPreviewProxyModel->data(index, Qt::DisplayRole);
        DFontPreviewItemData itemData = varModel.value<DFontPreviewItemData>();
        if (!itemData.fontInfo.filePath.isEmpty()) {
            if (itemData.fontInfo.isSystemFont) {
                systemNum++;
            } else {
                deleteNum++;
                ret << itemData.fontInfo.filePath;
            }
        }
    }

    if (systemCnt)
        *systemCnt = systemNum;
    if (deleteCnt)
        *deleteCnt = deleteNum;

    qDebug() << __FUNCTION__ << ret.size();
    return ret;
}

void DFontPreviewListView::selectedFontsNum(int *deleteCnt, int *systemCnt)
{
    QModelIndexList list = selectedIndexes();

    /*539 记录删除的位置*/
    sortModelIndexList(list);
    if (list.count() > 0)
        m_selectAfterDel = list.last().row();
    qDebug() << "_____________deleteRow_____________" << m_selectAfterDel;

    int deleteNum = 0;
    int systemNum = 0;
    for (QModelIndex &index : list) {
        QVariant varModel = m_fontPreviewProxyModel->data(index, Qt::DisplayRole);
        DFontPreviewItemData itemData = varModel.value<DFontPreviewItemData>();
        if (!itemData.fontInfo.filePath.isEmpty()) {
            if (itemData.fontInfo.isSystemFont) {
                systemNum++;
            } else {
                deleteNum++;
            }
        }
    }

    if (systemCnt)
        *systemCnt = systemNum;
    if (deleteCnt)
        *deleteCnt = deleteNum;
    list.clear();
}

QModelIndexList DFontPreviewListView::selectedIndex(int *deleteCnt, int *systemCnt)
{
    QModelIndexList list = selectedIndexes();
    int deleteNum = 0;
    int systemNum = 0;
    for (QModelIndex &index : list) {
        QVariant varModel = m_fontPreviewProxyModel->data(index, Qt::DisplayRole);
        DFontPreviewItemData itemData = varModel.value<DFontPreviewItemData>();
        if (!itemData.fontInfo.filePath.isEmpty()) {
            if (itemData.fontInfo.isSystemFont) {
                systemNum++;
            } else {
                deleteNum++;
            }
        }
    }

    if (systemCnt)
        *systemCnt = systemNum;
    if (deleteCnt)
        *deleteCnt = deleteNum;

    return list;
}

//字体变化设置选中行居中UT000539 /功能暂时保留，勿删/*/
void DFontPreviewListView::scrollWithTheSelected()
{
    QModelIndexList indexes = selectionModel()->selectedIndexes();
    sortModelIndexList(indexes);
    if (indexes.isEmpty())
        return;

    qDebug() << "__________m_isJustInstalled_______" << m_isJustInstalled;
    if (this->visibleRegion().contains(m_curRect.topLeft()) || this->visibleRegion().contains(m_curRect.bottomLeft())) {
        if (m_isJustInstalled) {
            if (m_curRect.center().y() < 156) {
                scrollTo(indexes.last(), ScrollHint::PositionAtTop);
            } else if (156 <= m_curRect.center().y() && m_curRect.center().y() <= 312) {
                scrollTo(indexes.last(), ScrollHint::PositionAtCenter);
            } else if (m_curRect.center().y() > 312) {
                scrollTo(indexes.last(), ScrollHint::PositionAtBottom);
            } return;
        } else {
            scrollTo(indexes.last());
        }
        qDebug() << __FUNCTION__ << "scroll to selectionModel";
    } return;//if selecteditems is not in this visibleRegion then return
}

//记录下当前选中的位置,用于局中显示UT000539
void DFontPreviewListView::refreshRect()
{
    QModelIndexList indexes = selectionModel()->selectedIndexes();
    if (indexes.isEmpty())
        return;
    sortModelIndexList(indexes);
    m_curRect = visualRect(indexes.last());
}

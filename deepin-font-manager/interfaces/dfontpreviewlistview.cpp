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
    qRegisterMetaType<DFontSpinnerWidget::SpinnerStyles>("DFontSpinnerWidget::SpinnerStyles");
    qRegisterMetaType<qint64>("qint64 &");
    m_fontPreviewItemModel->setColumnCount(1);
    setFrameShape(QFrame::NoFrame);
    connect(this, &DFontPreviewListView::itemsSelected, this, &DFontPreviewListView::selectFonts);
    //    connect(this, &DFontPreviewListView::itemAdded, this, &DFontPreviewListView::onItemAdded);
    connect(this, &DFontPreviewListView::multiItemsAdded, this, &DFontPreviewListView::onMultiItemsAdded);
    connect(this, &DFontPreviewListView::itemRemoved, this, &DFontPreviewListView::onItemRemoved);
    connect(this, &DFontPreviewListView::itemRemovedFromSys, this, &DFontPreviewListView::onItemRemovedFromSys);
    connect(this, &DFontPreviewListView::requestUpdateModel, this, &DFontPreviewListView::updateModel);

    QObject::connect(qApp, &DApplication::fontChanged, [ this ](const QFont & font) {
        qDebug() << __FUNCTION__ << "Font changed " << m_currentFont << font.family() << font.style();
        QTimer::singleShot(200, [this] {
            onUpdateCurrentFont();
        });
    });

    DFontMgrMainWindow *mw = qobject_cast<DFontMgrMainWindow *>(m_parentWidget);
    if (mw)
        connect(this, &DFontPreviewListView::requestShowSpinner, mw, &DFontMgrMainWindow::onShowSpinner);

    connect(m_signalManager, &SignalManager::cancelDel, this, &DFontPreviewListView::cancelDel);
    /*切换listview后，scrolltotop UT000539*/
    connect(m_signalManager, &SignalManager::changeView, this, &DFontPreviewListView::viewChanged);

    m_dataThread = DFontPreviewListDataThread::instance(this);

    /*setAutoScroll(true);*/
    setMouseTracking(true);
    setUpdatesEnabled(true);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    if (mw)
        connect(mw, &DFontMgrMainWindow::requestDeleted, this, [ = ](const QStringList & files) {
        updateSpinner(DFontSpinnerWidget::Delete);
        Q_EMIT requestDeleted(files);
    });
    initFontListData();

    initDelegate();
    initConnections();
    installEventFilter(this);
}

DFontPreviewListView::~DFontPreviewListView()
{
}

void DFontPreviewListView::initFontListData()
{
    emit onLoadFontsStatus(0);
}

bool DFontPreviewListView::isListDataLoadFinished()
{
    return m_bLoadDataFinish;
}

void DFontPreviewListView::onFinishedDataLoad()
{
    qDebug() << "onFinishedDataLoad thread id = " << QThread::currentThreadId();
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

void DFontPreviewListView::onMultiItemsAdded(QList<DFontPreviewItemData> &data, DFontSpinnerWidget::SpinnerStyles styles)
{
    if (data.isEmpty())
        return;

    QMutexLocker locker(&m_mutex);
    int rows = m_fontPreviewItemModel->rowCount();
    qDebug() << __FUNCTION__ << data.size() << rows;

    int i = 0;
    bool res = m_fontPreviewItemModel->insertRows(rows, data.size());
    if (!res) {
        qDebug() << __FUNCTION__ << "insertRows fail";
        return;
    }
    QStringList curFont;
    if (styles == DFontSpinnerWidget::StartupLoad)
        curFont = DFontInfoManager::instance()->getCurrentFontFamily();

    qDebug() << __FUNCTION__ << "rows = " << m_fontPreviewItemModel->rowCount();
    for (DFontPreviewItemData &itemData : data) {
        if (itemData.appFontId < 0) {
            int appFontId = QFontDatabase::addApplicationFont(itemData.fontInfo.filePath);
            itemData.appFontId = appFontId;
            m_dataThread->updateFontId(itemData, appFontId);

            QString strFontName;
            //非中文系统字体
            if (!(itemData.fontInfo.isSystemFont && itemData.isChineseFont) && !QFontDatabase::applicationFontFamilies(appFontId).isEmpty()) {
                QString familyName = QFontDatabase::applicationFontFamilies(appFontId).first();
                if (itemData.fontInfo.familyName.contains(QChar('?')) && !familyName.contains(QChar('?'))) {
                    int index = m_dataThread->getFontModelList().indexOf(itemData);
                    QString styleName = (itemData.strFontName.split("-").length() > 1) ? itemData.strFontName.split("-").last() : QString();
                    if (styleName.isEmpty()) {
                        strFontName = QFontDatabase::applicationFontFamilies(appFontId).first();
                    } else {
                        strFontName = QString("%1-%2").arg(QFontDatabase::applicationFontFamilies(appFontId).first()).arg(styleName);
                    }
                    itemData.strFontName = strFontName;

                    m_dataThread->updateItemStatus(index, itemData);
                }
            }
        }

        QModelIndex index = m_fontPreviewItemModel->index(rows + i,   0);
        res = m_fontPreviewItemModel->setData(index, QVariant::fromValue(itemData), Qt::DisplayRole);
        if (!res)
            qDebug() << __FUNCTION__ << "setData fail";
        i++;
        // repaint spinner
        if (styles != DFontSpinnerWidget::NoLabel)
            updateSpinner(styles);
    }

    qDebug() << __FUNCTION__ << "end";
}

void DFontPreviewListView::onItemRemoved(const DFontPreviewItemData &itemData)
{
    if (m_fontPreviewProxyModel == nullptr)
        return;

    QFontDatabase::removeApplicationFont(itemData.appFontId);
    deleteFontModelIndex(itemData.fontInfo.filePath);

    updateSelection();
}

void DFontPreviewListView::onItemRemovedFromSys(const DFontPreviewItemData &itemData)
{
    if (m_fontPreviewProxyModel == nullptr)
        return;

    qDebug() << __FUNCTION__ << ", path " << itemData.fontInfo.filePath << QThread::currentThreadId();
    QFontDatabase::removeApplicationFont(itemData.appFontId);
    deleteFontModelIndex(itemData.fontInfo.filePath, true);

    updateSelection();
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
    connect(this, &DFontPreviewListView::onShowContextMenu, this,
            &DFontPreviewListView::onListViewShowContextMenu, Qt::ConnectionType::QueuedConnection);

    connect(m_signalManager, &SignalManager::currentFontGroup, this, &DFontPreviewListView::updateCurrentFontGroup);
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
    m_currentSelectedRow = -1;
}

/*记录移除前位置*/
void DFontPreviewListView::markPositionBeforeRemoved(bool isDelete, const QModelIndexList &list)
{
    if (isDelete) {
        Q_UNUSED(list)
        QModelIndexList deleteFontList = selectedIndexes();
        if (deleteFontList.count() > 0) {
            sortModelIndexList(deleteFontList);
            m_selectAfterDel = deleteFontList.last().row();
        }
    } else {
        if (list.count() > 0)
            m_selectAfterDel = list.first().row();
    }
}

/*UT000539 设置focus状态、设置选中状态 */
void DFontPreviewListView::refreshFocuses()
{
    qDebug() << __FUNCTION__;
    setFocus(Qt::MouseFocusReason);
}

/*获取一页个数*/
int DFontPreviewListView::getOnePageCount()
{
    const int defaultCount = 12;
    int height = viewport()->height();
    const QModelIndex idx = getFontPreviewProxyModel()->index(0, 0);
    if (!idx.isValid())
        return defaultCount;
    const QStyleOptionViewItem option;
    QSize size = m_fontPreviewItemDelegate->sizeHint(option, idx);
    int itemHeight = size.height();
    int  count = height / itemHeight;
    return count;
}

void DFontPreviewListView::setIsTabFocus(bool IsTabFocus)
{
    m_IsTabFocus = IsTabFocus;
}

bool DFontPreviewListView::getIsTabFocus() const
{
    return m_IsTabFocus;
}

void DFontPreviewListView::updateSpinner(DFontSpinnerWidget::SpinnerStyles style, bool force)
{
    qint64 curtm = QDateTime::currentMSecsSinceEpoch();
    //超过500ms刷新
    if (curtm - m_curTm >= 350) {
        Q_EMIT requestShowSpinner(true, force, style);
        m_curTm = QDateTime::currentMSecsSinceEpoch();
    }
}

void DFontPreviewListView::updateModel(bool showSpinner)
{
    Q_UNUSED(showSpinner)
    DFontMgrMainWindow *mw = qobject_cast<DFontMgrMainWindow *>(m_parentWidget);
    m_bListviewAtTop = isAtListviewTop();
    m_bListviewAtButtom = isAtListviewBottom();
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
    QList<DFontPreviewItemData> modelist = m_dataThread->getFontModelList();

    DFontSpinnerWidget::SpinnerStyles spinnerstyle = (showSpinner) ? DFontSpinnerWidget::Delete : DFontSpinnerWidget::NoLabel;
    onMultiItemsAdded(modelist, spinnerstyle);

    Q_EMIT requestShowSpinner(false, true, DFontSpinnerWidget::Delete);

    //设置预览大小
    emit m_signalManager->fontSizeRequestToSlider();

    //设置字体删除后的选中
    selectItemAfterRemoved(m_bListviewAtButtom, m_bListviewAtTop);

    //删除之后设置焦点
    refreshFocuses();
    Q_EMIT rowCountChanged();
    Q_EMIT deleteFinished();
}

QRect DFontPreviewListView::getCollectionIconRect(const QRect &rect)
{
    int collectIconSize = 22 + 10;
    return QRect(rect.right() - 10 - 33, rect.top() + 10 - 5, collectIconSize, collectIconSize);
}

QRect DFontPreviewListView::getCheckboxRect(const QRect &rect)
{
    int checkBoxSize = 20 + 10;
    return QRect(rect.left() + 20, rect.top() + 10 - 5, checkBoxSize, checkBoxSize);
}

void DFontPreviewListView::onUpdateCurrentFont()
{
    qDebug() << __FUNCTION__ << "begin";
    QStringList curFont = DFontInfoManager::instance()->getCurrentFontFamily();

    if (curFont.isEmpty() || curFont.size() < 3) {
        qDebug() << __FUNCTION__ << curFont << " is invalid";
        return;
    }

    if ((m_currentFont.size() == curFont.size()) && (m_currentFont.at(1) == curFont.at(1)) && (m_currentFont.at(2) == curFont.at(2)))
        return;

    DFontPreviewItemData prevFontData = m_curFontData;
    for (DFontPreviewItemData &itemData : m_dataThread->getFontModelList()) {
        if (QFileInfo(itemData.fontInfo.filePath).fileName() != curFont.at(0))
            continue;

        QString styleName;
        QStringList families = DFontInfoManager::instance()->getFontFamilyStyle(itemData.fontInfo.filePath, styleName);
        if ((families.contains(curFont.at(1)) || (itemData.fontInfo.familyName == curFont.at(1)))
                && (curFont.at(2) == itemData.fontInfo.styleName)) {
            m_currentFont = curFont;
            m_curFontData = itemData;
            qDebug() << __FUNCTION__ << " found " << curFont << itemData.fontInfo.toString();
            break;
        }
    }
    qDebug() << __FUNCTION__ << "end" << m_currentFont;
}

bool DFontPreviewListView::isCurrentFont(DFontPreviewItemData &itemData)
{
    return (itemData == m_curFontData);
}

/* ut000442 对选中字体的索引按照row从大到小进行排序，为了在我的收藏界面和已激活界面进行操作时*/
/* 涉及到删除listview中item时从后往前删，避免在删除过程中索引出错的问题。*/
void DFontPreviewListView::sortModelIndexList(QModelIndexList &sourceList)
{
    QModelIndex temp;
    //是否交换的标志
    bool flag;
    for (int i = 0; i < sourceList.count() - 1; i++) {
        flag = false;
        //选出该趟排序的最大值向前移动
        for (int j = sourceList.count() - 1; j > i; j--) {

            if (sourceList[j].row() > sourceList[j - 1].row()) {
                temp = sourceList[j];
                sourceList[j] = sourceList[j - 1];
                sourceList[j - 1] = temp;
                //只要有发生了交换，flag就置为true
                flag = true;
            }
        }
        // 判断标志位是否为false，如果为false，说明后面的元素已经有序，就直接return
        if (!flag)
            break;
    }
}

/*设置item移除后的选中*/
void DFontPreviewListView::selectItemAfterRemoved(bool isAtBottom, bool isAtTop)
{
    int param = getOnePageCount();
    if (m_selectAfterDel != -1) {
        int nextIndexRow = -1;
        DFontPreviewProxyModel *filterModel = this->getFontPreviewProxyModel();
        //删除最后一个
        if (m_selectAfterDel == this->count()) {
            if (count() > 0) {
                //上移选中
                setCurrentIndex(filterModel->index(count() - 1, 0));
                nextIndexRow = count() - 1;
            }
            scrollToBottom();
        } else {
            //超过一页时
            if (this->count() > param) {
                //删除第一页的字体
                if (m_selectAfterDel <= param) {
                    setCurrentIndex(filterModel->index(m_selectAfterDel, 0));
                    //既在顶部一页又在底部一页
                    if (m_selectAfterDel >= this->count() - param) {
                        if (isAtTop) {
                            scrollToTop();
                            return;
                        } else if (isAtBottom) {
                            scrollToBottom();
                            return;
                        }
                        scrollTo(filterModel->index(m_selectAfterDel, 0));
                        return;
                    }
                    if (m_selectAfterDel < param)
                        scrollToTop();
                    else {
                        scrollTo(filterModel->index(m_selectAfterDel, 0));
                    }
                    nextIndexRow = m_selectAfterDel;
                }
                //删除最后一页的字体
                else if (m_selectAfterDel >= this->count() - param) {
                    if (isAtBottom) {
                        setCurrentIndex(filterModel->index(m_selectAfterDel - 1, 0));
                        nextIndexRow = m_selectAfterDel - 1;
                        if (m_selectAfterDel == this->count() - param) {
                            scrollTo(filterModel->index(m_selectAfterDel - 1, 0));
                        } else {
                            scrollToBottom();
                        }
                    } else {
                        setCurrentIndex(filterModel->index(m_selectAfterDel, 0));
                        nextIndexRow = m_selectAfterDel;
                        scrollTo(filterModel->index(m_selectAfterDel, 0));
                    }
                }
                //删除中间位置的字体
                else {
                    setCurrentIndex(filterModel->index(m_selectAfterDel, 0));
                    nextIndexRow = m_selectAfterDel;
                }
            }
            //只有一页时
            else {
                if (m_selectAfterDel <= param) {
                    if (filterModel->index(m_selectAfterDel, 0).isValid()) {
                        setCurrentIndex(filterModel->index(m_selectAfterDel, 0));
                        nextIndexRow = m_selectAfterDel;
                    } else {
                        setCurrentIndex(filterModel->index(m_selectAfterDel - 1, 0));
                        nextIndexRow = m_selectAfterDel - 1;
                    }
                    scrollToTop();
                }
            }
        }
        setCurrentSelected(nextIndexRow);
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
    if (fileList.isEmpty())
        return;

    QItemSelection selection;
    qDebug() << __FUNCTION__ << " fileList size " << fileList.size() << ", row count " << getFontPreviewProxyModel()->rowCount();
    for (int i = 0; i < getFontPreviewProxyModel()->rowCount(); ++i) {
        QModelIndex index = getFontPreviewProxyModel()->index(i, 0);
        DFontPreviewItemData itemData =
            qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(index));
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

    Q_EMIT SignalManager::instance()->requestInstallAdded();
    //用于安装后刷新聚焦、安装后focus for ctrl+a UT000539
    refreshFocuses();
}

QMutex *DFontPreviewListView::getMutex()
{
    return &m_mutex;
}

void DFontPreviewListView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QListView::selectionChanged(selected, deselected);
}

void DFontPreviewListView::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_fontPreviewProxyModel) {
        return;
    }

    QPoint clickPoint = event->pos();
    QModelIndex modelIndex = indexAt(clickPoint);

    if (modelIndex.isValid()) {
        QRect rect = visualRect(modelIndex);
        QRect collectIconRect = getCollectionIconRect(rect);

        DFontPreviewItemData itemData = qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(modelIndex));

        if (m_hoverModelIndex.row() != modelIndex.row())
            clearHoverState();

        if (collectIconRect.contains(clickPoint)) {
            if (itemData.collectIconStatus != IconHover) {
                itemData.collectIconStatus = IconHover;
                m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
            }
            m_hoverModelIndex = modelIndex;
        } else {
            if (itemData.collectIconStatus != IconNormal) {
                itemData.collectIconStatus = IconNormal;
                m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
            }
        }
    }

    QListView::mouseMoveEvent(event);
}

void DFontPreviewListView::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "\n" << __FUNCTION__ << event->type() << event->button();
    QListView::mousePressEvent(event);
    QPoint clickPoint = event->pos();
    QModelIndex modelIndex = indexAt(clickPoint);

    m_isMouseClicked = true;

    if ((event->button() == Qt::LeftButton) && modelIndex.isValid()) {
        if (QApplication::keyboardModifiers() == Qt::ShiftModifier) {
            //Shift多选
            updateShiftSelect(modelIndex);
        } else {
            //左键单击
            m_currentSelectedRow = modelIndex.row();
            m_curRect = visualRect(modelIndex);

            QRect collectIconRect = getCollectionIconRect(m_curRect);

            DFontPreviewItemData itemData = qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(modelIndex));

            if (collectIconRect.contains(clickPoint)) {
                if (itemData.collectIconStatus != IconPress) {
                    itemData.collectIconStatus = IconPress;
                    m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
                }
                m_pressModelIndex = modelIndex;
            } else if (itemData.collectIconStatus != IconNormal) {
                itemData.collectIconStatus = IconNormal;
                m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
            }
        }
    } else if ((event->button() == Qt::RightButton)  && modelIndex.isValid()) {
        //右键单击
        if (!this->selectedIndexes().contains(modelIndex)) {
            this->setCurrentIndex(modelIndex);
        }
        //右键index设置为shift起始位置
        setCurrentSelected(modelIndex.row());
        //恢复normal状态
        DFontPreviewItemData itemData = qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(modelIndex));
        if (itemData.collectIconStatus != IconNormal) {
            itemData.collectIconStatus =  IconNormal;
            m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
        }

        onListViewShowContextMenu(modelIndex);
        refreshFocuses();
    }
}

void DFontPreviewListView::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << __FUNCTION__ << " begin";
    QListView::mouseReleaseEvent(event);

    if (Qt::MidButton == event->button()) {
        return;
    }

    QPoint clickPoint = event->pos();

    QModelIndex modelIndex = indexAt(clickPoint);

    QModelIndexList indexList;
    m_currModelIndex = modelIndex;

    DFontPreviewItemData itemData =
        qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(modelIndex));

    QRect rect = visualRect(modelIndex);
    QRect checkboxRealRect = getCheckboxRect(rect);
    QRect collectIconRect = getCollectionIconRect(rect);
    //539 排除右键点击效果
    if (event->button() == Qt::LeftButton) {
        if (checkboxRealRect.contains(clickPoint)) {
            //触发启用/禁用字体
            int sysFontCnt = (itemData.isEnabled && itemData.fontInfo.isSystemFont) ? 1 : 0;
            int curFontCnt = (itemData == m_curFontData) ? 1 : 0;
            if (sysFontCnt == 0 && curFontCnt == 0)
                indexList << modelIndex;
            onEnableBtnClicked(indexList, sysFontCnt, curFontCnt, !itemData.isEnabled, m_currentFontGroup == FontGroup::ActiveFont);
        } else if (collectIconRect.contains(clickPoint)) {
            //恢复normal状态
            if (itemData.collectIconStatus != IconNormal) {
                itemData.collectIconStatus = IconNormal;
                m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
            }
            //触发收藏/取消收藏
            indexList << modelIndex;
            onCollectBtnClicked(indexList, !itemData.isCollected, (m_currentFontGroup == FontGroup::CollectFont));
        }

        //我的收藏界面,index点击之后会涉及到index的变化,所以需要重新获取一遍
        if (m_currentFontGroup == FontGroup::CollectFont) {
            qDebug() << __FUNCTION__ << " collect font grp";
            modelIndex = currModelIndex();
            rect = visualRect(modelIndex);
            checkboxRealRect = getCheckboxRect(rect);
            collectIconRect = getCollectionIconRect(rect);
        }

        itemData = qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(modelIndex));

        if (collectIconRect.contains(clickPoint)) {
            if (itemData.collectIconStatus != IconHover) {
                itemData.collectIconStatus = IconHover;
                m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
            }
            m_hoverModelIndex = modelIndex;
        } else if (itemData.collectIconStatus != IconNormal) {
            itemData.collectIconStatus = IconNormal;
            m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
        }
    }
    qDebug() << __FUNCTION__ << " end\n\n";
}

void DFontPreviewListView::mouseDoubleClickEvent(QMouseEvent *event)
{
    QListView::mouseDoubleClickEvent(event);
}

void DFontPreviewListView::setSelection(const QRect &rect,
                                        QItemSelectionModel::SelectionFlags command)
{
    QPoint clickPoint(rect.x(), rect.y());
    QModelIndex modelIndex = indexAt(clickPoint);
    m_currModelIndex = modelIndex;

    QListView::setSelection(rect, command);
}

void DFontPreviewListView::setModel(QAbstractItemModel *model)
{
//    m_fontPreviewItemModel = qobject_cast<QStandardItemModel *>(model);
    QListView::setModel(model);
}

void DFontPreviewListView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    if (selectionModel()->selectedIndexes().count() > 0) {
        //bug 37050 ut000442 当start与end位置相同时表示这一行是在我的收藏和已激活这个界面分别进行取消搜藏和禁用操作
        //时，listview一行被移出，这时候需要进行下面的操作，要不然界面会向下滚动。其他操作不需要这个操作，会导致bug现象。
        if (start == end) {
            selectionModel()->setCurrentIndex(parent, QItemSelectionModel::NoUpdate);
        }
    }

    QListView::rowsAboutToBeRemoved(parent, start, end);
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
            sortModelIndexList(list);
            //判断当前shift+上下键选中
            if (QApplication::keyboardModifiers() == Qt::ShiftModifier && list.count() > 0) {
                if (event->key() == Qt::Key_Up) {
                    keyPressEventFilter(list, true, false, true);
                    return;
                } else {
                    keyPressEventFilter(list, false, true, true);
                    return;
                }
            }
            if (QApplication::keyboardModifiers() == Qt::CTRL
                    && (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)
                    && list.count() > 0)
                return;
            //判断当前选中item是否为首个或末尾，首个按上键且在可见时切换至末尾选中，末尾按下键且可见时切换至首个选中 UT000539
            if (event->key() == Qt::Key_Up) {
                keyPressEventFilter(list, true, false, false);
            } else if (event->key() == Qt::Key_Down) {
                keyPressEventFilter(list, false, true, false);
            }
        }
        QListView::keyPressEvent(event);
    }
}
/*
 * param:list             :The selected QModelIndexList
 * param:isUp             :Key_Up pressed
 * param:isDown           :Key_Down pressed
 * param:isShiftModifier  :Key_Shift pressed
 * 根据按键设置选中
*/
void DFontPreviewListView::keyPressEventFilter(const QModelIndexList &list, bool isUp, bool isDown, bool isShiftModifier)
{
    //SP3--空白页面上下键选中判断
    if (list.count() < 1) {
        QModelIndex idx;
        if (isUp)
            idx = m_fontPreviewProxyModel->index(this->count() - 1, 0);
        else if (isDown) {
            idx = m_fontPreviewProxyModel->index(0, 0);
        }
        setCurrentIndex(idx);
        setCurrentSelected(idx.row());
        return;
    }
    //上键
    if (isUp) {
        //shift
        if (isShiftModifier) {
            //相反方向如果有选中，则清空并选中
            for (auto idx : list) {
                if (idx.row() > m_currentSelectedRow) {
                    clearSelection();
                    setCurrentIndex(m_fontPreviewProxyModel->index(m_currentSelectedRow, 0));
                    scrollTo(m_fontPreviewProxyModel->index(m_currentSelectedRow, 0));
                    return;
                }
            }
            if (list.last().row() > 0) {
                QModelIndex nextModelIndex = m_fontPreviewProxyModel->index(list.last().row() - 1, 0);
                selectionModel()->select(nextModelIndex, QItemSelectionModel::Select);
                scrollTo(nextModelIndex);
            }
            return;
        }
        if (isAtListviewTop()) {
            if (list.first().row() == 0) {
                QModelIndex modelIndex = m_fontPreviewProxyModel->index(this->count() - 1, 0);
                setCurrentIndex(modelIndex);
                return;
            }
        } else {
            if (list.first().row() == 0) {
                scrollToTop();
                return;
            }
        }
    }
    //下键
    else if (isDown) {
        //shift
        if (isShiftModifier) {
            //相反方向如果有选中，则清空并选中
            for (auto idx : list) {
                if (idx.row() < m_currentSelectedRow) {
                    clearSelection();
                    setCurrentIndex(m_fontPreviewProxyModel->index(m_currentSelectedRow, 0));
                    scrollTo(m_fontPreviewProxyModel->index(m_currentSelectedRow, 0));
                    return;
                }
            }
            if (list.first().row() < this->count()) {
                QModelIndex nextModelIndex = m_fontPreviewProxyModel->index(list.first().row() + 1, 0);
                selectionModel()->select(nextModelIndex, QItemSelectionModel::Select);
                scrollTo(nextModelIndex);
            }
            return;
        }
        if (isAtListviewBottom()) {
            if (list.last().row() == this->count() - 1) {
                QModelIndex modelIndex = m_fontPreviewProxyModel->index(0, 0);
                setCurrentIndex(modelIndex);
                return;
            }
        } else {
            if (list.last().row() == this->count() - 1) {
                scrollToBottom();
                return;
            }
        }
    }
}

bool DFontPreviewListView::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj)

    if (event->type() == QEvent::FocusOut) {
        m_isMouseClicked = false;
        m_IsTabFocus = false;
    }

    if (event->type() == QEvent::FocusIn) {

//        if (!m_isMouseClicked) {
//            m_IsTabFocus = true;
//            qDebug() << "ASD" << endl;
//        }
    }

    return false;
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

void DFontPreviewListView::updateShiftSelect(const QModelIndex &modelIndex)
{
    if (!modelIndex.isValid())
        return;

    int begin = m_currentSelectedRow;
    int end = m_currentSelectedRow;

    if (-1 != m_currentSelectedRow) {
        if (m_currentSelectedRow < modelIndex.row()) {
            end = modelIndex.row();
        } else if (m_currentSelectedRow > modelIndex.row()) {
            begin = modelIndex.row();
        }
    } else {
        begin = 0;
        end = modelIndex.row();
    }

    if (begin < 0)
        return;

    selectionModel()->clear();
    for (int i = begin; i <= end; i++) {
        QModelIndex modelIndex1 = m_fontPreviewProxyModel->index(i, 0);
        selectionModel()->select(modelIndex1, QItemSelectionModel::Select);
    }
}

bool DFontPreviewListView::isAtListviewBottom()
{
    if (this->verticalScrollBar()->value() >= this->verticalScrollBar()->maximum()) {
        return true;
    }
    return false;
}

bool DFontPreviewListView::isAtListviewTop()
{
    if (this->verticalScrollBar()->value() <= this->verticalScrollBar()->minimum()) {
        return true;
    }
    return false;
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

/*SP3--Alt+M右键菜单--弹出*/
void DFontPreviewListView::onRightMenuShortCutActivated()
{
    if (selectedIndexes().count() == 0) {
        return;
    }
    DFontMgrMainWindow *mw = qobject_cast<DFontMgrMainWindow *>(m_parentWidget);
    QModelIndexList indexes = selectedIndexes();
    QModelIndex temp;
    bool flag;
    //排序选中项
    for (int i = 0; i < indexes.count() - 1; i++) {
        flag = false;
        for (int j = indexes.count() - 1; j > i; j--) {
            if (indexes[j].row() < indexes[j - 1].row()) {
                temp = indexes[j];
                indexes[j] = indexes[j - 1];
                indexes[j - 1] = temp;
                flag = true;
            }
        }
        if (!flag)
            break;
    }
    //菜单弹出位置
    QPoint showMenuPosition;
    //记录鼠标位置下的QModelIndex
    QModelIndex n_currentIdx = indexAt(mapFromGlobal(QCursor::pos()));
    if (selectedIndexes().count() == 1) {
        QRect curRect = visualRect(selectedIndexes().first());
        //判断选中是否可见
        if (!viewport()->visibleRegion().contains(curRect.center())) {
            scrollTo(selectedIndexes().first());
            curRect = visualRect(selectedIndexes().first());
        }
        showMenuPosition = curRect.center();
        showMenuPosition = QPoint(showMenuPosition.x() + mw->pos().x() + POSITION_PARAM_X, showMenuPosition.y() + mw->pos().y() + POSITION_PARAM_Y);
    } else if (selectedIndexes().count() > 1) {
        bool n_needScroll = true;
        for (auto idx : indexes) {
            QRect curRect = visualRect(idx);
            //判断选中是否可见,在第一个可见位置弹出
            if (viewport()->visibleRegion().contains(curRect.center())) {
                setCurrentSelected(idx.row());
                showMenuPosition = QPoint((curRect.center()).x() + mw->pos().x() + POSITION_PARAM_X, (curRect.center()).y() + mw->pos().y() + POSITION_PARAM_Y);
                n_needScroll = false;
                break;
            }
        }
        if (n_needScroll) {
            scrollTo(indexes.first());
            setCurrentSelected(indexes.first().row());
            QPoint n_firstPosCenter = (visualRect(indexes.first())).center();
            showMenuPosition = QPoint(n_firstPosCenter.x() + mw->pos().x() + POSITION_PARAM_X, n_firstPosCenter.y() + mw->pos().y() + POSITION_PARAM_Y);
        }
    }
    //恢复normal状态
    DFontPreviewItemData itemData = qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(n_currentIdx));
    if (itemData.collectIconStatus != IconNormal) {
        itemData.collectIconStatus =  IconNormal;
        m_fontPreviewProxyModel->setData(n_currentIdx, QVariant::fromValue(itemData), Qt::DisplayRole);
    }
    if (!m_rightMenu->isVisible()) {
        m_IsNeedFocus = true;
        m_rightMenu->exec(showMenuPosition);
        return;
    }
}

void DFontPreviewListView::setCurrentSelected(int indexRow)
{
    m_currentSelectedRow = indexRow;
}

/*检查鼠标是否处于hover状态*/
void DFontPreviewListView::checkHoverState()
{
    //记录鼠标位置下的QModelIndex
    QModelIndex n_currentIdx = indexAt(mapFromGlobal(QCursor::pos()));
    QRect collectIconRect = getCollectionIconRect(visualRect(n_currentIdx));
    DFontPreviewItemData itemData = qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(n_currentIdx));
    if (collectIconRect.contains(mapFromGlobal(QCursor::pos()))) {
        if (itemData.collectIconStatus != IconHover) {
            itemData.collectIconStatus = IconHover;
            m_fontPreviewProxyModel->setData(n_currentIdx, QVariant::fromValue(itemData), Qt::DisplayRole);
        }
        m_hoverModelIndex = n_currentIdx;
    }
}

void DFontPreviewListView::onEnableBtnClicked(const QModelIndexList &itemIndexes, int systemCnt,
                                              int curCnt, bool setValue, bool isFromActiveFont)
{
    if (itemIndexes.isEmpty())
        return;

    qDebug() << __FUNCTION__ << " before " << currModelIndex().row() << currentIndex().row();
    bool needShowTips = false;
    int count = 0;
    QMutexLocker locker(&m_mutex);
    QString fontName;
    QModelIndexList itemIndexesNew = itemIndexes;
    if (isFromActiveFont)
        sortModelIndexList(itemIndexesNew);

    //记录禁用前选中位置
    m_bListviewAtButtom = isAtListviewBottom();
    m_bListviewAtTop = isAtListviewTop();
    //list为选中项，与itemIndexes有区别
    QModelIndexList list = selectionModel()->selectedIndexes();
    sortModelIndexList(list);
    qDebug() << list.count();
    int pos = list.last().row();
    m_selectAfterDel = pos;
    list.clear();

    QList<DFontPreviewItemData> modelist = m_dataThread->getFontModelList();

    for (QModelIndex &index : itemIndexesNew) {
        DFontPreviewItemData itemData =
            qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(index));
        QFileInfo fi(itemData.fontInfo.filePath);
        if (!fi.exists())
            continue;

        int idx = modelist.indexOf(itemData);
        if (setValue) {
            enableFont(itemData.fontInfo.filePath);
            itemData.isEnabled = setValue;
        } else {
            if (systemCnt > 0 || curCnt > 0) {
                needShowTips = true;
            }

            if (index == itemIndexes[0]) {
                fontName = itemData.strFontName;
            }
            disableFont(itemData.fontInfo.filePath);
            itemData.isEnabled = setValue;
            count++;
        }
        //更新状态
        m_dataThread->updateItemStatus(idx, itemData);
        DFMDBManager::instance()->updateFontInfo(itemData, "isEnabled");

        m_fontPreviewProxyModel->setData(index, QVariant::fromValue(itemData), Qt::DisplayRole);
    }

    DFMDBManager::instance()->commitUpdateFontInfo();

    if (setValue) {
        enableFonts();
        qDebug() << __FUNCTION__ << " after " << currModelIndex().row() << currentIndex().row();
        return;
    } else {
        disableFonts();
    }

    if (isFromActiveFont == true) {
        //设置移除后的选中
        selectItemAfterRemoved(m_bListviewAtButtom, m_bListviewAtTop);
    }

    QString message;
    if (needShowTips) {
        //不可禁用字体
        if (curCnt == 1 && systemCnt == 0) {
            message = DApplication::translate("MessageManager", "%1 is in use, so you cannot disable it").arg(m_curFontData.strFontName);
        } else if (curCnt > 0 && systemCnt > 0) {
            message = DApplication::translate("MessageManager", "You cannot disable system fonts and the fonts in use");
        } else {
            message = DApplication::translate("MessageManager", "You cannot disable system fonts");
        }
    } else {
        if (itemIndexes.size() == 1) {
            message = QString("%1 %2").arg(fontName).arg(DApplication::translate("MessageManager", "deactivated"));
        } else if (itemIndexes.size() > 1) {
            message = DApplication::translate("MessageManager", "The fonts have been deactivated");
        }
        Q_EMIT rowCountChanged();
    }
    //禁用字体大于零
    if (count > 0)
        DMessageManager::instance()->sendMessage(this->m_parentWidget, QIcon("://ok.svg"), message);

    qDebug() << __FUNCTION__ << " after " << currModelIndex().row() << currentIndex().row();
}

void DFontPreviewListView::onCollectBtnClicked(const QModelIndexList &index, bool setValue, bool isFromCollectFont)
{
    if (index.isEmpty())
        return;

    qDebug() << __FUNCTION__ << " before " << currModelIndex().row() << currentIndex().row();

    QMutexLocker locker(&m_mutex);
    QModelIndexList itemIndexesNew = index;
    sortModelIndexList(itemIndexesNew);
    if (itemIndexesNew.count() > 0) {
        m_selectAfterDel = itemIndexesNew.last().row();
    }

    m_bListviewAtButtom = isAtListviewBottom();
    m_bListviewAtTop = isAtListviewTop();

    QList<DFontPreviewItemData> modelist = m_dataThread->getFontModelList();
    for (QModelIndex &index : itemIndexesNew) {
        DFontPreviewItemData itemData =
            qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(index));
        itemData.isCollected = setValue;
        DFMDBManager::instance()->updateFontInfo(itemData, "isCollected");
        int idx = modelist.indexOf(itemData);
        //更新状态
        m_dataThread->updateItemStatus(idx, itemData);
        m_fontPreviewProxyModel->setData(index, QVariant::fromValue(itemData), Qt::DisplayRole);
    }
    if (isFromCollectFont == true) {
        //ut000442 bug 22350.通过快捷键访问时，有可能选中字体为空，添加相关判断，为空直接返回
        if (itemIndexesNew.count() == 0) {
            return;
        }
        //设置收藏页面移除后的选中
        selectItemAfterRemoved(m_bListviewAtButtom, m_bListviewAtTop);
    }

    DFMDBManager::instance()->commitUpdateFontInfo();

    Q_EMIT rowCountChanged();

    qDebug() << __FUNCTION__ << " after " << currModelIndex().row() << currentIndex().row();
}

void DFontPreviewListView::onListViewShowContextMenu(const QModelIndex &index)
{
    Q_UNUSED(index)

    QAction *action = m_rightMenu->exec(QCursor::pos());
    qDebug() << __FUNCTION__ << action;
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
    if (itemData.collectIconStatus != IconNormal) {
        qDebug() << " restore hover item " << itemData.strFontName;
        itemData.collectIconStatus = IconNormal;
        m_fontPreviewProxyModel->setData(m_hoverModelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
    }
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
    //qDebug() << __FUNCTION__ << path << " begin ";
    Q_UNUSED(path);
    QMutexLocker locker(&m_mutex);
    QList<DFontPreviewItemData> fontInfoList = m_dataThread->getFontModelList();
    //qDebug() << __FUNCTION__ << fontInfoList.size();
    for (int i = 0; i < fontInfoList.size(); ++i) {
        DFontPreviewItemData itemData = fontInfoList.at(i);
        QFileInfo filePathInfo(itemData.fontInfo.filePath);
        //如果字体文件已经不存在，则从t_manager表中删除
        if (!filePathInfo.exists()) {
            //删除字体之前启用字体，防止下次重新安装后就被禁用
            enableFont(itemData.fontInfo.filePath);
            DFMDBManager::instance()->deleteFontInfo(itemData);
            Q_EMIT itemRemovedFromSys(itemData);
            m_dataThread->removeFontData(itemData);
            m_dataThread->removePathWatcher(filePathInfo.filePath());
        }
    }
    DFMDBManager::instance()->commitDeleteFontInfo();
    enableFonts();

    Q_EMIT rowCountChanged();
    //qDebug() << __FUNCTION__ << "update end";
}

void DFontPreviewListView::deleteFontFiles(const QStringList &files, bool force)
{
    if (force) {
        for (const QString &path : files) {
            bool del = QFile::remove(path);
            qDebug() << __FUNCTION__ << " force delete file " << path << del;
        }
    }

    deleteCurFonts(files, force);
}

void DFontPreviewListView::deleteCurFonts(const QStringList &files, bool force)
{
    qDebug() << __FUNCTION__ << " before delete " << m_dataThread->getFontModelList().size() << m_fontPreviewProxyModel->rowCount()  << m_fontPreviewProxyModel->sourceModel()->rowCount();
    QList<DFontPreviewItemData> fontInfoList = m_dataThread->getFontModelList();
    qDebug() << fontInfoList.size() << __FUNCTION__ << files.size();
    int delCnt = 0;
    int total = fontInfoList.size();
    if (!force)
        updateSpinner(DFontSpinnerWidget::Delete, false);
    for (int i = 0; i < total; ++i) {
        DFontPreviewItemData itemData = fontInfoList.at(i);
        QString filePath = itemData.fontInfo.filePath;
        QFileInfo filePathInfo(filePath);
        //如果字体文件已经不存在，则从t_manager表中删除
        if (files.contains(filePath)) {
            //删除字体之前启用字体，防止下次重新安装后就被禁用
            enableFont(itemData.fontInfo.filePath);
            DFMDBManager::instance()->deleteFontInfo(itemData);
            QFontDatabase::removeApplicationFont(itemData.appFontId);
            delCnt++;
            m_dataThread->removeFontData(itemData);
            m_dataThread->removePathWatcher(filePath);
        }
    }
    DFMDBManager::instance()->commitDeleteFontInfo();

    enableFonts();

    Q_EMIT requestUpdateModel(!force);
    m_dataThread->onAutoDirWatchers();
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
            m_dataThread->removeFontData(itemData);
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

/**
 * @brief DFontPreviewListView::selectedFonts
 * @param deleteCnt : count of deletable fonts (minus system fonts and current font)
 * @param systemCnt : count of system font
 * @param curFontCnt : count of current font (0/1)
 * @param disableCnt : count of can be disabled/enabled fonts
 * @param delFontList : delete font path list (for delete fonts)
 * @param allIndexList : all font index list (includes system fonts and current font) (for collect fonts)
 * @param disableIndexList : can be disabled/enabled font index list (for enable/disable fonts)
 * @param allMinusSysFontList : all font path list exclude system fonts (for export fonts)
 */
void DFontPreviewListView::selectedFonts(int *deleteCnt, int *systemCnt, int *curFontCnt, int *disableCnt,
                                         QStringList *delFontList, QModelIndexList *allIndexList,
                                         QModelIndexList *disableIndexList, QStringList *allMinusSysFontList)
{
    QModelIndexList list = selectedIndexes();

    bool firstEnabled = false;
    bool calDisable = ((disableIndexList != nullptr) || (disableCnt != nullptr));
    int i = 0;
    for (QModelIndex &index : list) {
        QVariant varModel = m_fontPreviewProxyModel->data(index, Qt::DisplayRole);
        DFontPreviewItemData itemData = varModel.value<DFontPreviewItemData>();
        if (itemData.fontInfo.filePath.isEmpty())
            continue;

        if ((i == 0) && calDisable)
            firstEnabled = itemData.isEnabled;

        if (itemData.fontInfo.isSystemFont) {
            //处理启用禁用
            if (calDisable) {
                //禁用状态
                if (!firstEnabled) {
                    //系统字体可以启用
                    if (firstEnabled == itemData.isEnabled) {
                        if (disableCnt)
                            *disableCnt += 1;
                        if (disableIndexList)
                            *disableIndexList << index;
                    }
                    //启用状态, 系统字体不可禁用
                } else {
                    if (systemCnt != nullptr)
                        *systemCnt += 1;
                }
            } else {
                if (systemCnt != nullptr)
                    *systemCnt += 1;
            }
        } else if (itemData == m_curFontData) {
            qDebug() << __FUNCTION__ << " current font " << itemData.strFontName;
            if (curFontCnt)
                *curFontCnt += 1;
            appendFilePath(allMinusSysFontList, itemData.fontInfo.filePath);
        } else {
            if (deleteCnt)
                *deleteCnt += 1;

            appendFilePath(delFontList, itemData.fontInfo.filePath);
            appendFilePath(allMinusSysFontList, itemData.fontInfo.filePath);

            if (firstEnabled == itemData.isEnabled) {
                if (disableCnt)
                    *disableCnt += 1;
                if (disableIndexList)
                    *disableIndexList << index;
            }
        }

        i++;
    }

    if (allIndexList)
        *allIndexList = list;

    if (delFontList)
        qDebug() << __FUNCTION__ << delFontList->size();
}

/*SP3--切换至listview，已有选中且不可见，则滚动到第一并记录位置*/
void DFontPreviewListView::scrollWithTheSelected()
{
    if (0 == selectedIndexes().count()) {
        QModelIndex modelIndex = getFontPreviewProxyModel()->index(0, 0);
        if (modelIndex.isValid()) {
            setCurrentIndex(modelIndex);
            setCurrentSelected(0);
        }
        return;
    }
    QModelIndexList indexes = selectionModel()->selectedIndexes();
    sortModelIndexList(indexes);
    bool needScroll = true;
    if (indexes.isEmpty())
        return;
    for (auto idx : indexes) {
        QRect rect = visualRect(idx);
        if (this->viewport()->visibleRegion().contains(rect.center())) {
            needScroll = false;
            break;
        }
    }
    if (needScroll) {
        scrollTo(indexes.last());
        setCurrentSelected(indexes.last().row());
        indexes.clear();
    }
}

/*记录下当前选中的位置,用于局中显示UT000539*/
void DFontPreviewListView::refreshRect()
{
    QModelIndexList indexes = selectionModel()->selectedIndexes();
    if (indexes.isEmpty())
        return;
    sortModelIndexList(indexes);
    m_curRect = visualRect(indexes.last());
}

void DFontPreviewListView::updateSelection()
{
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
    }
}

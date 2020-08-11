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
    , m_fontChangeTimer(new QTimer(this))
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

    connect(m_fontChangeTimer, &QTimer::timeout, this, &DFontPreviewListView::onUpdateCurrentFont);

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

/*************************************************************************
 <Function>      initFontListData
 <Description>   初始化时,设置界面各个控件的状态
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::initFontListData()
{
    emit onLoadFontsStatus(0);
}

/*************************************************************************
 <Function>      isListDataLoadFinished
 <Description>   获取加载数据是否完成的标志位
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        m_bLoadDataFinish            Description:加载数据是否完成的标志位
 <Note>          null
*************************************************************************/
bool DFontPreviewListView::isListDataLoadFinished()
{
    return m_bLoadDataFinish;
}

/*************************************************************************
 <Function>      onFinishedDataLoad
 <Description>   数据加载完成响应函数
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::onFinishedDataLoad()
{
    qDebug() << "onFinishedDataLoad thread id = " << QThread::currentThreadId();
    m_bLoadDataFinish = true;
}


//void DFontPreviewListView::onItemAdded(const DFontPreviewItemData &itemData)
//{
//    QStandardItemModel *sourceModel = qobject_cast<QStandardItemModel *>(m_fontPreviewProxyModel->sourceModel());
//    QStandardItem *item = new QStandardItem;
//    item->setData(QVariant::fromValue(itemData), Qt::DisplayRole);
//    sourceModel->appendRow(item);
//}

/*************************************************************************
 <Function>      onMultiItemsAdded
 <Description>   listview中添加项响应函数
 <Author>        null
 <Input>
    <param1>     data              Description:需要添加项的数据列表
    <param2>     styles            Description:加载动画的样式
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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

    qDebug() << __FUNCTION__ << "rows = " << m_fontPreviewItemModel->rowCount();
    for (DFontPreviewItemData &itemData : data) {
        if (itemData.appFontId < 0) {
            int appFontId = QFontDatabase::addApplicationFont(itemData.fontInfo.filePath);
            itemData.appFontId = appFontId;
            m_dataThread->updateFontId(itemData, appFontId);
        }

        FontData fdata = itemData.fontData;
        if (fdata.strFontName.isEmpty()) {
            qDebug() << __FUNCTION__ << " empty strfontname";
        }

        QModelIndex index = m_fontPreviewItemModel->index(rows + i,   0);
        res = m_fontPreviewItemModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
        if (!res)
            qDebug() << __FUNCTION__ << "setData fail";
        i++;
        // repaint spinner
        if (styles != DFontSpinnerWidget::NoLabel)
            updateSpinner(styles);
    }

    if (styles == DFontSpinnerWidget::StartupLoad)
        Q_EMIT onLoadFontsStatus(1);

    qDebug() << __FUNCTION__ << "end";
}

/*************************************************************************
 <Function>      onItemRemoved
 <Description>   移除某一项响应函数
 <Author>        null
 <Input>
    <param1>     itemData            Description:需要移除项的数据
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::onItemRemoved(const DFontPreviewItemData &itemData)
{
    if (m_fontPreviewProxyModel == nullptr)
        return;

    QFontDatabase::removeApplicationFont(itemData.appFontId);
    deleteFontModelIndex(itemData.fontInfo.filePath);

    updateSelection();
}

/*************************************************************************
 <Function>      onItemRemovedFromSys
 <Description>   从文管中删除字体响应函数
 <Author>        null
 <Input>
    <param1>     itemData            Description:需要移除项的数据
 <Return>        null                Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::onItemRemovedFromSys(const DFontPreviewItemData &itemData)
{
    if (m_fontPreviewProxyModel == nullptr)
        return;

    qDebug() << __FUNCTION__ << ", path " << itemData.fontInfo.filePath << QThread::currentThreadId();
    QFontDatabase::removeApplicationFont(itemData.appFontId);
    deleteFontModelIndex(itemData.fontInfo.filePath, true);

    updateSelection();
}

/*************************************************************************
 <Function>      updateCurrentFontGroup
 <Description>   切换界面时,更新之前记录的当前字体组的信息
 <Author>        null
 <Input>
    <param1>     null            Description:null
    <param2>     null            Description:null
    <param3>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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

/*************************************************************************
 <Function>      initDelegate
 <Description>   初始化listview的代理
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::initDelegate()
{
    m_fontPreviewItemDelegate = new DFontPreviewItemDelegate(this);
    this->setItemDelegate(m_fontPreviewItemDelegate);

    m_fontPreviewProxyModel = new DFontPreviewProxyModel();
    m_fontPreviewProxyModel->setSourceModel(m_fontPreviewItemModel);
    m_fontPreviewProxyModel->setDynamicSortFilter(true);

    QListView::setModel(m_fontPreviewProxyModel);
}

/*************************************************************************
 <Function>      initConnections
 <Description>   初始化链接
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::initConnections()
{
    connect(m_signalManager, &SignalManager::currentFontGroup, this, &DFontPreviewListView::updateCurrentFontGroup);
}

/*************************************************************************
 <Function>      count
 <Description>   获取当前listview的行数
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        int             Description:当前listview的行数
 <Note>          null
*************************************************************************/
int DFontPreviewListView::count() const
{
    return model()->rowCount(rootIndex());
}

/*************************************************************************
 <Function>      cancelDel
 <Description>   取消删除后,重置之前记录的删除后的位置
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::cancelDel()
{
    m_selectAfterDel = -1;
}

/*************************************************************************
 <Function>      viewChanged
 <Description>   切换界面后,滚动到最上方
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::viewChanged()
{
    scrollToTop();
    m_currentSelectedRow = -1;
}

/*记录移除前位置*/
/*************************************************************************
 <Function>      markPositionBeforeRemoved
 <Description>   记录移除前位置
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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

/*************************************************************************
 <Function>      refreshFocuses
 <Description>   设置focus状态、设置选中状态
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::refreshFocuses()
{
    qDebug() << __FUNCTION__;
    if (isVisible())
        setFocus(Qt::MouseFocusReason);
}

/*************************************************************************
 <Function>      getOnePageCount
 <Description>   获取一页中列表项的个数
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        int             Description:列表项的个数
 <Note>          null
*************************************************************************/
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

/*************************************************************************
 <Function>      setRecoveryTabFocusState
 <Description>   记录操作前的tabfocus状态,用于进行操作后还原
 <Author>        null
 <Input>
    <param1>     recoveryTabFocusState            Description:操作前的tabfocus状态
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::setRecoveryTabFocusState(bool recoveryTabFocusState)
{
    m_recoveryTabFocusState = recoveryTabFocusState;
}

/*************************************************************************
 <Function>      setIsTabFocus
 <Description>   设置是否为tabfocus的标志位
 <Author>        null
 <Input>
    <param1>     IsTabFocus            Description:是否为tabfocus
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::setIsTabFocus(bool IsTabFocus)
{
    m_IsTabFocus = IsTabFocus;
}

/*************************************************************************
 <Function>      getIsTabFocus
 <Description>   获取是否为tab focus
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        bool            Description:是否为tabfocus的标志位
 <Note>          null
*************************************************************************/
bool DFontPreviewListView::getIsTabFocus() const
{
    return m_IsTabFocus;
}

/*************************************************************************
 <Function>      updateSpinner
 <Description>   更新加载动画spinner
 <Author>        null
 <Input>
    <param1>     style            Description:spinner的种类
    <param2>     force            Description:是否强制刷新
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::updateSpinner(DFontSpinnerWidget::SpinnerStyles style, bool force)
{
    qint64 curtm = QDateTime::currentMSecsSinceEpoch();
    //超过500ms刷新
    if (curtm - m_curTm >= 350) {
        Q_EMIT requestShowSpinner(true, force, style);
        m_curTm = QDateTime::currentMSecsSinceEpoch();
    }
}

/*************************************************************************
 <Function>      updateModel
 <Description>   删除字体后更新整个model
 <Author>        null
 <Input>
    <param1>     showSpinner            Description:显示加载图标的样式
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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
    setIsTabFocus(m_recoveryTabFocusState);
    m_recoveryTabFocusState = false;
}

/*************************************************************************
 <Function>      getCollectionIconRect
 <Description>   获取收藏图标尺寸
 <Author>        null
 <Input>
    <param1>     rect             Description:listview中一项的范围
 <Return>        QRect            Description:收藏图标尺寸
 <Note>          null
*************************************************************************/
QRect DFontPreviewListView::getCollectionIconRect(const QRect &rect)
{
    int collectIconSize = 22 + 10;
    return QRect(rect.right() - 10 - 33, rect.top() + 10 - 5, collectIconSize, collectIconSize);
}

/*************************************************************************
 <Function>      getCheckboxRect
 <Description>   获取复选框尺寸
 <Author>        null
 <Input>
    <param1>     rect            Description:listview中一项的范围
 <Return>        QRect            Description:复选框尺寸
 <Note>          null
*************************************************************************/
QRect DFontPreviewListView::getCheckboxRect(const QRect &rect)
{
    int checkBoxSize = 20 + 10;
    return QRect(rect.left() + 20, rect.top() + 10 - 5, checkBoxSize, checkBoxSize);
}

/*************************************************************************
 <Function>      onUpdateCurrentFont
 <Description>   更新当前字体
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::onUpdateCurrentFont()
{
    if (!m_fontChanged) {
        m_fontChangeTimer->stop();
        return;
    }

    QStringList curFont = DFontInfoManager::instance()->getCurrentFontFamily();
    qDebug() << __FUNCTION__ << "begin " << curFont << m_fontChanged;

    if (curFont.isEmpty() || curFont.size() < 3) {
        qDebug() << __FUNCTION__ << curFont << " is invalid";
        m_fontChanged = false;
        m_fontChangeTimer->stop();
        return;
    }

    if ((m_currentFont.size() == curFont.size()) && (m_currentFont.at(1) == curFont.at(1)) && (m_currentFont.at(2) == curFont.at(2))) {
        qDebug() << __FUNCTION__ << " will check 2 times : " << m_tryCnt;
        m_tryCnt++;
        if (m_tryCnt > 2) {
            m_tryCnt = 0;
            m_fontChanged = false;
            m_fontChangeTimer->stop();
        }
        return;
    }

    DFontPreviewItemData prevFontData = m_curFontData;
    for (DFontPreviewItemData &itemData : m_dataThread->getFontModelList()) {
        if (QFileInfo(itemData.fontInfo.filePath).fileName() != curFont.at(0))
            continue;

        if (curFont.at(2) != itemData.fontInfo.styleName)
            continue;

        bool found = false;
        if (itemData.fontInfo.familyName == curFont.at(1)) {
            found = true;
        } else {
            QStringList families = DFontInfoManager::instance()->getFontFamilyStyle(itemData.fontInfo.filePath);
            if (families.contains(curFont.at(1)))
                found = true;
        }

        if (found) {
            m_currentFont = curFont;
            m_curFontData = itemData;
            qDebug() << __FUNCTION__ << " found " << curFont << itemData.fontData.strFontName;
            m_fontChanged = false;
            m_fontChangeTimer->stop();
            break;
        }
    }
    qDebug() << __FUNCTION__ << "end" << m_currentFont;
}

/*************************************************************************
 <Function>      onFontChanged
 <Description>   应用字体变化时触发函数
 <Author>        null
 <Input>
    <param1>     font            Description:当前使用字体
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::onFontChanged(const QFont &font)
{
    if (font.weight() == m_curAppFont.weight() && font.style() == m_curAppFont.style()
            && font.stretch() == m_curAppFont.stretch() && font.styleHint() == m_curAppFont.styleHint()
            && font.styleStrategy() == m_curAppFont.styleStrategy() && font.fixedPitch() == m_curAppFont.fixedPitch()
            && font.family() == m_curAppFont.family() && font.styleName() == m_curAppFont.styleName()
            && font.hintingPreference() == m_curAppFont.hintingPreference()) {
        qDebug() << __FUNCTION__ << "same fontChanged , ignore it " << font.family();
        return;
    }

    m_fontChangeTimer->stop();
    qDebug() << __FUNCTION__ << "Font changed " << m_currentFont << font.family() << font.style() << m_currentFont;
    m_fontChanged = true;
    onUpdateCurrentFont();
    if (m_fontChanged) {
        m_fontChangeTimer->setInterval(300);
        m_fontChangeTimer->start();
    }
    m_curAppFont = font;
    qDebug() << __FUNCTION__ << " end";
}

/*************************************************************************
 <Function>      isCurrentFont
 <Description>   判断这个字体是否为当前系统使用字体
 <Author>        null
 <Input>
    <param1>     itemData            Description:需要判断的字体
 <Return>        bool                Description:是否为当前系统使用字体
 <Note>          null
*************************************************************************/
bool DFontPreviewListView::isCurrentFont(DFontPreviewItemData &itemData)
{
    return (itemData == m_curFontData);
}

/*************************************************************************
 <Function>      sortModelIndexList
 <Description>   对选中字体的索引按照row从大到小进行排序，为了在我的收藏界面和已激活界面进行操作时
 <Author>        ut000442
 <Input>
    <param1>     sourceList            Description:需要排序的字体列表
 <Return>        null                  Description:null
 <Note>          null
*************************************************************************/
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

/*************************************************************************
 <Function>      selectItemAfterRemoved
 <Description>   设置item移除后的选中
 <Author>        null
 <Input>
    <param1>     isAtBottom            Description:是否在列表底部
    <param2>     isAtTop               Description:是否在列表顶部
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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

/*************************************************************************
 <Function>      deleteFontModelIndex
 <Description>   根据文件路径删除字体列表中项
 <Author>        null
 <Input>
    <param1>     filePath            Description:需要删除的字体路径
    <param2>     isFromSys           Description:是否为从文管中删除
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::deleteFontModelIndex(const QString &filePath, bool isFromSys)
{
    Q_UNUSED(isFromSys);
    qDebug() << __FUNCTION__ << "m_fontPreviewProxyModel rowcount = " << m_fontPreviewProxyModel->rowCount();
    if (m_fontPreviewItemModel && m_fontPreviewItemModel->rowCount() == 0) {
        return;
    }

    QStringList str;

    for (int i = 0; i < m_fontPreviewProxyModel->sourceModel()->rowCount(); i++) {
        QModelIndex modelIndex = m_fontPreviewProxyModel->sourceModel()->index(i, 0);
        QVariant varModel = m_fontPreviewProxyModel->sourceModel()->data(modelIndex, Qt::DisplayRole);

        FontData fdata = varModel.value<FontData>();
        DFontPreviewItemData itemData = m_dataThread->getFontData(fdata);
        if (itemData.fontInfo.filePath.isEmpty())
            continue;

        if (itemData.fontInfo.filePath == filePath) {
            m_fontPreviewProxyModel->sourceModel()->removeRow(i);
            m_dataThread->removeFontData(itemData);
            break;
        }
    }
}

/*************************************************************************
 <Function>      isDeleting
 <Description>   返回是否正在删除
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        bool            Description:是否在删除
 <Note>          null
*************************************************************************/
bool DFontPreviewListView::isDeleting()
{
    DFontMgrMainWindow *mw = qobject_cast<DFontMgrMainWindow *>(m_parentWidget);
    if (mw)
        return mw->isDeleting();
    return false;
}

/*************************************************************************
 <Function>      selectFonts
 <Description>   根据提供的路径选中哦个listview中的项
 <Author>        null
 <Input>
    <param1>     fileList            Description:需要选中字体的路径
 <Return>        null                Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::selectFonts(const QStringList &fileList)
{
    if (fileList.isEmpty())
        return;

    QItemSelection selection;
    qDebug() << __FUNCTION__ << " fileList size " << fileList.size() << ", row count " << getFontPreviewProxyModel()->rowCount();
    for (int i = 0; i < getFontPreviewProxyModel()->rowCount(); ++i) {
        QModelIndex index = getFontPreviewProxyModel()->index(i, 0);
        FontData fdata = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(index));
        DFontPreviewItemData itemData = m_dataThread->getFontData(fdata);
        if (itemData.fontInfo.filePath.isEmpty()) {
            qDebug() << __FUNCTION__ << fdata.strFontName;
            continue;
        }

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
    setIsTabFocus(m_recoveryTabFocusState);
    m_recoveryTabFocusState = false;
}

/*************************************************************************
 <Function>      getMutex
 <Description>   获取线程锁
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
QMutex *DFontPreviewListView::getMutex()
{
    return &m_mutex;
}

/*************************************************************************
 <Function>      selectionChanged
 <Description>   选中切换后触发函数
 <Author>        null
 <Input>
    <param1>     selected              Description:下个选中项
    <param2>     deselected            Description:取消选中项
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    QListView::selectionChanged(selected, deselected);
}

/*************************************************************************
 <Function>      mouseMoveEvent
 <Description>   鼠标移动事件
 <Author>        null
 <Input>
    <param1>     event            Description:鼠标事件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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

        FontData itemData = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(modelIndex));

        if (m_hoverModelIndex.row() != modelIndex.row())
            clearHoverState();

        if (collectIconRect.contains(clickPoint)) {

            if (m_isMousePressNow) {
                if (itemData.getHoverState() != IconPress) {
                    itemData.setHoverState(IconPress);
                    m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
                }
            } else {
                if (itemData.getHoverState() != IconHover) {
                    itemData.setHoverState(IconHover);
                    m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
                }
                m_hoverModelIndex = modelIndex;
            }
        } else {
            if (itemData.getHoverState() != IconNormal) {
                itemData.setHoverState(IconNormal);
                m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
            }
        }
    }

    QListView::mouseMoveEvent(event);
}

/*************************************************************************
 <Function>      mousePressEvent
 <Description>   鼠标点击事件
 <Author>        null
 <Input>
    <param1>     event            Description:鼠标事件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "\n" << __FUNCTION__ << event->type() << event->button();
    //检查当前是否有选中，恢复起始位
    checkIfHasSelection();
    QListView::mousePressEvent(event);
    //应该设置焦点，否则鼠标在其他区域release会导致缺失焦点。
    setFocus(Qt::MouseFocusReason);
    QPoint clickPoint = event->pos();
    QModelIndex modelIndex = indexAt(clickPoint);

    m_isMouseClicked = true;
    if ((event->button() == Qt::LeftButton) && modelIndex.isValid()) {
        m_isMousePressNow = true;
        if (QApplication::keyboardModifiers() == Qt::ShiftModifier) {
            //Shift多选
            updateShiftSelect(modelIndex);
        } else {
            //左键单击
            m_currentSelectedRow = modelIndex.row();
            m_curRect = visualRect(modelIndex);

            QRect collectIconRect = getCollectionIconRect(m_curRect);

            FontData fdata = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(modelIndex));
//            DFontPreviewItemData itemData = getFontData(fdata.strFontName);

            if (collectIconRect.contains(clickPoint)) {
                if (fdata.getHoverState() != IconPress) {
                    fdata.setHoverState(IconPress);
                    m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(fdata), Qt::DisplayRole);
                }
                m_pressModelIndex = modelIndex;
            } else if (fdata.getHoverState() != IconNormal) {
                fdata.setHoverState(IconNormal);
                m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(fdata), Qt::DisplayRole);
            }
        }
    } else if ((event->button() == Qt::RightButton)  && modelIndex.isValid()) {
        if (event->modifiers() == Qt::ShiftModifier) {
            clearSelection();
            if (m_currentSelectedRow <= modelIndex.row()) {
                for (auto it = m_currentSelectedRow; it <= modelIndex.row(); it++) {
                    selectionModel()->select(m_fontPreviewProxyModel->index(it, 0), QItemSelectionModel::Select);
                }
            } else if (m_currentSelectedRow > modelIndex.row()) {
                for (auto it = modelIndex.row(); it <= m_currentSelectedRow; it++) {
                    selectionModel()->select(m_fontPreviewProxyModel->index(it, 0), QItemSelectionModel::Select);
                }
            }
        }
        //右键单击
        if (!this->selectedIndexes().contains(modelIndex) && event->modifiers() != Qt::ShiftModifier) {
            this->setCurrentIndex(modelIndex);
        }
        //右键index设置为shift起始位置
        if (event->modifiers() != Qt::ShiftModifier)
            setCurrentSelected(modelIndex.row());
        //恢复normal状态
        FontData fdata = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(modelIndex));
//        DFontPreviewItemData itemData = getFontData(fdata.strFontName);
        if (fdata.getHoverState() != IconNormal) {
            fdata.setHoverState(IconNormal);
            m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(fdata), Qt::DisplayRole);
        }

        onListViewShowContextMenu();
        refreshFocuses();
    } else if (event->button() == Qt::MidButton && QApplication::keyboardModifiers() == Qt::NoModifier) {
        clearSelection();
        setCurrentIndex(modelIndex);
        setCurrentSelected(modelIndex.row());
    }
}

/*************************************************************************
 <Function>      mouseReleaseEvent
 <Description>   鼠标释放事件
 <Author>        null
 <Input>
    <param1>     event            Description:鼠标事件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << __FUNCTION__ << " begin";
    if (Qt::MidButton == event->button()) {
        return;
    }
    QListView::mouseReleaseEvent(event);

    QPoint clickPoint = event->pos();

    QModelIndex modelIndex = indexAt(clickPoint);

    QModelIndexList indexList;
    m_currModelIndex = modelIndex;

    FontData fdata = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(modelIndex));
    DFontPreviewItemData itemData = m_dataThread->getFontData(fdata);

    QRect rect = visualRect(modelIndex);
    QRect checkboxRealRect = getCheckboxRect(rect);
    QRect collectIconRect = getCollectionIconRect(rect);
    //539 排除右键点击效果
    if (event->button() == Qt::LeftButton) {
        m_isMousePressNow = false;
        if (checkboxRealRect.contains(clickPoint)) {
            //触发启用/禁用字体
            int sysFontCnt = (fdata.isEnabled() && itemData.fontInfo.isSystemFont) ? 1 : 0;
            int curFontCnt = (itemData == m_curFontData) ? 1 : 0;
            if (sysFontCnt == 0 && curFontCnt == 0)
                indexList << modelIndex;
            onEnableBtnClicked(indexList, sysFontCnt, curFontCnt, !itemData.fontData.isEnabled(), m_currentFontGroup == FontGroup::ActiveFont);
        } else if (collectIconRect.contains(clickPoint)) {
            //恢复normal状态
            if (fdata.getHoverState() != IconNormal) {
                fdata.setHoverState(IconNormal);
                m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(fdata), Qt::DisplayRole);
            }
            //触发收藏/取消收藏
            indexList << modelIndex;
            onCollectBtnClicked(indexList, !itemData.fontData.isCollected(), (m_currentFontGroup == FontGroup::CollectFont));
        }

        //我的收藏界面,index点击之后会涉及到index的变化,所以需要重新获取一遍
        if (m_currentFontGroup == FontGroup::CollectFont) {
            qDebug() << __FUNCTION__ << " collect font grp";
            modelIndex = currModelIndex();
            rect = visualRect(modelIndex);
            checkboxRealRect = getCheckboxRect(rect);
            collectIconRect = getCollectionIconRect(rect);
        }

        fdata = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(modelIndex));

        if (collectIconRect.contains(clickPoint)) {
            if (fdata.getHoverState() != IconHover) {
                fdata.setHoverState(IconHover);
                m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(fdata), Qt::DisplayRole);
            }
            m_hoverModelIndex = modelIndex;
        } else if (fdata.getHoverState() != IconNormal) {
            fdata.setHoverState(IconNormal);
            m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(fdata), Qt::DisplayRole);
        }
    }
    qDebug() << __FUNCTION__ << " end\n\n";
}

/*************************************************************************
 <Function>      mouseDoubleClickEvent
 <Description>   鼠标双击事件
 <Author>        null
 <Input>
    <param1>     event            Description:鼠标事件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::mouseDoubleClickEvent(QMouseEvent *event)
{
    QListView::mouseDoubleClickEvent(event);
}

/*************************************************************************
 <Function>      ifHasSelection
 <Description>   检查当前是否无选中，恢复起始位
 <Author>        UT000539
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::checkIfHasSelection()
{
    if (selectedIndexes().count() == 0)
        m_currentSelectedRow = -1;
}

/*************************************************************************
 <Function>      setSelection
 <Description>   listview设置选中的函数,主要在这里获取当前选中的字体index
 <Author>        null
 <Input>
    <param1>     rect            Description:需要选中的区域范围
    <param2>     command            Description:SelectionFlag
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::setSelection(const QRect &rect,
                                        QItemSelectionModel::SelectionFlags command)
{
    QPoint clickPoint(rect.x(), rect.y());
    QModelIndex modelIndex = indexAt(clickPoint);
    m_currModelIndex = modelIndex;

    QListView::setSelection(rect, command);
}

/*************************************************************************
 <Function>      setModel
 <Description>   为listview设置模型
 <Author>        null
 <Input>
    <param1>     model            Description:需要设置的模型
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::setModel(QAbstractItemModel *model)
{
//    m_fontPreviewItemModel = qobject_cast<QStandardItemModel *>(model);
    QListView::setModel(model);
}

/*************************************************************************
 <Function>      rowsAboutToBeRemoved
 <Description>   listview一行将要移出时触发函数
 <Author>        null
 <Input>
    <param1>     parent            Description:对应被删除的夫索引
    <param2>     start            Description:第一个眼删除的行号
    <param3>     end            Description:最后一个要删除的行号
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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

/*************************************************************************
 <Function>      keyPressEvent
 <Description>   键盘点击事件
 <Author>        null
 <Input>
    <param1>     event            Description:键盘点击事件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::keyPressEvent(QKeyEvent *event)
{
    //检查当前是否有选中，恢复起始位
    checkIfHasSelection();
    if (event->key() == Qt::Key_End) {
        if (event->modifiers() == Qt::NoModifier) {
            setCurrentIndex(m_fontPreviewProxyModel->index(count() - 1, 0));
            setCurrentSelected(count() - 1);
        } else if (event->modifiers() == Qt::ShiftModifier) {
            clearSelection();
            for (int i = m_currentSelectedRow; i < count(); i++) {
                selectionModel()->select(m_fontPreviewProxyModel->index(i, 0), QItemSelectionModel::Select);
            }
            if (-1 == m_currentSelectedRow)
                setCurrentSelected(0);
        }
        scrollToBottom();
    } else if (event->key() == Qt::Key_Home) {
        if (event->modifiers() == Qt::NoModifier || selectedIndexes().count() == 0) {
            setCurrentIndex(m_fontPreviewProxyModel->index(0, 0));
            setCurrentSelected(0);
        } else if (event->modifiers() == Qt::ShiftModifier) {
            clearSelection();
            for (int i = m_currentSelectedRow; i >= 0; i--) {
                selectionModel()->select(m_fontPreviewProxyModel->index(i, 0), QItemSelectionModel::Select);
            }
        }
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
            } else if (QApplication::keyboardModifiers() == Qt::CTRL && list.count() > 0) {
                return;
            }
            //判断当前选中item是否为首个或末尾，首个按上键且在可见时切换至末尾选中，末尾按下键且可见时切换至首个选中 UT000539
            if (event->key() == Qt::Key_Up) {
                keyPressEventFilter(list, true, false, false);
                return;
            } else if (event->key() == Qt::Key_Down) {
                keyPressEventFilter(list, false, true, false);
                return;
            }
        }
        QListView::keyPressEvent(event);
    }
}

/*************************************************************************
 <Function>      keyPressEventFilter
 <Description>   根据按键设置选中
 <Author>        null
 <Input>
    <param1>     list              Description:The selected QModelIndexList
    <param2>     isUp              Description:Key_Up pressed
    <param3>     isDown            Description:Key_Down pressed
    <param4>     isShiftModifier   Description:Key_Shift pressed
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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
        //相反方向如果有选中，则清空并选中
        for (auto idx : list) {
            if (idx.row() > m_currentSelectedRow) {
                clearSelection();
                setCurrentIndex(m_fontPreviewProxyModel->index(m_currentSelectedRow, 0));
                scrollTo(m_fontPreviewProxyModel->index(m_currentSelectedRow, 0));
                return;
            }
        }
        //shift
        if (isShiftModifier) {
            if (list.last().row() > 0) {
                QModelIndex nextModelIndex = m_fontPreviewProxyModel->index(list.last().row() - 1, 0);
                selectionModel()->select(nextModelIndex, QItemSelectionModel::Select);
                scrollTo(nextModelIndex);
            }
        } else if (list.last().row() == 0 && list.count() == 1) {
            if (isAtListviewTop()) {
                m_currentSelectedRow = count() - 1;
                setCurrentIndex(m_fontPreviewProxyModel->index(count() - 1, 0));
            } else {
                scrollToTop();
            }
        } else {
            setCurrentIndex(m_fontPreviewProxyModel->index(m_currentSelectedRow - 1, 0));
            m_currentSelectedRow --;
        }
    }
    //下键
    else if (isDown) {
        //相反方向如果有选中，则清空并选中
        for (auto idx : list) {
            if (idx.row() < m_currentSelectedRow) {
                clearSelection();
                setCurrentIndex(m_fontPreviewProxyModel->index(m_currentSelectedRow, 0));
                scrollTo(m_fontPreviewProxyModel->index(m_currentSelectedRow, 0));
                return;
            }
        }
        //shift
        if (isShiftModifier) {
            if (list.first().row() < this->count()) {
                QModelIndex nextModelIndex = m_fontPreviewProxyModel->index(list.first().row() + 1, 0);
                selectionModel()->select(nextModelIndex, QItemSelectionModel::Select);
                scrollTo(nextModelIndex);
            }
        } else if (list.last().row() == this->count() - 1 && list.count() == 1) {
            if (isAtListviewBottom()) {
                QModelIndex modelIndex = m_fontPreviewProxyModel->index(0, 0);
                m_currentSelectedRow = 0;
                setCurrentIndex(modelIndex);
            } else {
                scrollToBottom();
            }
        } else {
            QModelIndex modelIndex = m_fontPreviewProxyModel->index(m_currentSelectedRow + 1, 0);
            setCurrentIndex(modelIndex);
            m_currentSelectedRow ++;
        }
    }
}

/*************************************************************************
 <Function>      eventFilter
 <Description>   事件过滤器,用于处理焦点移动事件
 <Author>        null
 <Input>
    <param1>     obj              Description:UNUSED
    <param2>     event            Description:触发的事件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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

/*************************************************************************
 <Function>      enableFont
 <Description>   启用字体
 <Author>        null
 <Input>
    <param1>     filePath            Description:启用字体路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::enableFont(const QString &filePath)
{
    if (!m_enableFontList.contains(filePath))
        m_enableFontList << filePath;
}

/*************************************************************************
 <Function>      disableFont
 <Description>   禁用字体
 <Author>        null
 <Input>
    <param1>     filePath            Description:禁用字体路径
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::disableFont(const QString &filePath)
{
    if (!m_disableFontList.contains(filePath))
        m_disableFontList << filePath;
}

/*************************************************************************
 <Function>      enableFonts
 <Description>   批量启用字体
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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

/*************************************************************************
 <Function>      enableFonts
 <Description>   批量禁用字体
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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

/*************************************************************************
 <Function>      updateShiftSelect
 <Description>   更新shift选中的字体.
 <Author>        null
 <Input>
    <param1>     modelIndex            Description:需要更新的index
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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
        selectionModel()->select(m_fontPreviewProxyModel->index(i, 0), QItemSelectionModel::Select);
    }
}

/*************************************************************************
 <Function>      isAtListviewBottom
 <Description>   判断listview是否在底部
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        bool            Description:listview是否在底部
 <Note>          null
*************************************************************************/
bool DFontPreviewListView::isAtListviewBottom()
{
    if (this->verticalScrollBar()->value() >= this->verticalScrollBar()->maximum()) {
        return true;
    }
    return false;
}

/*************************************************************************
 <Function>      isAtListviewTop
 <Description>   判断listview是否在顶部
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        bool            Description:listview是否在顶部
 <Note>          null
*************************************************************************/
bool DFontPreviewListView::isAtListviewTop()
{
    if (this->verticalScrollBar()->value() <= this->verticalScrollBar()->minimum()) {
        return true;
    }
    return false;
}

/*************************************************************************
 <Function>      onRightMenuShortCutActivated
 <Description>   Alt+M右键菜单--弹出
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
/*SP3--Alt+M右键菜单--弹出*/
void DFontPreviewListView::onRightMenuShortCutActivated()
{
    m_isMousePressNow = false;
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
    FontData itemData = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(n_currentIdx));
    if (itemData.getHoverState() != IconNormal) {
        itemData.setHoverState(IconNormal);
        m_fontPreviewProxyModel->setData(n_currentIdx, QVariant::fromValue(itemData), Qt::DisplayRole);
    }
    if (!m_rightMenu->isVisible()) {
        m_IsNeedFocus = true;
        m_rightMenu->exec(showMenuPosition);
        return;
    }
}

/*************************************************************************
 <Function>      setCurrentSelected
 <Description>   记录当前选中行的行数
 <Author>        null
 <Input>
    <param1>     indexRow            Description:当前选中行的行数
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::setCurrentSelected(int indexRow)
{
    m_currentSelectedRow = indexRow;
}

/*************************************************************************
 <Function>      checkHoverState
 <Description>   检查鼠标是否处于hover状态
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::checkHoverState()
{
    //记录鼠标位置下的QModelIndex
    QModelIndex n_currentIdx = indexAt(mapFromGlobal(QCursor::pos()));
    QRect collectIconRect = getCollectionIconRect(visualRect(n_currentIdx));
    FontData itemData = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(n_currentIdx));
    if (collectIconRect.contains(mapFromGlobal(QCursor::pos()))) {
        if (itemData.getHoverState() != IconHover) {
            itemData.setHoverState(IconHover);
            m_fontPreviewProxyModel->setData(n_currentIdx, QVariant::fromValue(itemData), Qt::DisplayRole);
        }
        m_hoverModelIndex = n_currentIdx;
    }
}

/*************************************************************************
 <Function>      onEnableBtnClicked
 <Description>   listview中启用禁用响应函数
 <Author>        null
 <Input>
    <param1>     itemIndexes            Description:选中项的索引
    <param2>     systemCnt              Description:是否为系统字体
    <param3>     curCnt                 Description:是否为系统当前使用的字体
    <param4>     setValue               Description:需要设置的值
    <param5>     isFromActiveFont       Description:是否在已激活界面触发
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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
        FontData fdata = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(index));
        DFontPreviewItemData itemData = m_dataThread->getFontData(fdata);
        QFileInfo fi(itemData.fontInfo.filePath);
        if (!fi.exists())
            continue;

        int idx = modelist.indexOf(itemData);
        if (setValue) {
            enableFont(itemData.fontInfo.filePath);
        } else {
            if (systemCnt > 0 || curCnt > 0) {
                needShowTips = true;
            }

            if (index == itemIndexes[0]) {
                fontName = itemData.fontData.strFontName;
            }
            disableFont(itemData.fontInfo.filePath);
            count++;
        }
        itemData.fontData.setEnabled(setValue);
        fdata.setEnabled(setValue);
        //更新状态
        m_dataThread->updateItemStatus(idx, itemData);
        DFMDBManager::instance()->updateFontInfo(itemData, "isEnabled");

        m_fontPreviewProxyModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
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
            message = DApplication::translate("MessageManager", "%1 is in use, so you cannot disable it").arg(m_curFontData.fontData.strFontName);
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
    }
    //禁用字体大于零
    if (count > 0)
        DMessageManager::instance()->sendMessage(this->m_parentWidget, QIcon("://ok.svg"), message);

    qDebug() << __FUNCTION__ << " after " << currModelIndex().row() << currentIndex().row();
    Q_EMIT rowCountChanged();
}

/*************************************************************************
 <Function>      onCollectBtnClicked
 <Description>   listview收藏界面点击后触发函数
 <Author>        null
 <Input>
    <param1>     index               Description:选中项的indexlist
    <param2>     setValue            Description:需要设置的值
    <param3>     isFromCollectFont   Description:是否来自于我的收藏界面
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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
    } else {
        return;
    }

    m_bListviewAtButtom = isAtListviewBottom();
    m_bListviewAtTop = isAtListviewTop();

    QList<DFontPreviewItemData> modelist = m_dataThread->getFontModelList();
    for (QModelIndex &index : itemIndexesNew) {
        FontData fdata = qvariant_cast<FontData>(m_fontPreviewProxyModel->data(index));
        DFontPreviewItemData itemData = m_dataThread->getFontData(fdata);

        itemData.fontData.setCollected(setValue);
        fdata.setCollected(setValue);
        DFMDBManager::instance()->updateFontInfo(itemData, "isCollected");
        int idx = modelist.indexOf(itemData);
        //更新状态
        m_dataThread->updateItemStatus(idx, itemData);
        m_fontPreviewProxyModel->setData(index, QVariant::fromValue(fdata), Qt::DisplayRole);
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

/*************************************************************************
 <Function>      onListViewShowContextMenu
 <Description>   显示右键菜单
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::onListViewShowContextMenu()
{
    QAction *action = m_rightMenu->exec(QCursor::pos());
    m_isMousePressNow = false;
    qDebug() << __FUNCTION__ << action;
}

/*************************************************************************
 <Function>      setRightContextMenu
 <Description>   设置右键菜单
 <Author>        null
 <Input>
    <param1>     rightMenu       Description:需要设置的右键菜单
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::setRightContextMenu(QMenu *rightMenu)
{
    m_rightMenu = rightMenu;
}

/*************************************************************************
 <Function>      currModelIndex
 <Description>   获取当前项的Index
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        QModelIndex     Description:当前项的Index
 <Note>          null
*************************************************************************/
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

/*************************************************************************
 <Function>      currModelData
 <Description>   返回当前项的itemdata
 <Author>        null
 <Input>
    <param1>     null                            Description:null
 <Return>        DFontPreviewItemData            Description:当前项的itemdata
 <Note>          null
*************************************************************************/
DFontPreviewItemData DFontPreviewListView::currModelData()
{
    QVariant varModel = m_fontPreviewProxyModel->data(currModelIndex(), Qt::DisplayRole);
    FontData fdata = varModel.value<FontData>();
    DFontPreviewItemData itemData = m_dataThread->getFontData(fdata);

    return itemData;
}

/*************************************************************************
 <Function>      getFontPreviewProxyModel
 <Description>   获取listview的model;
 <Author>        null
 <Input>
    <param1>     null                         Description:null
 <Return>        DFontPreviewProxyModel*      Description:listview的model
 <Note>          null
*************************************************************************/
DFontPreviewProxyModel *DFontPreviewListView::getFontPreviewProxyModel()
{
    return m_fontPreviewProxyModel;
}

/*************************************************************************
 <Function>      clearPressState
 <Description>   清空选中状态
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::clearPressState()
{
    if (!m_pressModelIndex.isValid())
        return;

    FontData pressData =
        qvariant_cast<FontData>(m_fontPreviewProxyModel->data(m_pressModelIndex));
    if (pressData.getHoverState() == IconNormal)
        return;
    pressData.setHoverState(IconNormal);
    m_fontPreviewProxyModel->setData(m_pressModelIndex, QVariant::fromValue(pressData), Qt::DisplayRole);
    m_pressModelIndex = QModelIndex();
}

/*************************************************************************
 <Function>      clearHoverState
 <Description>   清空hover状态
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::clearHoverState()
{
    if (!m_hoverModelIndex.isValid())
        return;

    FontData itemData =
        qvariant_cast<FontData>(m_fontPreviewProxyModel->data(m_hoverModelIndex));
    if (itemData.getHoverState() != IconNormal) {
        qDebug() << " restore hover item " << itemData.strFontName;
        itemData.setHoverState(IconNormal);
        m_fontPreviewProxyModel->setData(m_hoverModelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
    }
    m_hoverModelIndex = QModelIndex();
}

/*************************************************************************
 <Function>      updateChangedFile
 <Description>   文件改动时触发的函数
 <Author>        null
 <Input>
    <param1>     path            Description:改动文件的路径

 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::updateChangedFile(const QString &path)
{
    qDebug() << __FUNCTION__ << path << " begin ";
    QMutexLocker locker(&m_mutex);
    changeFontFile(path);

    Q_EMIT rowCountChanged();
    qDebug() << __FUNCTION__ << path << " end ";
}

/*************************************************************************
 <Function>      updateChangedFile
 <Description>   目录改动时触发的函数
 <Author>        null
 <Input>
    <param1>     path            Description:改动目录的路径

 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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
//            m_dataThread->removeFontData(itemData);
            m_dataThread->removePathWatcher(filePathInfo.filePath());
        }
    }
    DFMDBManager::instance()->commitDeleteFontInfo();
    enableFonts();

    Q_EMIT rowCountChanged();
    //qDebug() << __FUNCTION__ << "update end";
}

/*************************************************************************
 <Function>      deleteFontFiles
 <Description>   删除字体
 <Author>        null
 <Input>
    <param1>     files            Description:需要删除的字体
    <param2>     force            Description:是否为强制删除
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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
    if (!force)
        updateSpinner(DFontSpinnerWidget::Delete, false);
    for (DFontPreviewItemData &itemData : fontInfoList) {
        QString filePath = itemData.fontInfo.filePath;
        //如果字体文件已经不存在，则从t_manager表中删除
        if (files.contains(filePath)) {
            //删除字体之前启用字体，防止下次重新安装后就被禁用
            enableFont(itemData.fontInfo.filePath);
            DFMDBManager::instance()->deleteFontInfo(itemData);
            QFontDatabase::removeApplicationFont(itemData.appFontId);
            m_dataThread->removeFontData(itemData);
            m_dataThread->removePathWatcher(filePath);
        }
    }
    DFMDBManager::instance()->commitDeleteFontInfo();

    enableFonts();

    Q_EMIT requestUpdateModel(!force);
    m_dataThread->onAutoDirWatchers();
}

/*************************************************************************
 <Function>      changeFontFile
 <Description>   文件改动时触发的函数,对相应文件进行处理
 <Author>        null
 <Input>
    <param1>     null            Description:null
    <param2>     null            Description:null
    <param3>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::changeFontFile(const QString &path, bool force)
{
    QFileInfo fi(path);
    bool isDir = fi.isDir();
    if (force) {
        bool del = QFile::remove(path);
        qDebug() << __FUNCTION__ << " force delete file " << path << del;
    }

    qDebug() << __FUNCTION__ << path;
    foreach (DFontPreviewItemData itemData, m_dataThread->getFontModelList()) {
        QString filePath = itemData.fontInfo.filePath;
        QFileInfo filePathInfo(filePath);
        //如果字体文件已经不存在，则从t_manager表中删除
        if ((!isDir && (filePath == path) && (!filePathInfo.exists())) || (isDir && filePath.startsWith(path) && !filePathInfo.exists())) {
            //删除字体之前启用字体，防止下次重新安装后就被禁用
            enableFont(itemData.fontInfo.filePath);
            DFMDBManager::instance()->deleteFontInfo(itemData);
            Q_EMIT itemRemoved(itemData);
//            m_dataThread->removeFontData(itemData);
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

/*************************************************************************
 <Function>      selectedFonts
 <Description>   选中字体,并通过传入参数获得各种字体的数目等信息.
 <Author>        null
 <Input>
    <param1> deleteCnt           Description: count of deletable fonts (minus system fonts and current font)
    <param1> systemCnt           Description: count of system font
    <param1> curFontCnt          Description: count of current font (0/1)
    <param1> disableCnt          Description: count of can be disabled/enabled fonts
    <param1> delFontList         Description: delete font path list (for delete fonts)
    <param1> allIndexList        Description: all font index list (includes system fonts and current font) (for collect fonts)
    <param1> disableIndexList    Description: can be disabled/enabled font index list (for enable/disable fonts)
    <param1> allMinusSysFontList Description: all font path list exclude system fonts (for export fonts)
    <param1> curData             Description: current index data (required for disable/enable/collect fonts)
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::selectedFonts(const DFontPreviewItemData &curData,
                                         int *deleteCnt, int *systemCnt, qint8 *curFontCnt, int *disableCnt,
                                         QStringList *delFontList, QModelIndexList *allIndexList,
                                         QModelIndexList *disableIndexList, QStringList *allMinusSysFontList)
{
    if (deleteCnt != nullptr)
        *deleteCnt = 0;
    if (systemCnt != nullptr)
        *systemCnt = 0;
    if (curFontCnt != nullptr)
        *curFontCnt = 0;
    if (disableCnt != nullptr)
        *disableCnt = 0;
    if (delFontList != nullptr)
        delFontList->clear();
    if (allIndexList != nullptr)
        allIndexList->clear();
    if (disableIndexList != nullptr)
        disableIndexList->clear();
    if (allMinusSysFontList != nullptr)
        allMinusSysFontList->clear();

    QModelIndexList list = selectedIndexes();

    bool curEnableCollect = false;
    bool calDisable = ((disableIndexList != nullptr) || (disableCnt != nullptr));
    if (calDisable) {
        curEnableCollect = curData.fontData.isEnabled();
    } else if (allIndexList != nullptr) {
        curEnableCollect = curData.fontData.isCollected();
    }

    for (QModelIndex &index : list) {
        QVariant varModel = m_fontPreviewProxyModel->data(index, Qt::DisplayRole);
        FontData fdata = varModel.value<FontData>();
        DFontPreviewItemData itemData = m_dataThread->getFontData(fdata);
        if (itemData.fontInfo.filePath.isEmpty())
            continue;

        if (itemData.fontInfo.isSystemFont) {
            //处理启用禁用
            if (calDisable) {
                if (!curEnableCollect) { //禁用状态
                    //系统字体可以启用
                    if (curEnableCollect == itemData.fontData.isEnabled()) {
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
                if ((allIndexList != nullptr) && (curEnableCollect == itemData.fontData.isCollected()))
                    *allIndexList << index;
            }
        } else if (itemData == m_curFontData) {
            qDebug() << __FUNCTION__ << " current font " << itemData.fontData.strFontName;
            if (curFontCnt)
                *curFontCnt += 1;
            appendFilePath(allMinusSysFontList, itemData.fontInfo.filePath);
            if ((allIndexList != nullptr) && (curEnableCollect == itemData.fontData.isCollected()))
                *allIndexList << index;
        } else {
            if (deleteCnt)
                *deleteCnt += 1;

            appendFilePath(delFontList, itemData.fontInfo.filePath);
            appendFilePath(allMinusSysFontList, itemData.fontInfo.filePath);

            if (calDisable && (curEnableCollect == itemData.fontData.isEnabled())) {
                if (disableCnt)
                    *disableCnt += 1;
                if (disableIndexList)
                    *disableIndexList << index;
            } else if ((allIndexList != nullptr) && (curEnableCollect == itemData.fontData.isCollected())) {
                *allIndexList << index;
            }
        }
    }

    if (delFontList && deleteCnt)
        qDebug() << __FUNCTION__ << delFontList->size() << *deleteCnt;
}

/*************************************************************************
 <Function>      scrollWithTheSelected
 <Description>   切换至listview，已有选中且不可见，则滚动到第一并记录位置
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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

/*************************************************************************
 <Function>      refreshRect
 <Description>   记录下当前选中的位置,用于局中显示
 <Author>        UT000539
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontPreviewListView::refreshRect()
{
    QModelIndexList indexes = selectionModel()->selectedIndexes();
    if (indexes.isEmpty())
        return;
    sortModelIndexList(indexes);
    m_curRect = visualRect(indexes.last());
}

/*************************************************************************
 <Function>      updateSelection
 <Description>   更新选中状态
 <Author>        null
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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

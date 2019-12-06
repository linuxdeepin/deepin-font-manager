#include "dfontpreviewlistview.h"
#include "dfontinfomanager.h"
#include "dfontpreviewitemdelegate.h"
#include "globaldef.h"
#include "dfmxmlwrapper.h"

#include <DLog>
#include <DMenu>
#include <DGuiApplicationHelper>
#include <DApplication>
#include <DMessageManager>

#include <QFontDatabase>
#include <QSet>

DWIDGET_USE_NAMESPACE

DFontPreviewListView::DFontPreviewListView(QWidget *parent)
    : DListView(parent)
    , m_bLoadDataFinish(false)
    , m_parentWidget(parent)
    , m_fontPreviewItemModel(new QStandardItemModel)
    , m_dataThread(DFontPreviewListDataThread::instance())
{
    QWidget *topSpaceWidget = new QWidget;
    topSpaceWidget->setFixedSize(this->width(), 10);
    this->addHeaderWidget(topSpaceWidget);

    setAutoScroll(true);
    setMouseTracking(true);
    setUpdatesEnabled(true);

    initFontListData();
}

DFontPreviewListView::~DFontPreviewListView()
{
}

void DFontPreviewListView::initFontListData()
{
    //qDebug() << "main thread id = " << QThread::currentThreadId();
    connect(m_dataThread, SIGNAL(resultReady()), this, SLOT(onFinishedDataLoad()));
}

int DFontPreviewListView::getListDataCount()
{
    return m_dataThread->getFontModelList().size();
}

bool DFontPreviewListView::isListDataLoadFinished()
{
    return m_bLoadDataFinish;
}

void DFontPreviewListView::refreshFontListData()
{
    m_dataThread->refreshFontListData();

    QList<DFontPreviewItemData> fontInfoList = m_dataThread->getFontModelList();
    qDebug() << "total count:" << fontInfoList.size();

    QList<DFontPreviewItemData> diffFontInfoList = m_dataThread->getDiffFontModelList();
    QStandardItemModel *sourceModel = qobject_cast<QStandardItemModel *>(m_fontPreviewProxyModel->sourceModel());
    for (int i = 0; i < diffFontInfoList.size(); ++i) {

        DFontPreviewItemData itemData = diffFontInfoList.at(i);
        QString filePath = itemData.fontInfo.filePath;
        QFileInfo filePathInfo(filePath);

        QStandardItem *item = new QStandardItem;
        item->setData(QVariant::fromValue(itemData), Qt::DisplayRole);
        sourceModel->appendRow(item);
    }
}

void DFontPreviewListView::onFinishedDataLoad()
{
    //qDebug() << "onFinishedDataLoad thread id = " << QThread::currentThreadId();
    QList<DFontPreviewItemData> fontInfoList = m_dataThread->getFontModelList();
    //qDebug() << fontInfoList.size();
    for (int i = 0; i < fontInfoList.size(); ++i) {

        DFontPreviewItemData itemData = fontInfoList.at(i);
        QString filePath = itemData.fontInfo.filePath;
        QFileInfo filePathInfo(filePath);
        //如果字体文件已经不存在，则从t_manager表中删除
        if (!filePathInfo.exists()) {
            //删除字体之前启用字体，防止下次重新安装后就被禁用
            enableFont(itemData);
            DFMDBManager::instance()->deleteFontInfo("fontId", itemData.strFontId);
            continue;
        }

        QStandardItem *item = new QStandardItem;
        item->setData(QVariant::fromValue(itemData), Qt::DisplayRole);
        m_fontPreviewItemModel->appendRow(item);
    }

    initDelegate();
    initConnections();

    m_bLoadDataFinish = true;
}

void DFontPreviewListView::initDelegate()
{
    m_fontPreviewItemDelegate = new DFontPreviewItemDelegate(this);
    this->setItemDelegate(m_fontPreviewItemDelegate);

    m_fontPreviewProxyModel = new DFontPreviewProxyModel(this);
    m_fontPreviewProxyModel->setSourceModel(m_fontPreviewItemModel);
    m_fontPreviewProxyModel->setDynamicSortFilter(true);
    this->setModel(m_fontPreviewProxyModel);
}

void DFontPreviewListView::initConnections()
{
    connect(this, &DFontPreviewListView::onClickEnableButton, this,
            &DFontPreviewListView::onListViewItemEnableBtnClicked);
    connect(this, &DFontPreviewListView::onClickCollectionButton, this,
            &DFontPreviewListView::onListViewItemCollectionBtnClicked);
    connect(this, &DFontPreviewListView::onShowContextMenu, this,
            &DFontPreviewListView::onListViewShowContextMenu, Qt::ConnectionType::QueuedConnection);

    QObject::connect(m_fontPreviewProxyModel,
                     SIGNAL(onFilterFinishRowCountChangedInt(unsigned int)),
                     m_parentWidget,
                     SLOT(onFontListViewRowCountChanged(unsigned int)));
}

QRect DFontPreviewListView::getCollectionIconRect(QRect visualRect)
{
    int collectIconSize = 22 + 10;
    return QRect(visualRect.right() - 10 - 33, visualRect.top() + 10 - 5, collectIconSize, collectIconSize);
}

void DFontPreviewListView::mouseMoveEvent(QMouseEvent *event)
{
    DListView::mouseMoveEvent(event);

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

    DFontPreviewItemData itemData =
        qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(modelIndex));

    if (collectIconRect.contains(clickPoint)) {
        itemData.collectIconStatus = IconHover;
    } else {
        itemData.collectIconStatus = IconNormal;
    }
    m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
}

void DFontPreviewListView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_bLeftMouse = true;
    } else {
        m_bLeftMouse = false;
    }

    DListView::mousePressEvent(event);

    if (m_fontPreviewItemModel && m_fontPreviewItemModel->rowCount() == 0) {
        return;
    }

    QPoint clickPoint = event->pos();

    QModelIndex modelIndex = indexAt(clickPoint);
    QRect rect = visualRect(modelIndex);

    QRect collectIconRect = getCollectionIconRect(rect);

    DFontPreviewItemData itemData =
        qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(modelIndex));

    if (collectIconRect.contains(clickPoint)) {
        itemData.collectIconStatus = IconPress;
    } else {
        itemData.collectIconStatus = IconNormal;
    }
    m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
}

void DFontPreviewListView::mouseReleaseEvent(QMouseEvent *event)
{
    DListView::mouseReleaseEvent(event);

    if (m_fontPreviewItemModel && m_fontPreviewItemModel->rowCount() == 0) {
        return;
    }

    QPoint clickPoint = event->pos();

    QModelIndex modelIndex = indexAt(clickPoint);
    m_currModelIndex = modelIndex;

    DFontPreviewItemData itemData =
        qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(modelIndex));

    itemData.collectIconStatus = IconNormal;
    m_fontPreviewProxyModel->setData(modelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);

    QRect rect = visualRect(modelIndex);

    int checkBoxSize = 20 + 10;
    QRect checkboxRealRect = QRect(rect.left() + 25, rect.top() + 10 - 5, checkBoxSize, checkBoxSize);
    QRect collectIconRect = getCollectionIconRect(rect);

    if (checkboxRealRect.contains(clickPoint)) {
        //触发启用/禁用字体
        emit onClickEnableButton(modelIndex);
    } else if (collectIconRect.contains(clickPoint)) {
        //触发收藏/取消收藏
        emit onClickCollectionButton(modelIndex);
    }
}

void DFontPreviewListView::setSelection(const QRect &rect,
                                        QItemSelectionModel::SelectionFlags command)
{
    DListView::setSelection(rect, command);

    QPoint clickPoint(rect.x(), rect.y());
    QModelIndex modelIndex = indexAt(clickPoint);
    m_currModelIndex = modelIndex;

    if (!m_bLeftMouse) {
        emit onShowContextMenu(modelIndex);
    }
}

void DFontPreviewListView::setModel(QAbstractItemModel *model)
{
    m_fontPreviewItemModel = qobject_cast<QStandardItemModel *>(model);
    DListView::setModel(model);
}

bool DFontPreviewListView::enableFont(DFontPreviewItemData itemData)
{
    QString fontConfigPath = DFMXmlWrapper::m_fontConfigFilePath;
    bool isCreateSuccess = DFMXmlWrapper::createFontConfigFile(fontConfigPath);

    if (!isCreateSuccess) {
        return false;
    }

    QStringList strFontPathList;
    DFMXmlWrapper::queryAllChildNodes_Text(fontConfigPath, "rejectfont", strFontPathList);

    QString fontFilePath = itemData.fontInfo.filePath;
    if (strFontPathList.contains(fontFilePath)) {
        return DFMXmlWrapper::deleteNodeWithText(fontConfigPath, "pattern", fontFilePath);
    }

    return false;
}

bool DFontPreviewListView::disableFont(DFontPreviewItemData itemData)
{
    QString fontConfigPath = DFMXmlWrapper::m_fontConfigFilePath;
    bool isCreateSuccess = DFMXmlWrapper::createFontConfigFile(fontConfigPath);

    if (!isCreateSuccess) {
        return false;
    }

    QStringList strDisableFontPathList = DFMXmlWrapper::getFontConfigDisableFontPathList();

    QString fontFilePath = itemData.fontInfo.filePath;
    if (!strDisableFontPathList.contains(fontFilePath)) {
        return DFMXmlWrapper::addPatternNodesWithText(fontConfigPath, "rejectfont", fontFilePath);
    }

    return false;
}

void DFontPreviewListView::onListViewItemEnableBtnClicked(QModelIndex index)
{
    DFontPreviewItemData itemData =
        qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(index));
    itemData.isEnabled = !itemData.isEnabled;

    qDebug() << "familyName" << itemData.fontInfo.familyName << endl;

    QString deactivatedMessage =
        QString("%1 %2").arg(itemData.strFontName).arg(DApplication::translate("MessageManager", "deactivated"));
    if (itemData.isEnabled) {
        enableFont(itemData);
    } else {
        disableFont(itemData);
        DMessageManager::instance()->sendMessage(this, QIcon(":/images/ok.svg"), deactivatedMessage);
    }

    DFMDBManager::instance()->updateFontInfoByFontId(itemData.strFontId, "isEnabled", QString::number(itemData.isEnabled));

    m_fontPreviewProxyModel->setData(index, QVariant::fromValue(itemData), Qt::DisplayRole);
}

void DFontPreviewListView::onListViewItemCollectionBtnClicked(QModelIndex index)
{
    DFontPreviewItemData itemData =
        qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(index));
    itemData.isCollected = !itemData.isCollected;

    DFMDBManager::instance()->updateFontInfoByFontId(itemData.strFontId, "isCollected", QString::number(itemData.isCollected));

    m_fontPreviewProxyModel->setData(index, QVariant::fromValue(itemData), Qt::DisplayRole);
}

void DFontPreviewListView::onListViewShowContextMenu(QModelIndex index)
{
    Q_UNUSED(index)

    DMenu *rightMenu = m_rightMenu;

    //在当前鼠标位置显示
    rightMenu->exec(QCursor::pos());
}

void DFontPreviewListView::setRightContextMenu(QMenu *rightMenu)
{
    m_rightMenu = rightMenu;
}

QModelIndex DFontPreviewListView::currModelIndex()
{
    return m_currModelIndex;
}

DFontPreviewItemData DFontPreviewListView::currModelData()
{
    QVariant varModel = m_fontPreviewProxyModel->data(m_currModelIndex, Qt::DisplayRole);
    DFontPreviewItemData itemData = varModel.value<DFontPreviewItemData>();

    return itemData;
}

DFontPreviewProxyModel *DFontPreviewListView::getFontPreviewProxyModel()
{
    return m_fontPreviewProxyModel;
}

void DFontPreviewListView::removeRowAtIndex(QModelIndex modelIndex)
{
    if (m_fontPreviewItemModel && m_fontPreviewItemModel->rowCount() == 0) {
        return;
    }

    QVariant varModel = m_fontPreviewProxyModel->data(modelIndex, Qt::DisplayRole);
    DFontPreviewItemData itemData = varModel.value<DFontPreviewItemData>();

    //删除字体之前启用字体，防止下次重新安装后就被禁用
    enableFont(itemData);
    QMap<QString, QString> deleteInfo;
    deleteInfo.insert("filePath", itemData.fontInfo.filePath);
    deleteInfo.insert("familyName", itemData.fontInfo.familyName);
    deleteInfo.insert("styleName", itemData.fontInfo.styleName);
    // DFMDBManager::instance()->deleteFontInfoByFontId(itemData.strFontId);
    DFMDBManager::instance()->deleteFontInfoByFontMap(deleteInfo);

    m_fontPreviewProxyModel->removeRow(modelIndex.row(), modelIndex.parent());

    m_dataThread->removeFontData(itemData);
}

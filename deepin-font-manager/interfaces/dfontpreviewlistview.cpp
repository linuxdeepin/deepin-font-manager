#include "dfontpreviewlistview.h"
#include "dfontinfomanager.h"
#include "dfontpreviewitemdelegate.h"
#include "globaldef.h"

#include <DLog>
#include <DMenu>

#include <QFontDatabase>
#include <QSet>

DWIDGET_USE_NAMESPACE

DFontPreviewListView::DFontPreviewListView(QWidget *parent)
    : DListView(parent)
    , m_dbManager(DFMDBManager::instance())
{
    setAutoScroll(false);

    initFontListData();
    initDelegate();
    initConnections();
}

DFontPreviewListView::~DFontPreviewListView()
{
}

void DFontPreviewListView::initFontListData()
{
    QStringList fontNameList;
    DFontInfoManager *fontInfoMgr = DFontInfoManager::instance();
    QStringList strAllFontList = fontInfoMgr->getAllFontPath();

    qDebug() << "strAllFontList.size()" << strAllFontList.size() << endl;

    m_fontPreviewItemModel = new QStandardItemModel;

    int recordCount = m_dbManager->getRecordCount();
    if (recordCount > 0) {

        refreshFontListData(m_fontPreviewItemModel, true);

        return;
    }

    //开启事务
    m_dbManager->beginTransaction();
    for (int i = 0; i < strAllFontList.size(); ++i) {
        QString filePath = strAllFontList.at(i);
        if (filePath.length() > 0) {
            insertFontItemData(m_fontPreviewItemModel, filePath, i + 1);
        }
    }

    m_dbManager->endTransaction();
}

void DFontPreviewListView::insertFontItemData(QStandardItemModel *sourceModel, QString filePath, int index)
{
    DFontInfoManager *fontInfoMgr = DFontInfoManager::instance();
    DFontPreviewItemData itemData;
    QFileInfo filePathInfo(filePath);
    itemData.pFontInfo = fontInfoMgr->getFontInfo(filePath);

    int appFontId = QFontDatabase::addApplicationFont(filePath);
    QStringList fontFamilyList = QFontDatabase::applicationFontFamilies(appFontId);
    if (fontFamilyList.size() > 0) {
        QString fontFamily = QString(fontFamilyList.first().toLocal8Bit());
        if (itemData.pFontInfo->styleName.length() > 0) {
            itemData.strFontName =
                QString("%1-%2").arg(fontFamily).arg(itemData.pFontInfo->styleName);
        } else {
            itemData.strFontName = fontFamily;
        }
        itemData.pFontInfo->familyName = fontFamily;
    } else {
        itemData.strFontName = itemData.pFontInfo->familyName;
    }
    itemData.strFontId = QString::number(index);
    itemData.appFontId = appFontId;
    itemData.strFontFileName = filePathInfo.baseName();
    itemData.strFontPreview = FTM_DEFAULT_PREVIEW_TEXT;
    itemData.iFontSize = FTM_DEFAULT_PREVIEW_FONTSIZE;
    itemData.isEnabled = true;
    itemData.isCollected = false;

    QStandardItem *item = new QStandardItem;
    item->setData(QVariant::fromValue(itemData), Qt::DisplayRole);

    sourceModel->appendRow(item);

    m_dbManager->addFontInfo(itemData);
}

void DFontPreviewListView::refreshFontListData(QStandardItemModel *sourceModel, bool isStartup)
{
    DFontInfoManager *fontInfoMgr = DFontInfoManager::instance();
    QStringList strAllFontList = fontInfoMgr->getAllFontPath();

    QList <DFontPreviewItemData> fontInfoList = m_dbManager->getAllFontInfo();

    //开启事务
    m_dbManager->beginTransaction();

    QSet<QString> dbFilePathSet;
    for (int i = 0; i < fontInfoList.size(); ++i) {

        DFontPreviewItemData itemData = fontInfoList.at(i);
        QString filePath = itemData.pFontInfo->filePath;
        QFileInfo filePathInfo(filePath);
        //如果字体文件已经不存在，则从t_manager表中删除
        if (!filePathInfo.exists()) {
            m_dbManager->deleteFontInfo("fontId", itemData.strFontId);
            continue;
        }

        QStandardItem *item = new QStandardItem;
        item->setData(QVariant::fromValue(itemData), Qt::DisplayRole);

        if (isStartup) {
            sourceModel->appendRow(item);
        }

        //只添加已启用的字体, 用于下面比对系统中是否有新增字体文件
        if (itemData.isEnabled) {
            dbFilePathSet.insert(filePath);
        }
    }

    //根据文件路径比较出不同的字体文件
    QSet<QString> fontListSet = strAllFontList.toSet();
    QSet<QString> diffSet = fontListSet.subtract(dbFilePathSet);
    qDebug() << "diffSet count:" << diffSet.count();
    if (diffSet.count() > 0) {
        QList<QString> diffFilePathList = diffSet.toList();
        for (int i = 0; i < diffFilePathList.size(); ++i) {
            QString filePath = diffFilePathList.at(i);
            if (filePath.length() > 0) {
                insertFontItemData(sourceModel, filePath, strAllFontList.size() + i + 1);
            }
        }
    }

    m_dbManager->endTransaction();
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
        qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(index));
    itemData.isEnabled = !itemData.isEnabled;

#warning todo...
    if (itemData.isEnabled) {

    } else {

    }

    m_dbManager->updateFontInfoByFontId(itemData.strFontId, "isEnabled", QString::number(itemData.isEnabled));

    m_fontPreviewProxyModel->setData(index, QVariant::fromValue(itemData), Qt::DisplayRole);
}

void DFontPreviewListView::onListViewItemCollectionBtnClicked(QModelIndex index)
{
    DFontPreviewItemData itemData =
        qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(index));
    itemData.isCollected = !itemData.isCollected;

    m_dbManager->updateFontInfoByFontId(itemData.strFontId, "isCollected", QString::number(itemData.isCollected));

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
    QVariant varModel = m_fontPreviewProxyModel->data(modelIndex, Qt::DisplayRole);
    DFontPreviewItemData itemData = varModel.value<DFontPreviewItemData>();

    m_dbManager->deleteFontInfoByFontId(itemData.strFontId);

    m_fontPreviewProxyModel->removeRow(modelIndex.row(), modelIndex.parent());
}

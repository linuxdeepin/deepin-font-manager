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
    : DListView(parent)
    , m_bLoadDataFinish(false)
    , m_parentWidget(parent)
    , m_fontPreviewItemModel(new QStandardItemModel)
    , m_dataThread(DFontPreviewListDataThread::instance(this))
{
    QWidget *topSpaceWidget = new QWidget;
    topSpaceWidget->setFixedSize(this->width(), 10);
    this->addHeaderWidget(topSpaceWidget);

    setAutoScroll(true);
    setMouseTracking(true);
    setUpdatesEnabled(true);
//    setSelectionMode(QAbstractItemView::ExtendedSelection);

    DFontMgrMainWindow *mw = qobject_cast<DFontMgrMainWindow *>(m_parentWidget);
    if (mw)
        connect(mw, &DFontMgrMainWindow::requestDeleted, this, [ = ](const QStringList files) {
//            deleteFontFiles(files);
        Q_EMIT requestDeleted(files);
    });
    initFontListData();
    connect(this, &DFontPreviewListView::itemAdded, this, &DFontPreviewListView::onItemAdded);
    connect(this, &DFontPreviewListView::itemRemoved, this, &DFontPreviewListView::onItemRemoved);
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
    for (int i = 0; i < diffFontInfoList.size(); ++i) {
        DFontPreviewItemData itemData = diffFontInfoList.at(i);
        QString filePath = itemData.fontInfo.filePath;
        Q_EMIT itemAdded(itemData);
        m_dataThread->addPathWatcher(filePath);
    }
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
            enableFont(itemData);
            QMap<QString, QString> delInfo;
            delInfo.insert("filePath", itemData.fontInfo.filePath);
            delInfo.insert("familyName", itemData.fontInfo.familyName);
            delInfo.insert("styleName", itemData.fontInfo.styleName);
            DFMDBManager::instance()->deleteFontInfoByFontMap(delInfo);
            continue;
        } else {
            //add it to file system watcher
            m_dataThread->addPathWatcher(filePath);
        }

        QStandardItem *item = new QStandardItem;
        item->setData(QVariant::fromValue(itemData), Qt::DisplayRole);
        m_fontPreviewItemModel->appendRow(item);
    }

    initDelegate();
    initConnections();

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

void DFontPreviewListView::onItemRemoved(const DFontPreviewItemData &itemData)
{
    if (m_fontPreviewProxyModel == nullptr)
        return;

    qDebug() << __FUNCTION__ << ", path " << itemData.fontInfo.filePath;

    for (int i = 0; i < m_fontPreviewProxyModel->rowCount(); i++) {
        QModelIndex modelIndex = m_fontPreviewProxyModel->index(i, 0);
        QVariant varModel = m_fontPreviewProxyModel->data(modelIndex, Qt::DisplayRole);
        DFontPreviewItemData item = varModel.value<DFontPreviewItemData>();
        //qDebug() << __FUNCTION__ << item.fontInfo.filePath << endl;
        //qDebug() << __FUNCTION__ << itemData.fontInfo.filePath << endl;
        if (item.fontInfo.filePath == itemData.fontInfo.filePath) {
            qDebug() << __FUNCTION__ << item.fontInfo.filePath << " font remove row " << i;
            m_fontPreviewProxyModel->removeRow(i, modelIndex.parent());
            return;
        }
    }
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
                     SLOT(onFontListViewRowCountChanged(unsigned int)), Qt::QueuedConnection);
}

QRect DFontPreviewListView::getCollectionIconRect(QRect visualRect)
{
    int collectIconSize = 22 + 10;
    return QRect(visualRect.right() - 10 - 33, visualRect.top() + 10 - 5, collectIconSize, collectIconSize);
}

void DFontPreviewListView::deleteFontModelIndex(const QString &filePath)
{
    if (m_fontPreviewItemModel && m_fontPreviewItemModel->rowCount() == 0) {
        return;
    }

    for (int i = 0; i < m_fontPreviewProxyModel->rowCount(); i++) {
        QModelIndex modelIndex = m_fontPreviewProxyModel->index(i, 0);
        QVariant varModel = m_fontPreviewProxyModel->data(modelIndex, Qt::DisplayRole);
        DFontPreviewItemData itemData = varModel.value<DFontPreviewItemData>();

        if (itemData.fontInfo.filePath == filePath) {
            qDebug() << __FUNCTION__ << filePath << " font remove row " << i;
            QModelIndex nextIndex = modelIndex.siblingAtRow(i + 1);
            if (!nextIndex.isValid())
                nextIndex = modelIndex.siblingAtRow(i - 1);
            setCurrentIndex(nextIndex);
            m_fontPreviewProxyModel->removeRow(i, modelIndex.parent());
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
    QModelIndexList indexes;
    QItemSelectionModel *selection_model = selectionModel();
    selection_model->reset();
    QItemSelection selection;

    DFMDBManager *dbManager = DFMDBManager::instance();
    QList<DFontPreviewItemData> allFontInfo = dbManager->getAllFontInfo();
    for (auto font : allFontInfo) {
//        qDebug() << "db " << __FUNCTION__ << font.fontInfo.familyName << font.fontInfo.styleName << font.fontInfo.filePath;
    }
    QStringList outlist;
    for (QString filePath : fileList) {
        DFontInfo fi = DFontInfoManager::instance()->getFontInfo(filePath);
        QStringList list;
        list << fi.familyName << fi.styleName;
//        qDebug() << __FUNCTION__ << filePath << list;

        if (list.isEmpty() || list.size() < 2) {
            qDebug() << __FUNCTION__ << "continue";
            continue;
        }

        bool found = false;
        for (const auto &famItem : allFontInfo) {
//            qDebug() <<  __FUNCTION__ << famItem.fontInfo.familyName << " , " << famItem.fontInfo.styleName;
            if (list[0] == famItem.fontInfo.familyName && list[1] == famItem.fontInfo.styleName) {
//                qDebug() << "FOUND" << list << famItem.fontInfo.filePath;
                outlist << famItem.fontInfo.filePath;
                found = true;
                break;
            }
        }
        if (!found)
            qDebug() << __FUNCTION__ << " not found " << list << filePath;
    }

    qDebug() << __FUNCTION__ << "files " << fileList << " installed " << outlist;
    if (outlist.size() == 1) {
        for (int i = 0; i < getFontPreviewProxyModel()->rowCount(); ++i) {
            QModelIndex index = getFontPreviewProxyModel()->index(i, 0);
            DFontPreviewItemData itemData =
                qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(index));

            if (outlist.contains(itemData.fontInfo.filePath)) {
                setCurrentIndex(index);
                break;
            }
        }
        return;
    }

    for (int i = 0; i < getFontPreviewProxyModel()->rowCount(); ++i) {
        QModelIndex index = getFontPreviewProxyModel()->index(i, 0);
        DFontPreviewItemData itemData =
            qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(index));

        if (outlist.contains(itemData.fontInfo.filePath)) {
            QModelIndex left = m_fontPreviewProxyModel->index(index.row(), 0);
            QModelIndex right = m_fontPreviewProxyModel->index(index.row(), m_fontPreviewProxyModel->columnCount() - 1);
            QItemSelection sel(left, right);
            selection.merge(sel, QItemSelectionModel::Select);
        }
    }
    qDebug() << " selection size " << selection.size();
    selection_model->select(selection, QItemSelectionModel::Select);
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

    DFontPreviewItemData itemData =
        qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(modelIndex));

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

    DListView::mouseMoveEvent(event);
}

void DFontPreviewListView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_bLeftMouse = true;
    } else {
        m_bLeftMouse = false;
    }

    if (m_fontPreviewItemModel && m_fontPreviewItemModel->rowCount() == 0) {
        return;
    }

    QPoint clickPoint = event->pos();

    QModelIndex modelIndex = indexAt(clickPoint);
    QRect rect = visualRect(modelIndex);

    QRect collectIconRect = getCollectionIconRect(rect);
    int checkBoxSize = 20 + 10;
    QRect checkboxRealRect = QRect(rect.left() + 25, rect.top() + 10 - 5, checkBoxSize, checkBoxSize);

    if (collectIconRect.contains(clickPoint) || checkboxRealRect.contains(clickPoint)) {
        m_bClickCollectionOrEnable = true;
    } else {
        m_bClickCollectionOrEnable = false;
    }

    DFontPreviewItemData itemData =
        qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(modelIndex));

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


    if (m_bClickCollectionOrEnable) {
        return;
    }
    DListView::mousePressEvent(event);
}

void DFontPreviewListView::mouseReleaseEvent(QMouseEvent *event)
{
    DListView::mouseReleaseEvent(event);

    if (m_fontPreviewItemModel && m_fontPreviewItemModel->rowCount() == 0) {
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

    if (checkboxRealRect.contains(clickPoint)) {
        //触发启用/禁用字体
//        emit onClickEnableButton(indexList, !itemData.isEnabled);
        onListViewItemEnableBtnClicked(indexList, !itemData.isEnabled);
    } else if (collectIconRect.contains(clickPoint)) {
        //触发收藏/取消收藏
//        emit onClickCollectionButton(modelIndex);
        onListViewItemCollectionBtnClicked(modelIndex);
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

    if (!m_bLeftMouse) {
        emit onShowContextMenu(modelIndex);
    }

    if (m_bClickCollectionOrEnable) {
        return;
    }
    DListView::setSelection(rect, command);
}

void DFontPreviewListView::setModel(QAbstractItemModel *model)
{
    m_fontPreviewItemModel = qobject_cast<QStandardItemModel *>(model);
    DListView::setModel(model);
}

void DFontPreviewListView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
}

bool DFontPreviewListView::enableFont(const DFontPreviewItemData &itemData)
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

bool DFontPreviewListView::disableFont(const DFontPreviewItemData &itemData)
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

void DFontPreviewListView::onListViewItemEnableBtnClicked(QModelIndexList itemIndexes, bool setValue)
{
    QString fontName;

    DFMDBManager::instance()->beginTransaction();   //开启事务
    for (QModelIndex index : itemIndexes) {
//        DFontPreviewItemData itemData =
//            qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(itemIndexes[0]));
        DFontPreviewItemData itemData =
            qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(index));
        itemData.isEnabled = setValue;

        qDebug() << __FUNCTION__ << "familyName" << itemData.fontInfo.familyName << endl;

        if (itemData.isEnabled) {
            enableFont(itemData);
        } else {
            if (index == itemIndexes[0])
                fontName = itemData.strFontName;
            disableFont(itemData);
        }

        DFMDBManager::instance()->updateFontInfoByFontId(itemData.strFontId, "isEnabled", QString::number(itemData.isEnabled));

        m_fontPreviewProxyModel->setData(index, QVariant::fromValue(itemData), Qt::DisplayRole);
//        m_fontPreviewProxyModel->setData(itemIndexes[0], QVariant::fromValue(itemData), Qt::DisplayRole);
    }
    DFMDBManager::instance()->endTransaction(); //提交事务

    if (setValue)
        return;

    QString message;
    if (itemIndexes.size() == 1) {
        message = QString("%1 %2").arg(fontName).arg(DApplication::translate("MessageManager", "deactivated"));
    } else if (itemIndexes.size() > 1) {
        //            message = tr("The fonts have been deactivated");
        message = DApplication::translate("MessageManager", "The fonts have been deactivated");
    }
    DMessageManager::instance()->sendMessage(this, QIcon(":/images/ok.svg"), message);
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
    if (!selectedIndexes().isEmpty() && m_currModelIndex != selectedIndexes().first())
        m_currModelIndex = selectedIndexes().first();

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
    qDebug() << " restore press item " << pressData.strFontName;
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
    qDebug() << " restore hover item " << itemData.strFontName;
    itemData.collectIconStatus = IconNormal;
    m_fontPreviewProxyModel->setData(m_hoverModelIndex, QVariant::fromValue(itemData), Qt::DisplayRole);
    m_hoverModelIndex = QModelIndex();
}

void DFontPreviewListView::updateChangedFile(const QString &path)
{
    deleteFontFile(path);
}

void DFontPreviewListView::updateChangedDir(const QString &path)
{
    //no different between "share" or "fonts" dir
    QList<DFontPreviewItemData> fontInfoList = m_dataThread->getFontModelList();
    qDebug() << fontInfoList.size();

    DFMDBManager::instance()->beginTransaction();
    for (int i = 0; i < fontInfoList.size(); ++i) {
        DFontPreviewItemData itemData = fontInfoList.at(i);
        QFileInfo filePathInfo(itemData.fontInfo.filePath);
        //如果字体文件已经不存在，则从t_manager表中删除
        if (!filePathInfo.exists()) {
            //删除字体之前启用字体，防止下次重新安装后就被禁用
            enableFont(itemData);
            QMap<QString, QString> delInfo;
            delInfo.insert("filePath", itemData.fontInfo.filePath);
            delInfo.insert("familyName", itemData.fontInfo.familyName);
            delInfo.insert("styleName", itemData.fontInfo.styleName);
            if (!DFMDBManager::instance()->deleteFontInfoByFontMap(delInfo))
                qDebug() << QThread::currentThreadId() << " delete fontdb failed : " << filePathInfo.filePath();
            Q_EMIT itemRemoved(itemData);
            m_dataThread->removeFontData(itemData);
            m_dataThread->removePathWatcher(filePathInfo.filePath());
        }
    }
    DFMDBManager::instance()->endTransaction();
}

void DFontPreviewListView::deleteFontFiles(const QStringList files)
{
    for (QString filePath : files) {
        deleteFontFile(filePath);
    }

//    qDebug() << "after delete font readd watched files " << m_watchFiles;
    m_dataThread->addWatchers(m_watchFiles);
    m_watchFiles.clear();
}

void DFontPreviewListView::deleteFontFile(const QString &path)
{
    QFileInfo fi(path);
    bool isDir = fi.isDir();
    QList<DFontPreviewItemData> fontInfoList = m_dataThread->getFontModelList();
    qDebug() << fontInfoList.size();
    for (int i = 0; i < fontInfoList.size(); ++i) {
        DFontPreviewItemData itemData = fontInfoList.at(i);
        QString filePath = itemData.fontInfo.filePath;
        QFileInfo filePathInfo(filePath);
        //如果字体文件已经不存在，则从t_manager表中删除
        if ((!isDir && (filePath == path)) || (isDir && filePath.startsWith(path) && !filePathInfo.exists())) {
            //删除字体之前启用字体，防止下次重新安装后就被禁用
            enableFont(itemData);
            QMap<QString, QString> delInfo;
            delInfo.insert("filePath", itemData.fontInfo.filePath);
            delInfo.insert("familyName", itemData.fontInfo.familyName);
            delInfo.insert("styleName", itemData.fontInfo.styleName);
            if (!DFMDBManager::instance()->deleteFontInfoByFontMap(delInfo))
                qDebug() << QThread::currentThreadId() << " delete fontdb failed : " << filePath;
            Q_EMIT itemRemoved(itemData);
            m_dataThread->removeFontData(itemData);
            m_dataThread->removePathWatcher(filePath);
            if (!isDir)
                break;
        }
    }


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
    for (QModelIndex index : list) {
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

    if (ret.isEmpty()) {
        DFontPreviewItemData itemData = currModelData();
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

    qDebug() << __FUNCTION__ << ret;
    return ret;
}

QList<DFontPreviewItemData> DFontPreviewListView::selectedFontData(int *deleteCnt, int *systemCnt)
{
    QModelIndexList list = selectedIndexes();
    QList<DFontPreviewItemData> ret;
    int deleteNum = 0;
    int systemNum = 0;
    for (QModelIndex index : list) {
        QVariant varModel = m_fontPreviewProxyModel->data(index, Qt::DisplayRole);
        DFontPreviewItemData itemData = varModel.value<DFontPreviewItemData>();
        if (!itemData.fontInfo.filePath.isEmpty()) {
            if (itemData.fontInfo.isSystemFont) {
                systemNum++;
            } else {
                deleteNum++;
                ret << itemData;
            }
        }
    }

    if (ret.isEmpty()) {
        DFontPreviewItemData itemData = currModelData();
        if (!itemData.fontInfo.filePath.isEmpty()) {
            if (itemData.fontInfo.isSystemFont) {
                systemNum++;
            } else {
                deleteNum++;
                ret << itemData;
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

QModelIndexList DFontPreviewListView::selectedIndex(int *deleteCnt, int *systemCnt)
{
    QModelIndexList list = selectedIndexes();
    int deleteNum = 0;
    int systemNum = 0;
    for (QModelIndex index : list) {
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

    if (deleteNum == 0 && systemNum == 0) {
        DFontPreviewItemData itemData = currModelData();
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

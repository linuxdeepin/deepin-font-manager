#include "dfontpreviewlistview.h"
#include <fontconfig/fontconfig.h>
#include "dfontinfomanager.h"
#include "dfontpreviewitemdelegate.h"

#include <DLog>
#include <DMenu>

#include <QFontDatabase>
#include <QSet>

DWIDGET_USE_NAMESPACE

DFontPreviewListView::DFontPreviewListView(QWidget *parent)
    : DListView(parent)
    , m_sqlUtil(new DSqliteUtil(".font_manager.db"))
{
    setAutoScroll(false);

    initFontListData();
    initDelegate();
    initConnections();
}

DFontPreviewListView::~DFontPreviewListView()
{
    delete m_sqlUtil;
}

QMap<QString, QString> DFontPreviewListView::mapItemData(DFontPreviewItemData itemData)
{
    QMap<QString, QString> mapData;
    mapData.insert("fontName", itemData.strFontName);
    mapData.insert("familyName", itemData.pFontInfo->familyName);
    mapData.insert("isEnabled", QString::number(itemData.isEnabled));
    mapData.insert("isCollected", QString::number(itemData.isCollected));
    mapData.insert("filePath", itemData.pFontInfo->filePath);
    mapData.insert("appFontId", QString::number(itemData.appFontId));

    return mapData;
}

void DFontPreviewListView::initFontListData()
{
    QStringList fontNameList;
    DFontInfoManager *fontInfoMgr = DFontInfoManager::instance();
    QStringList strAllFontList = fontInfoMgr->getAllFontPath();

    qDebug() << "strAllFontList.size()" << strAllFontList.size() << endl;

    int recordCount = m_sqlUtil->getRecordCount();
    if (recordCount > 0) {
        QList<QMap<QString, QString>> recordList;

        QList<QString> keyList;
        keyList.append("fontId");
        keyList.append("fontName");
        keyList.append("familyName");
        keyList.append("isEnabled");
        keyList.append("isCollected");
        keyList.append("filePath");
        keyList.append("appFontId");

        m_sqlUtil->findRecords(keyList, &recordList);

        QSet<QString> dbFilePathSet;
        m_fontPreviewItemModel = new QStandardItemModel;
        for (int i = 0; i < recordList.size(); ++i) {
            QMap<QString, QString> record = recordList.at(i);
            if (record.size() > 0) {
                DFontPreviewItemData itemData;
                QString filePath = record.value("filePath");
                QFileInfo filePathInfo(filePath);
                itemData.strFontId = record.value("fontId");
                itemData.pFontInfo = fontInfoMgr->getFontInfo(filePath);
                itemData.strFontName = record.value("fontName");
                itemData.pFontInfo->familyName = record.value("familyName");
                itemData.strFontFileName = filePathInfo.baseName();
                itemData.strFontPreview = FTM_DEFAULT_PREVIEW_TEXT;
                itemData.iFontSize = FTM_DEFAULT_PREVIEW_FONTSIZE;
                itemData.isEnabled = record.value("isEnabled").toInt();
                itemData.isCollected = record.value("isCollected").toInt();
                itemData.appFontId = record.value("appFontId").toInt();

                if (itemData.isCollected) {
                    qDebug() << itemData.strFontName << record.value("isCollected").toInt() << endl;
                }

                QStandardItem *item = new QStandardItem;
                item->setData(QVariant::fromValue(itemData), Qt::DisplayRole);

                m_fontPreviewItemModel->appendRow(item);

                dbFilePathSet.insert(filePath);
            }
        }

        //根据文件路径比较出不同的字体文件
        QSet<QString> fontListSet = strAllFontList.toSet();
        QSet<QString> diffSet = fontListSet.subtract(dbFilePathSet);
        if (diffSet.count() > 0) {
            QList<QString> diffFilePathList = diffSet.toList();
            for (int i = 0; i < diffFilePathList.size(); ++i) {
                QString filePath = diffFilePathList.at(i);
                if (filePath.length() > 0) {
                    insertFontItemData(filePath, strAllFontList.size() + i + 1);
                }
            }
        }

        return;
    }

    //开启事务
    m_sqlUtil->m_db.transaction();
    m_fontPreviewItemModel = new QStandardItemModel;
    for (int i = 0; i < strAllFontList.size(); ++i) {
        QString filePath = strAllFontList.at(i);
        if (filePath.length() > 0) {
            insertFontItemData(filePath, i + 1);
        }
    }

    m_sqlUtil->m_db.commit();
}

void DFontPreviewListView::insertFontItemData(QString filePath, int index)
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

    m_fontPreviewItemModel->appendRow(item);

    m_sqlUtil->addRecord(mapItemData(itemData));
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

    m_fontPreviewProxyModel->setData(index, QVariant::fromValue(itemData), Qt::DisplayRole);
}

void DFontPreviewListView::onListViewItemCollectionBtnClicked(QModelIndex index)
{
    DFontPreviewItemData itemData =
        qvariant_cast<DFontPreviewItemData>(m_fontPreviewProxyModel->data(index));
    itemData.isCollected = !itemData.isCollected;

    QMap<QString, QString> where;
    where.insert("fontId", itemData.strFontId);
    QMap<QString, QString> data;
    data.insert("isCollected", QString::number(itemData.isCollected));
    m_sqlUtil->updateRecord(where, data);

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

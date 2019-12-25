#ifndef DFONTPREVIEWLISTVIEW_H
#define DFONTPREVIEWLISTVIEW_H

#include "dfontpreviewitemdef.h"
#include "dfontpreviewitemdelegate.h"
#include "dfontpreviewlistdatathread.h"
#include "dfontpreviewproxymodel.h"
#include "dfmdbmanager.h"

#include <DListView>

#include <QMouseEvent>
#include <QSortFilterProxyModel>

DWIDGET_USE_NAMESPACE

class DFontPreviewListView : public DListView
{
    Q_OBJECT

public:
    explicit DFontPreviewListView(QWidget *parent = nullptr);
    ~DFontPreviewListView() override;

    void initFontListData();
    void initDelegate();

    int getListDataCount();
    bool isListDataLoadFinished();
    void refreshFontListData();

    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    void setSelection(const QRect &rect,
                      QItemSelectionModel::SelectionFlags command) Q_DECL_OVERRIDE;

    void setModel(QAbstractItemModel *model) Q_DECL_OVERRIDE;

    void setRightContextMenu(QMenu *rightMenu);

    QModelIndex currModelIndex();
    DFontPreviewItemData currModelData();
    DFontPreviewProxyModel *getFontPreviewProxyModel();
    void removeRowAtIndex(QModelIndex modelIndex);

private:
    void initConnections();

    bool enableFont(DFontPreviewItemData itemData);
    bool disableFont(DFontPreviewItemData itemData);

    inline QRect getCollectionIconRect(QRect visualRect);

    bool m_bLoadDataFinish;
    bool m_bLeftMouse;
    QWidget *m_parentWidget;
    QStandardItemModel *m_fontPreviewItemModel {nullptr};
    QList<DFontPreviewItemData> m_fontPreviewItemDataList;
    DFontPreviewItemDelegate *m_fontPreviewItemDelegate {nullptr};

    QMenu *m_rightMenu {nullptr};

    QModelIndex m_currModelIndex;
    DFontPreviewProxyModel *m_fontPreviewProxyModel {nullptr};

    DFontPreviewListDataThread *m_dataThread;

signals:
    //用于DFontPreviewListView内部使用的信号
    void onClickEnableButton(QModelIndex index);
    void onClickCollectionButton(QModelIndex index);
    void onShowContextMenu(QModelIndex index);

    //右键菜单
    void onContextMenu(QModelIndex index);

    //字体列表加载状态
    void onLoadFontsStatus(int type);

private slots:

    void onListViewItemEnableBtnClicked(QModelIndex index);
    void onListViewItemCollectionBtnClicked(QModelIndex index);
    void onListViewShowContextMenu(QModelIndex index);
    void onFinishedDataLoad();
};

#endif  // DFONTPREVIEWLISTVIEW_H

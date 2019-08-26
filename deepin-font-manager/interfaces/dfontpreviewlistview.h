#ifndef DFONTPREVIEWLISTVIEW_H
#define DFONTPREVIEWLISTVIEW_H

#include "dfontpreviewitemdef.h"
#include "dfontpreviewitemdelegate.h"

#include <DListView>

#include <QMouseEvent>
#include <QSortFilterProxyModel>

DWIDGET_USE_NAMESPACE

class DFontPreviewListView : public DListView
{
    Q_OBJECT

public:
    explicit DFontPreviewListView(QWidget *parent = nullptr);

    void initFontListData();
    void initDelegate();

    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void setSelection(const QRect &rect,
                      QItemSelectionModel::SelectionFlags command) Q_DECL_OVERRIDE;

    void setModel(QAbstractItemModel *model) Q_DECL_OVERRIDE;

    void setRightContextMenu(QMenu *rightMenu);

    QModelIndex currModelIndex();
    DFontPreviewItemData currModelData();
    QSortFilterProxyModel *getFontPreviewProxyModel();

private:
    void initConnections();

    bool m_bLeftMouse;
    QStandardItemModel *m_fontPreviewItemModel;
    QList<DFontPreviewItemData> m_fontPreviewItemDataList;
    DFontPreviewItemDelegate *m_fontPreviewItemDelegate;

    QMenu *m_rightMenu;

    QModelIndex m_currModelIndex;
    QSortFilterProxyModel *m_fontPreviewProxyModel;

signals:
    //用于DFontPreviewListView内部使用的信号
    void onClickEnableButton(QModelIndex index);
    void onClickCollectionButton(QModelIndex index);
    void onShowContextMenu(QModelIndex index);

    //右键菜单
    void onContextMenu(QModelIndex index);

private slots:

    void onListViewItemEnableBtnClicked(QModelIndex index);
    void onListViewItemCollectionBtnClicked(QModelIndex index);
    void onListViewShowContextMenu(QModelIndex index);
};

#endif  // DFONTPREVIEWLISTVIEW_H

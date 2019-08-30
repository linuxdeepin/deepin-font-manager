#ifndef DFONTPREVIEWLISTVIEW_H
#define DFONTPREVIEWLISTVIEW_H

#include "dfontpreviewitemdef.h"
#include "dfontpreviewitemdelegate.h"
#include "dfontpreviewproxymodel.h"
#include "dsqliteutil.h"

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

    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void setSelection(const QRect &rect,
                      QItemSelectionModel::SelectionFlags command) Q_DECL_OVERRIDE;

    void setModel(QAbstractItemModel *model) Q_DECL_OVERRIDE;

    void setRightContextMenu(QMenu *rightMenu);

    QModelIndex currModelIndex();
    DFontPreviewItemData currModelData();
    DFontPreviewProxyModel *getFontPreviewProxyModel();

private:
    void initConnections();
    void insertFontItemData(QString filePath, int index);
    QMap<QString, QString> mapItemData(DFontPreviewItemData itemData);

    bool m_bLeftMouse;
    QStandardItemModel *m_fontPreviewItemModel;
    QList<DFontPreviewItemData> m_fontPreviewItemDataList;
    DFontPreviewItemDelegate *m_fontPreviewItemDelegate;

    QMenu *m_rightMenu;

    QModelIndex m_currModelIndex;
    DFontPreviewProxyModel *m_fontPreviewProxyModel;

    DSqliteUtil *m_sqlUtil;

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

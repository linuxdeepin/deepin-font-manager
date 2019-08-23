#ifndef DFONTPREVIEWLISTVIEW_H
#define DFONTPREVIEWLISTVIEW_H

#include <QMouseEvent>

#include <DListView>

DWIDGET_USE_NAMESPACE

class DFontPreviewListView : public DListView
{
    Q_OBJECT

public:
    explicit DFontPreviewListView(QWidget *parent = nullptr);

    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void setSelection(const QRect &rect,
                      QItemSelectionModel::SelectionFlags command) Q_DECL_OVERRIDE;

    void setModel(QAbstractItemModel *model) Q_DECL_OVERRIDE;

private:
    void initConnections();

    bool m_bLeftMouse;
    QStandardItemModel *m_fontPreviewItemModel;

    QModelIndex m_currModelIndex;

signals:
    //用于DFontPreviewListView内部使用的信号
    void onClickEnableButton(QModelIndex index);
    void onClickCollectionButton(QModelIndex index);
    void onShowContextMenu(QModelIndex index);

    //右键菜单
    void onContextMenu(QModelIndex index);

    //添加字体
    void onAddFont(QModelIndex index);
    //启用字体
    void onEnableFont(QModelIndex index);
    //删除字体
    void onDeleteFont(QModelIndex index);
    //收藏字体
    void onCollectFont(QModelIndex index);
    //显示字体信息
    void onDisplayFontInfo(QModelIndex index);
    //在文件管理器中显示
    void onShowFontFile(QModelIndex index);

private slots:

    void onListViewItemEnableBtnClicked(QModelIndex index);
    void onListViewItemCollectionBtnClicked(QModelIndex index);
    void onListViewShowContextMenu(QModelIndex index);

    void onClickAddFontRightMenu();
    void onClickEnableFontRightMenu();
    void onClickDeleteFontRightMenu();
    void onClickCollectFontRightMenu();
    void onClickDisplayFontInfoRightMenu();
    void onClickShowFontFileRightMenu();
};

#endif  // DFONTPREVIEWLISTVIEW_H

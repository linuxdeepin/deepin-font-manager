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
    void refreshFontListData(const QStringList &installFont);

    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

    void setSelection(const QRect &rect,
                      QItemSelectionModel::SelectionFlags command) Q_DECL_OVERRIDE;

    void setModel(QAbstractItemModel *model) Q_DECL_OVERRIDE;

    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) Q_DECL_OVERRIDE;

    void setRightContextMenu(QMenu *rightMenu);

    QModelIndex currModelIndex();
    DFontPreviewItemData currModelData();
    DFontPreviewProxyModel *getFontPreviewProxyModel();
    void clearPressState();
    void clearHoverState();
    void updateChangedFile(const QString &path);
    void updateChangedDir(const QString &path);
    void deleteFontFiles(const QStringList &files, bool force = false);
    void deleteCurFonts(const QStringList &files);
    void changeFontFile(const QString &path, bool force = false);
    QStringList selectedFonts(int *deleteCnt, int *systemCnt);
    QList<DFontPreviewItemData> selectedFontData(int *deleteCnt, int *systemCnt);
    QModelIndexList selectedIndex(int *deleteCnt, int *systemCnt);
    void deleteFontModelIndex(const QString &filePath, bool isFromSys = false);
    inline bool isDeleting();
    QMutex *getMutex();
    void enableFont(const QString &filePath);
    void disableFont(const QString &filePath);
    void enableFonts();
    void disableFonts();

protected:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

private:
    void initConnections();

    inline QRect getCollectionIconRect(QRect visualRect);

    void deleteFontModelIndex(const DFontInfo &fontInfo);

    void clearFontSelect();
    void sortModelIndexList(QModelIndexList &sourceList);

    QStringList getSelectFont(const QStringList &fontList);

    bool m_bLoadDataFinish = false;
    bool m_bLeftMouse = true;
    bool m_bClickCollectionOrEnable = false;
    QWidget *m_parentWidget;
    QStandardItemModel *m_fontPreviewItemModel {nullptr};
    QList<DFontPreviewItemData> m_fontPreviewItemDataList;
    DFontPreviewItemDelegate *m_fontPreviewItemDelegate {nullptr};

    QMenu *m_rightMenu {nullptr};

    QModelIndex m_currModelIndex;
    DFontPreviewProxyModel *m_fontPreviewProxyModel {nullptr};

    DFontPreviewListDataThread *m_dataThread;
    QModelIndex m_pressModelIndex;
    QModelIndex m_hoverModelIndex;
    QMutex m_mutex;
    QStringList m_enableFontList;
    QStringList m_disableFontList;

signals:
    //用于DFontPreviewListView内部使用的信号
    void onClickEnableButton(const QModelIndexList &index, bool setValue);
    void onClickCollectionButton(const QModelIndexList &index, bool setValue);
    void onShowContextMenu(const QModelIndex &index);

    //右键菜单
    void onContextMenu(const QModelIndex &index);

    //字体列表加载状态
    void onLoadFontsStatus(int type);

    void requestDeleted(const QStringList &files);
    void requestAdded(const QStringList &files);
    void itemAdded(const DFontPreviewItemData &data);
    void itemRemoved(const DFontPreviewItemData &data);
    void itemRemovedFromSys(const DFontPreviewItemData &data);
    void itemSelected(const QStringList &files);
    void itemSingleSelect(const QString &file);

public slots:

    void onListViewItemEnableBtnClicked(const QModelIndexList &itemIndexes, bool setValue);
    void onListViewItemCollectionBtnClicked(const QModelIndexList &index, bool setValue);
    void onListViewShowContextMenu(const QModelIndex &index);
    void onFinishedDataLoad();
    void selectFonts(const QStringList &fileList);
    void selectFont(const QString &file);
    void onItemAdded(const DFontPreviewItemData &itemData);
    void onItemRemoved(const DFontPreviewItemData &itemData);
    void onItemRemovedFromSys(const DFontPreviewItemData &itemData);
};

#endif  // DFONTPREVIEWLISTVIEW_H

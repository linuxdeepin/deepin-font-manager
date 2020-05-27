#ifndef DFONTPREVIEWLISTVIEW_H
#define DFONTPREVIEWLISTVIEW_H

#include "dfontpreviewitemdef.h"
#include "dfontpreviewitemdelegate.h"
#include "dfontpreviewlistdatathread.h"
#include "dfontpreviewproxymodel.h"
#include "dfmdbmanager.h"
#include "signalmanager.h"
#include <QScrollBar>
#include <DListView>

#include <QMouseEvent>
#include <QSortFilterProxyModel>

DWIDGET_USE_NAMESPACE

class DFontPreviewListView : public DListView
{
    Q_OBJECT
public:
    enum FontGroup {
        AllFont,        //所有字体
        SysFont,        //系统字体
        UserFont,       //用户字体
        CollectFont,    //收藏
        ActiveFont,     //已激活
        ChineseFont,    //中文
        EqualWidthFont  //等宽
    };

public:
    explicit DFontPreviewListView(QWidget *parent = nullptr);
    ~DFontPreviewListView() override;

    void initFontListData();
    void initDelegate();

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
    void keyPressEvent(QKeyEvent *event)Q_DECL_OVERRIDE;


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
    void selectedFontsNum(int *deleteCnt, int *systemCnt);
    QModelIndexList selectedIndex(int *deleteCnt, int *systemCnt);
    void deleteFontModelIndex(const QString &filePath, bool isFromSys = false);
    void updateFont(bool nofont = true);
    inline bool isDeleting();
    QMutex *getMutex();
    void enableFont(const QString &filePath);
    void disableFont(const QString &filePath);
    void enableFonts();
    void disableFonts();
    void toSetCurrentIndex(QModelIndexList &itemIndexesNew);
    bool isAtListviewBottom();
    bool isAtListviewTop();
    QString getPreviewTextWithSize(int *fontSize = nullptr);
    void setCurrentSelected(int indexRow);
    static bool misdelete;//是否是删除操作
    static int mfontsize;//当前字号的大小
    static QString mcurtext;//当前显示内容
protected:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

private:
    void initConnections();

    inline QRect getCollectionIconRect(QRect visualRect);

    void deleteFontModelIndex(const DFontInfo &fontInfo);

    void sortModelIndexList(QModelIndexList &sourceList);

    void scrollWithTheSelected();
    void refreshRect();

    bool m_bLoadDataFinish = false;
    bool m_bLeftMouse = true;
    bool m_bRightMous = true;
    bool m_bClickCollectionOrEnable = false;
    bool m_bListviewAtButtom = false;
    bool m_bListviewAtTop = false;
    QWidget *m_parentWidget;
    QStandardItemModel *m_fontPreviewItemModel {nullptr};

    DFontPreviewItemDelegate *m_fontPreviewItemDelegate {nullptr};
    SignalManager *m_signalManager = SignalManager::instance();
    QMenu *m_rightMenu {nullptr};

    QModelIndex m_currModelIndex;
    DFontPreviewProxyModel *m_fontPreviewProxyModel {nullptr};

    DFontPreviewListDataThread *m_dataThread;
    QModelIndex m_pressModelIndex;
    QModelIndex m_hoverModelIndex;
    QMutex m_mutex;
    QStringList m_enableFontList;
    QStringList m_disableFontList;
    FontGroup m_currentFontGroup;

    QRect m_curRect;
    int m_currentSelectedRow = -1;
    bool isSelectedNow = false;
    bool m_isJustInstalled = false;

    int m_selectAfterDel = -1;/*539 删除后的选中位置*/

signals:
    //用于DFontPreviewListView内部使用的信号
    void onClickEnableButton(const QModelIndexList &index, bool setValue, bool isFromActiveFont = false);
    void onClickCollectionButton(const QModelIndexList &index, bool setValue, bool isFromCollectFont = false);
    void onShowContextMenu(const QModelIndex &index);

    //右键菜单
    void onContextMenu(const QModelIndex &index);

    //字体列表加载状态
    void onLoadFontsStatus(int type);

    void requestDeleted(const QStringList &files);
    void requestAdded(const QStringList &files, bool isFirstInstall = false);
    void itemAdded(const DFontPreviewItemData &data);
    void multiItemsAdded(const QList<DFontPreviewItemData> &data);
    void itemRemoved(const DFontPreviewItemData &data);
    void itemRemovedFromSys(const DFontPreviewItemData &data);
    void itemsSelected(const QStringList &files, bool isFirstInstall = false);
    void itemSelected(const QString &file);
    void rowCountChanged();

public slots:

    void onListViewItemEnableBtnClicked(const QModelIndexList &itemIndexes, bool setValue, bool isFromActiveFont = false);
    void onListViewItemCollectionBtnClicked(const QModelIndexList &index, bool setValue, bool isFromCollectFont = false);
    void onListViewShowContextMenu(const QModelIndex &index);
    void onFinishedDataLoad();
    void selectFonts(const QStringList &fileList);
    void selectFont(const QString &file);
    void onItemAdded(const DFontPreviewItemData &itemData);
    void onMultiItemsAdded(const QList<DFontPreviewItemData> &data);
    void onItemRemoved(const DFontPreviewItemData &itemData);
    void onItemRemovedFromSys(const DFontPreviewItemData &itemData);
    void updateCurrentFontGroup(int currentFontGroup);
};

#endif  // DFONTPREVIEWLISTVIEW_H

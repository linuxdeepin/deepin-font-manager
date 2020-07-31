#ifndef DFONTPREVIEWLISTVIEW_H
#define DFONTPREVIEWLISTVIEW_H

#include "dfontpreviewitemdef.h"
#include "dfontpreviewitemdelegate.h"
#include "dfontpreviewlistdatathread.h"
#include "dfontpreviewproxymodel.h"
#include "dfmdbmanager.h"
#include "signalmanager.h"
#include "../views/dfontspinnerwidget.h"
#include <QScrollBar>
#include <QListView>

#include <QMouseEvent>
#include <QSortFilterProxyModel>

DWIDGET_USE_NAMESPACE

class DFontPreviewListView : public QListView
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
    void deleteCurFonts(const QStringList &files, bool force = false);
    void changeFontFile(const QString &path, bool force = false);
    void selectedFonts(int *deleteCnt = nullptr, int *systemCnt = nullptr,
                       int *curFontCnt = nullptr, int *disableCnt = nullptr,
                       QStringList *delFontList = nullptr, QModelIndexList *allIndexList = nullptr,
                       QModelIndexList *disableIndexList = nullptr, QStringList *allMinusSysFontList = nullptr, DFontPreviewItemData *curData = nullptr);
    inline void appendFilePath(QStringList *allFontList, const QString &filePath)
    {
        if ((allFontList != nullptr) && (!allFontList->contains(filePath)))
            *allFontList << filePath;
    }
    void deleteFontModelIndex(const QString &filePath, bool isFromSys = false);
    inline bool isDeleting();
    QMutex *getMutex();
    QMenu *m_rightMenu {nullptr};
    void enableFont(const QString &filePath);
    void disableFont(const QString &filePath);
    void enableFonts();
    void disableFonts();
    void updateShiftSelect(const QModelIndex &modelIndex);
    void toSetCurrentIndex(QModelIndexList &itemIndexesNew);
    void toSetCurrentIndex(QModelIndexList &itemIndexesNew, int count, int size);
    bool isAtListviewBottom();
    bool isAtListviewTop();
    QString getPreviewTextWithSize(int *fontSize = nullptr);
    void setCurrentSelected(int indexRow);
    void cancelDel();
    void viewChanged();
    void markPositionBeforeRemoved(bool isDelete, const QModelIndexList &list); //记录移除前位置
    void refreshFocuses();
    void updateSpinner(DFontSpinnerWidget::SpinnerStyles style, bool force = true);
    inline QString getCurFontStrName()
    {
        return QString("%1").arg(m_curFontData.strFontName);
    }

    inline DFontPreviewItemData getCurFontData()
    {
        return m_curFontData;
    }

protected:
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected) override;

private:
    void initConnections();
    int count() const;
    inline QRect getCollectionIconRect(const QRect &rect);
    inline QRect getCheckboxRect(const QRect &rect);
    void deleteFontModelIndex(const DFontInfo &fontInfo);
    bool isCurrentFont(DFontPreviewItemData &itemData);
    void sortModelIndexList(QModelIndexList &sourceList);
    void scrollWithTheSelected();
    void refreshRect();
    void updateSelection();
    int getOnePageCount();

    bool m_bLoadDataFinish = false;
    volatile bool m_fontChanged = false;
    bool m_bListviewAtButtom = false;
    bool m_bListviewAtTop = false;
    QWidget *m_parentWidget;
    QStandardItemModel *m_fontPreviewItemModel {nullptr};

    DFontPreviewItemDelegate *m_fontPreviewItemDelegate {nullptr};
    SignalManager *m_signalManager = SignalManager::instance();

    DFontPreviewProxyModel *m_fontPreviewProxyModel {nullptr};

    DFontPreviewListDataThread *m_dataThread;
    QTimer *m_fontChangeTimer;
    QFont m_curAppFont;
    QModelIndex m_currModelIndex;
    QModelIndex m_pressModelIndex;
    QModelIndex m_hoverModelIndex;
    QMutex m_mutex;
    QStringList m_enableFontList;
    QStringList m_disableFontList;
    QStringList m_currentFont;
    DFontPreviewItemData m_curFontData;
    FontGroup m_currentFontGroup;

    QRect m_curRect;
    int m_currentSelectedRow = -1;

    int m_selectAfterDel = -1;/*539 删除后的选中位置*/
    int m_tryCnt = 0;
    qint64 m_curTm {0};

signals:
    //字体列表加载状态
    void onLoadFontsStatus(int type);

    void requestDeleted(const QStringList &files);
    void requestAdded(const QStringList &files, bool isFirstInstall = false);
    void itemAdded(const DFontPreviewItemData &data);
    void multiItemsAdded(QList<DFontPreviewItemData> &data, DFontSpinnerWidget::SpinnerStyles styles);
    void itemRemoved(const DFontPreviewItemData &data);
    void itemRemovedFromSys(const DFontPreviewItemData &data);
    void itemsSelected(const QStringList &files, bool isFirstInstall = false);
    void rowCountChanged();
    void deleteFinished();
    void requestUpdateModel(bool showSpinner);
    void requestShowSpinner(bool bShow, bool force, DFontSpinnerWidget::SpinnerStyles style);

public slots:

    void onEnableBtnClicked(const QModelIndexList &itemIndexes, int systemCnt, int curCnt, bool setValue, bool isFromActiveFont = false);
    void onCollectBtnClicked(const QModelIndexList &index, bool setValue, bool isFromCollectFont = false);
    void onListViewShowContextMenu();
    void onFinishedDataLoad();
    void selectFonts(const QStringList &fileList);
    void onItemAdded(const DFontPreviewItemData &itemData);
    void onMultiItemsAdded(QList<DFontPreviewItemData> &data, DFontSpinnerWidget::SpinnerStyles styles);
    void onUpdateCurrentFont();
    void onFontChanged(const QFont &font);
    void onItemRemoved(const DFontPreviewItemData &itemData);
    void onItemRemovedFromSys(const DFontPreviewItemData &itemData);
    void updateCurrentFontGroup(int currentFontGroup);
    void updateModel(bool showSpinner = true);
};

#endif  // DFONTPREVIEWLISTVIEW_H

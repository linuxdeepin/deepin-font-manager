#ifndef DFONTMGRMAINWINDOW_H
#define DFONTMGRMAINWINDOW_H

#include "dfontpreviewlistview.h"
#include "dfontmanager.h"

#include <QShortcut>

#include <DFrame>
#include <DMainWindow>

DWIDGET_USE_NAMESPACE

/*
 * TODO:
 *     The font manager may need to be move to library.
 *
 */
class DFQuickInstallWindow;
class DFontMgrMainWindowPrivate;
class DFontMgrMainWindow : public DMainWindow
{
    Q_OBJECT

public:
    explicit DFontMgrMainWindow(bool isQuickMode = false, QWidget *parent = nullptr);
    ~DFontMgrMainWindow() override;

    static constexpr int MIN_FONT_SIZE = 6;
    static constexpr int MAX_FONT_SIZE = 60;
    static constexpr int DEFAULT_FONT_SIZE = FTM_DEFAULT_PREVIEW_FONTSIZE;
    static constexpr char const *FMT_FONT_SIZE = "%dpx";

    enum Theme {
        Dark,
        Light,
        FollowSystem,
    };

    void setQuickInstallMode(bool isQuick);
    void hideQucikInstallWindow();
    void InitQuickWindowIfNeeded();

protected:
    void initData();
    void initUI();
    void initConnections();
    void initShortcuts();

    void initTileBar();
    void initTileFrame();
    void initMainVeiws();
    void initLeftSideBar();
    void initRightFontView();
    void initStateBar();
    void initRightKeyMenu();
    void initFontPreviewListView(QWidget *parent);
    void initFontPreviewItemsData();

    void handleAddFontEvent();
    void installFont(const QStringList &files);
    void showFontFilePostion();

    void delCurrentFont();

    DFontPreviewListView *m_fontPreviewListView;
    DListView *m_noResultListView;
signals:
    void fileSelected(const QStringList files) const;

    // Only use when user double click font file
    void quickModeInstall(const QStringList files) const;
public slots:
    void handleMenuEvent(QAction *);

    void onSearchTextChanged(const QString &currStr);
    void onPreviewTextChanged(const QString &currStr);
    void onFontSizeChanged(int fontSize);

    void onLeftSiderBarItemClicked(int index);
    void onFontInstallFinished();
    void onFontUninstallFinished(const QModelIndex &uninstallIndex);

    void onFontListViewRowCountChanged(bool bShowNoResult);

protected:
    // For quick install mode
    bool m_isQuickMode = {false};
    DFontManager *m_fontManager;
    QShortcut *m_scFullScreen;  //全屏快捷键F11
    QShortcut *m_scZoomIn;        //放大字体快捷键Ctrl+=
    QShortcut *m_scZoomOut;       //放大字体快捷键Ctrl+-
    QShortcut *m_scDefaultSize;   //默认⼤⼩字体快捷键Ctrl+0
    int m_previewFontSize;

    //Stand shortcut
    //Implement by DTK                       //Close window     --> Alt+F4
    QShortcut *m_scShowAllSC     {nullptr};  //Show shortcut    --> Ctrl+Shift+/
    QShortcut *m_scWndReize      {nullptr};  //Resize Window    --> Ctrl+Alt+F
    QShortcut *m_scFindFont      {nullptr};  //Find font        --> Ctrl+F
    QShortcut *m_scDeleteFont    {nullptr};  //Delete font      --> Delete
    QShortcut *m_scAddNewFont    {nullptr};  //Add Font         --> Ctrl+O
    QShortcut *m_scAddFavFont    {nullptr};  //Add favorite     --> Ctrl+K
    QShortcut *m_scCancelFavFont {nullptr};  //Cancel favorite  --> Ctrl+Shift+K
    QShortcut *m_scFontInfo      {nullptr};  //Font information --> Alt+Enter

    QScopedPointer<DFQuickInstallWindow> m_quickInstallWnd;

    QScopedPointer<DFontMgrMainWindowPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DFontMgrMainWindow)
};

#endif  // DFONTMGRMAINWINDOW_H

#ifndef DFONTMGRMAINWINDOW_H
#define DFONTMGRMAINWINDOW_H

#include "dfontpreviewlistview.h"
#include "dfontmanager.h"
#include "views/dfontspinnerwidget.h"
#include "dfontuninstalldialog.h"
#include "signalmanager.h"
#include "views/dfdeletedialog.h"

#include <QShortcut>

#include <DFrame>
#include <DMainWindow>
#include <QResizeEvent>



DWIDGET_USE_NAMESPACE

/*
 * TODO:
 *     The font manager may need to be move to library.
 *
 */
class DFQuickInstallWindow;
class DFInstallNormalWindow;
class DFontMgrMainWindowPrivate;
class DFontMgrMainWindow : public DMainWindow
{
    Q_OBJECT

    void startToDelete();
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
    void forceNoramlInstalltionQuitIfNeeded();
    void setDeleteFinish();
    inline DFontManager *getFontManager()
    {
        return m_fontManager;
    }
    inline bool isDeleting()
    {
        return m_fIsDeleting;
    }
    inline void setFileList(QStringList list)
    {
        m_fileList = list;
    }

    //Main window Size
    int m_winHight;
    int m_winWidth;
protected:
    void initData();
    void initUI();
    void initConnections();
    void initShortcuts();
    void initFontFiles();
    void initFontUninstallDialog();
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
    void installFontFromSys(const QStringList &files);
    void showFontFilePostion();
    void delCurrentFont();
    void exportFont();
    void showAllShortcut();
    void showInstalledFiles(const QStringList &fileList);
    void checkCloseUninstallDialog();
    void waitForInsert();

    //Add drag install
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    DFontPreviewListView *m_fontPreviewListView;
    DListView *m_noResultListView;
    DListView *m_noInstallListView;
    DFontSpinnerWidget *m_fontLoadingSpinner {nullptr};
    DFontuninstalldialog *m_fontUninstallDialog;
    DFMDBManager *m_dbManager {nullptr};
    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;
signals:
    void fileSelected(const QStringList &files) const;
    void fileSelectedInSys(const QStringList &files) const;
    void requestDeleted(const QStringList &files);

    // Only use when user double click font file
    void quickModeInstall(const QStringList files) const;
    void requestUpdatePreview();

    void requestHide();

public slots:
    void handleMenuEvent(QAction *);

    void onSearchTextChanged(const QString &currStr);
    void onPreviewTextChanged(const QString &currStr);
    void onFontSizeChanged(int fontSize);

    void onLeftSiderBarItemClicked(int index);
    void onFontInstallFinished(const QStringList &fileList);
    void onFontUninstallFinished(const QStringList &uninstallIndex);

    void onFontListViewRowCountChanged(unsigned int bShow);

    void onLoadStatus(int type);
    void onShowMessage(int totalCount);

protected:
    // For quick install mode
    bool m_isQuickMode = {false};
    DFontManager *m_fontManager;
    SignalManager *m_signalManager = SignalManager::instance();
    QShortcut *m_scFullScreen;  //全屏快捷键F11
    QShortcut *m_scZoomIn;        //放大字体快捷键Ctrl+=
    QShortcut *m_scZoomOut;       //放大字体快捷键Ctrl+-
    QShortcut *m_scDefaultSize;   //默认⼤⼩字体快捷键Ctrl+0
    int m_previewFontSize;
    int m_leftIndex {0};
    int m_needDelCount = 0;
    int m_deledCount = 0;

    bool m_searchTextStatusIsEmpty = true;
    bool m_isDeleting = false;
    bool m_isFromSys = false;

    //Stand shortcut
    //Implement by DTK                       //Close window       --> Alt+F4
    QShortcut *m_scShowAllSC     {nullptr};  //Show shortcut      --> Ctrl+Shift+/
    QShortcut *m_scPageUp        {nullptr};  //Show previous page --> PageUp
    QShortcut *m_scPageDown      {nullptr};  //Show next page     --> PageDown
    QShortcut *m_scWndReize      {nullptr};  //Resize Window      --> Ctrl+Alt+F
    QShortcut *m_scFindFont      {nullptr};  //Find font          --> Ctrl+F
    QShortcut *m_scDeleteFont    {nullptr};  //Delete font        --> Delete
    QShortcut *m_scAddNewFont    {nullptr};  //Add Font           --> Ctrl+O
    QShortcut *m_scAddFavFont    {nullptr};  //Add favorite       --> Ctrl+K
    QShortcut *m_scCancelFavFont {nullptr};  //Cancel favorite    --> Ctrl+Shift+K
    QShortcut *m_scFontInfo      {nullptr};  //Font information   --> Alt+Enter

    //is in installing font flow
    //Avoid start multi-NormalInstalltion window
    bool                    m_fIsInstalling {false};
    //is it in uninstalling font flow
    //Avoid start multi delete confirm dialog
    volatile bool m_fIsDeleting {false};
    QStringList m_fileList;
    QStringList m_uninstallFilePath;
    QStringList m_waitForInstall;
    DFInstallNormalWindow  *m_dfNormalInstalldlg {nullptr};

    QScopedPointer<DFQuickInstallWindow> m_quickInstallWnd;

    QScopedPointer<DFontMgrMainWindowPrivate> d_ptr;

    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DFontMgrMainWindow)

public:
    DSpinner *waitForInstallSpinner = nullptr;
};

#endif  // DFONTMGRMAINWINDOW_H

#ifndef DFONTMGRMAINWINDOW_H
#define DFONTMGRMAINWINDOW_H

#include "dfontpreviewlistview.h"

#include <DMainWindow>

DWIDGET_USE_NAMESPACE

/*
 * TODO:
 *     The font manager may need to be move to library.
 *
 */

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

protected:
    void initData();
    void initUI();
    void initConnections();

    void initTileBar();
    void initTileFrame();
    void initToolBar();
    void initMainVeiws();
    void initLeftSideBar();
    void initRightFontView();
    void initStateBar();
    void initRightKeyMenu();
    void initFontPreviewListView(QFrame *parent);
    void initFontPreviewItemsData();

    void handleAddFontEvent();
    void switchAppTheme(int type);
    void installFont(const QStringList &files);
    void showFontFilePostion();

    void resizeEvent(QResizeEvent *event) Q_DECL_OVERRIDE;

    DFontPreviewListView *m_fontPreviewListView;
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

protected:
    // For quick install mode
    bool m_isQuickMode = {false};

    QScopedPointer<DFontMgrMainWindowPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DFontMgrMainWindow)
};

#endif  // DFONTMGRMAINWINDOW_H

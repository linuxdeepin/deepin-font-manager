#ifndef DFONTMGRMAINWINDOW_H
#define DFONTMGRMAINWINDOW_H

#include <DMainWindow>

DWIDGET_USE_NAMESPACE

/*
 * TODO:
 *     The font manager may need to be move to library.
 *
 */

class DFontMgrMainWindowPrivate;
class DFontMgrMainWindow : public DMainWindow {
    Q_OBJECT

public:
    explicit DFontMgrMainWindow(QWidget* parent = nullptr);
    ~DFontMgrMainWindow();

    static constexpr int MIN_FONT_SIZE = 6;
    static constexpr int MAX_FONT_SIZE = 60;
    static constexpr int DEFAULT_FONT_SIZE = MIN_FONT_SIZE;
    static constexpr char* FMT_FONT_SIZE = "%dpx";

protected:
    void initUI();
    void initConnections();

    void initTileBar();
    void initTileFrame();
    void initToolBar();
    void initMainVeiws();
    void initLeftSideBar();
    void initRightFontView();
    void initStateBar();

    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
signals:

public slots:

protected:
    QScopedPointer<DFontMgrMainWindowPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DFontMgrMainWindow)
};

#endif  // DFONTMGRMAINWINDOW_H

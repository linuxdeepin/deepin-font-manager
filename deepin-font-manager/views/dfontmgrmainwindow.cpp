#include "views/dfontmgrmainwindow.h"
#include "dfinstallnormalwindow.h"
#include "globaldef.h"
#include "interfaces/dfontmenumanager.h"
#include "utils.h"
#include "views/dfdeletedialog.h"
#include "views/dfontinfodialog.h"
#include "views/dfquickinstallwindow.h"
#include "performancemonitor.h"

#include <DWidgetUtil>
#include <DApplication>
#include <DApplicationHelper>
#include <DFontSizeManager>
#include <DFileDialog>
#include <DIconButton>
#include <DLabel>
#include <DLineEdit>
#include <DMenu>
#include <DSearchEdit>
#include <DSlider>
#include <DSplitter>
#include <DTitlebar>
#include <DMessageManager>
#include <DDesktopServices>

#include <QSettings>
#include <QDateTime>
#include <QMimeData>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QProcess>
#include <QStorageInfo>
#include <QStandardPaths>
#include <QStandardItem>
#include <QDBusConnection>

class DFontMgrMainWindowPrivate
{
public:
    explicit DFontMgrMainWindowPrivate(DFontMgrMainWindow *q)
        : settingsQsPtr(new QSettings(QDir(Utils::getConfigPath()).filePath("config.conf"),
                                      QSettings::IniFormat))
        , q_ptr(q)
    {
    }

    //~DFontMgrMainWindowPrivate() {}
    QWidget *titleActionArea {nullptr};
    DIconButton *addFontButton {nullptr};
    DSearchEdit *searchFontEdit {nullptr};

    QWidget *fontShowArea {nullptr};

    //Shadow line of StateBar
    DHorizontalLine  *sbarShadowLine {nullptr};

    QWidget *stateBar {nullptr};
    DLineEdit *textInputEdit {nullptr};
    DSlider *fontScaleSlider {nullptr};
    DLabel *fontSizeLabel {nullptr};

    DSplitter *mainWndSpliter {nullptr};
    QWidget *leftBarHolder {nullptr};
    QWidget *rightViewHolder {nullptr};

    // Menu
    DMenu *toolBarMenu {nullptr};
    DMenu *rightKeyMenu {nullptr};

    DSplitListWidget *leftSiderBar {nullptr};

    QScopedPointer<QSettings> settingsQsPtr;
    DFontMgrMainWindow *q_ptr;

    Q_DECLARE_PUBLIC(DFontMgrMainWindow)
};

/*************************************************************************
 <Function>      DFontMgrMainWindow
 <Description>   构造函数
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFontMgrMainWindow::DFontMgrMainWindow(bool isQuickMode, QWidget *parent)
    : DMainWindow(parent)
    , m_fontManager(DFontManager::instance())
    , m_scFullScreen(nullptr)
    , m_scZoomIn(nullptr)
    , m_scZoomOut(nullptr)
    , m_scDefaultSize(nullptr)
    , m_previewText(QString()) //用户输入的预览
    , m_isQuickMode(isQuickMode)
    , m_previewFontSize(DEFAULT_FONT_SIZE)
    , m_menuCurCnt(0)
    , m_menuDelCnt(0)
    , m_menuDisableSysCnt(0)
    , m_menuDisableCnt(0)
    , m_quickInstallWnd(nullptr)
    , m_ptr(new DFontMgrMainWindowPrivate(this))
{
    // setWindoDSpinnerwOpacity(0.5); //Debug
    // setWindowFlags(windowFlags() | (Qt::FramelessWindowHint | Qt::WindowMaximizeButtonHint));
    initData();
    initUI();
    initConnections();
    initShortcuts();
    //SP3--设置tab顺序--安装事件过滤器(539)
    installEventFilters();
}

/*************************************************************************
 <Function>      DFontMgrMainWindow
 <Description>   析构函数
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFontMgrMainWindow::~DFontMgrMainWindow()
{
    d_func()->settingsQsPtr->setValue(FTM_MWSIZE_H_KEY, m_winHight);
    d_func()->settingsQsPtr->setValue(FTM_MWSIZE_W_KEY, m_winWidth);
    d_func()->settingsQsPtr->setValue(FTM_MWSTATUS_KEY, m_IsWindowMax);
    //ut000442 bug33870 偶现关闭窗口后,因没有取消dbus服务注册导致的应用无法启动的问题,在这里进行取消
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.unregisterService("com.deepin.FontManager");
}

/*************************************************************************
 <Function>      initData
 <Description>   初始化应用数据
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::initData()
{
    D_D(DFontMgrMainWindow);
    //Initialize app Theme
    QVariant theme;
    theme = d->settingsQsPtr->value(FTM_THEME_KEY);

    bool ok = false;
    int color = theme.toInt(&ok);

    DGuiApplicationHelper::ColorType colorType = DGuiApplicationHelper::ColorType::UnknownType;

    if (ok) {
        colorType = static_cast<DGuiApplicationHelper::ColorType>(color);
    }

    m_winHight = static_cast<short>(d->settingsQsPtr->value(FTM_MWSIZE_H_KEY).toInt());
    m_winWidth = static_cast<short>(d->settingsQsPtr->value(FTM_MWSIZE_W_KEY).toInt());
    m_IsWindowMax = d->settingsQsPtr->value(FTM_MWSTATUS_KEY).toBool();
    qDebug() << __FUNCTION__ << "init theme = " << colorType;

    DGuiApplicationHelper::instance()->setPaletteType(colorType);
}

/*************************************************************************
 <Function>      initUI
 <Description>   页面初始化入口
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::initUI()
{
    //Enable main window accept drag event
    setAcceptDrops(true);
    //m_loadingSpinner = new DSpinner(this);
    //m_loadingSpinner->setFixedSize(32, 32);
    //m_loadingSpinner->hide();
    initTileBar();
    initRightKeyMenu();
    initMainVeiws();
}

/*************************************************************************
 <Function>      installEventFilters
 <Description>   SP3--设置tab顺序--安装事件过滤器
 <Author>        UT000539
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::installEventFilters()
{
    D_D(DFontMgrMainWindow);
    titlebar()->installEventFilter(this);//标题栏
    d->leftSiderBar->installEventFilter(this);// 菜单
    m_fontPreviewListView->installEventFilter(this);//预览区域
    d->textInputEdit->lineEdit()->installEventFilter(this);//输入框
    d->addFontButton->installEventFilter(this);//添加按钮
    d->fontScaleSlider->installEventFilter(this);//滑块
    d->searchFontEdit->lineEdit()->installEventFilter(this);//输入框
    m_noInstallListView->installEventFilter(this);//无字体页面
    m_noResultListView->installEventFilter(this);//无结果页面
}

/*************************************************************************
 <Function>      initConnections
 <Description>   初始化信号和槽的连接函数
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::initConnections()
{
    D_D(DFontMgrMainWindow);

    // Loading Font List Signal
    QObject::connect(m_fontPreviewListView, SIGNAL(onLoadFontsStatus(int)),
                     this, SLOT(onLoadStatus(int)));

    connect(m_fontPreviewListView, &DFontPreviewListView::rowCountChanged, this,
            &DFontMgrMainWindow::onFontListViewRowCountChanged, Qt::UniqueConnection);

    connect(m_fontPreviewListView, &DFontPreviewListView::requestShowSpinner, this,
            &DFontMgrMainWindow::onShowSpinner);

    connect(m_fontPreviewListView, &DFontPreviewListView::deleteFinished, this, [ = ]() {
        setDeleteFinish();
    });
    connect(d->searchFontEdit, &DSearchEdit::searchAborted, [ = ] {
        d->searchFontEdit->lineEdit()->setFocus(Qt::TabFocusReason);
    });
    // Add Font button event
    QObject::connect(d->addFontButton, &DIconButton::clicked, this,
                     &DFontMgrMainWindow::handleAddFontEvent);

    QObject::connect(this, &DFontMgrMainWindow::fileSelected, this,
    [this](const QStringList & files, bool isAddBtnHasTabs) {
        this->installFont(files, isAddBtnHasTabs);
    });

    QObject::connect(this, &DFontMgrMainWindow::fileSelectedInSys, this,
    [this](const QStringList & files) {
        this->installFontFromSys(files);
    });

    // Menu event
    QObject::connect(d->toolBarMenu, &QMenu::triggered, this, &DFontMgrMainWindow::handleMenuEvent);

    //Theme change event
    QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::paletteTypeChanged,
    [this](DGuiApplicationHelper::ColorType type) {
        qDebug() << "Update Theme type:" << type;
        //Save theme value
        d_func()->settingsQsPtr->setValue(FTM_THEME_KEY, type);
    });

    // Right Key menu
    QObject::connect(d->rightKeyMenu, &QMenu::triggered, this,
                     &DFontMgrMainWindow::handleMenuEvent);

    // Initialize rigth menu it state
    QObject::connect(d->rightKeyMenu, &QMenu::aboutToShow, this, [ = ]() {
        qDebug() << __FUNCTION__ << "about toshow";
        //记录操作之前有无tab聚焦
        m_hasMenuTriggered = false;
//        m_fontPreviewListView->setFocus(Qt::MouseFocusReason);
        m_fontPreviewListView->syncRecoveryTabStatus();
        m_menuCurData = m_fontPreviewListView->currModelData();
        m_fontPreviewListView->selectedFonts(m_menuCurData, &m_menuDelCnt, &m_menuDisableSysCnt,
                                             &m_menuSysCnt, &m_menuCurCnt, &m_menuDisableCnt,
                                             &m_menuDelFontList, &m_menuAllIndexList,
                                             &m_menuDisableIndexList, &m_menuAllMinusSysFontList);

        qDebug() << __FUNCTION__ << "sysCnt" <<  m_menuSysCnt << m_menuCurCnt;
        DFontMenuManager::getInstance()->onRightKeyMenuPopup(m_menuCurData, (m_menuDelCnt > 0), (m_menuDisableCnt > 0), (m_menuCurCnt > 0));
        qDebug() << __FUNCTION__ << "about toshow end \n";
    });

    connect(d->rightKeyMenu, &QMenu::aboutToHide, this, [ = ] {
        qDebug() << __FUNCTION__ << "about to hide\n\n";
        m_fontPreviewListView->clearPressState(DFontPreviewListView::ClearType::MoveClear);
        //检查鼠标是否处于hover状态
        m_fontPreviewListView->checkHoverState();
    });

    // State bar event
    QObject::connect(d->fontScaleSlider, &DSlider::valueChanged,
                     this, &DFontMgrMainWindow::respondToValueChanged);

    // Search text changed
    QObject::connect(d->searchFontEdit, SIGNAL(textChanged(const QString &)), this,
                     SLOT(onSearchTextChanged(const QString &)));

    QObject::connect(d->textInputEdit, SIGNAL(textChanged(const QString &)), this,
                     SLOT(onPreviewTextChanged(const QString &)));
    QObject::connect(d->leftSiderBar, SIGNAL(onListWidgetItemClicked(int)), this,
                     SLOT(onLeftSiderBarItemClicked(int)));

    QObject::connect(m_fontManager, &DFontManager::uninstallFontFinished, this, [ = ](QStringList & files) {
        m_fontPreviewListView->updateSpinner(DFontSpinnerWidget::Delete);
        Q_EMIT DFontPreviewListDataThread::instance()->requestDeleted(files);
    });

    QObject::connect(m_fontManager, &DFontManager::uninstallFcCacheFinish, this, &DFontMgrMainWindow::onUninstallFcCacheFinish);

    connect(m_fontManager, &DFontManager::cacheFinish, this, [ = ] {
        qDebug() << __FUNCTION__;
        m_cacheFinish = true;
        hideSpinner();
    });

    connect(DFontPreviewListDataThread::instance(), &DFontPreviewListDataThread::requstShowInstallToast,
    this, [ = ](int fontCnt) {
        m_installFinish = true;
        m_installFontCnt = fontCnt;
        hideSpinner();
    });

    /*调节右下角字体大小显示label显示内容 UT000539*/
    connect(qApp, &DApplication::fontChanged, this, [ = ]() {
        int size = d->fontScaleSlider->value();
        QString fontSize = QString::number(size) + "px";
        autoLabelWidth(fontSize, d->fontSizeLabel, d->fontSizeLabel->fontMetrics());
        m_fontPreviewListView->onFontChanged(qApp->font());
    });

    connect(m_signalManager, &SignalManager::popInstallErrorDialog, this, [ = ] {
        m_isPopInstallErrorDialog = true;
    });

    connect(m_signalManager, &SignalManager::hideInstallErrorDialog, this, [ = ] {
        m_isPopInstallErrorDialog = false;
    });

    connect(m_fontManager, &DFontManager::requestCancelInstall, this, [ = ]() {
        m_fIsInstalling = false;
    });

    //UT000539 增加slider press聚焦的判断
    QObject::connect(d->fontScaleSlider, &DSlider::sliderPressed, [ = ] {
        d->fontScaleSlider->setFocus(Qt::MouseFocusReason);
    });

    connect(m_signalManager, &SignalManager::fontSizeRequestToSlider, this, [ = ] {
        if (!d->searchFontEdit->text().isEmpty())
        {
            onSearchTextChanged(d->searchFontEdit->text());
        }
        //删除过程中安装字体，删除过后要继续安装
        qDebug() << "m_waitForInstall" << m_waitForInstall;
        waitForInsert();
    });

    connect(DFontPreviewListDataThread::instance(), &DFontPreviewListDataThread::exportFontFinished,
    this, [ = ](int count) {
        showExportFontMessage(count, m_menuAllMinusSysFontList.count() - count);
        PerformanceMonitor::exportFontFinish(count);
    });

    //安装字体刷新后，按下左键保持焦点正常切换至菜单
    connect(m_signalManager, &SignalManager::requestSetLeftSiderBarFocus, this, [ = ] {
        emit m_signalManager->setLostFocusState(false);
        d->leftSiderBar->setFocus(Qt::MouseFocusReason);
    });

    connect(m_signalManager, &SignalManager::requestSetTabFocusToAddBtn, [ = ] {
        QTimer::singleShot(25, [ = ]{
            d->addFontButton->setFocus(Qt::TabFocusReason);
        });
    });

    connect(m_signalManager, &SignalManager::onMenuHidden, [ = ] {
        if (m_fontPreviewListView->getFontViewHasFocus())
        {
            m_fontPreviewListView->setFocus(Qt::TabFocusReason);
            if (!m_hasMenuTriggered)
                m_fontPreviewListView->syncTabStatus();

            //  菜单消失后就对这个标志位进行复位，此时删除过程还未结束，删除结束后检测此标志位时为false，
            //  导致后面判断逻辑出错，没法正常设置焦点，所以删除之后无法全选 有因为在后面的代码中有复位
            //  的逻辑所以取消这里代码。
            //  m_fontPreviewListView->setFontViewHasFocus(false);
        }
        m_hasMenuTriggered = false;
    });
}

/*************************************************************************
 <Function>      initShortcuts
 <Description>   初始化快捷键和处理函数
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::initShortcuts()
{
    D_D(DFontMgrMainWindow);

    //设置字体放大快捷键
    if (!m_scZoomIn) {
        m_scZoomIn = new QShortcut(this);
        m_scZoomIn->setKey(tr("Ctrl+="));
        m_scZoomIn->setContext(Qt::ApplicationShortcut);
        m_scZoomIn->setAutoRepeat(false);

        connect(m_scZoomIn, &QShortcut::activated, this, [this, d] {
            if (m_previewFontSize < MAX_FONT_SIZE)
            {
                ++m_previewFontSize;
            }
            d->fontScaleSlider->setValue(m_previewFontSize);
        });
    }

    //设置字体缩小快捷键
    if (!m_scZoomOut) {
        m_scZoomOut = new QShortcut(this);
        m_scZoomOut->setKey(tr("Ctrl+-"));
        m_scZoomOut->setContext(Qt::ApplicationShortcut);
        m_scZoomOut->setAutoRepeat(false);

        connect(m_scZoomOut, &QShortcut::activated, this, [this, d] {
            if (m_previewFontSize > MIN_FONT_SIZE)
            {
                --m_previewFontSize;
            }
            d->fontScaleSlider->setValue(m_previewFontSize);
        });
    }

    //设置字体默认大小快捷键
    if (!m_scDefaultSize) {
        m_scDefaultSize = new QShortcut(this);
        m_scDefaultSize->setKey(tr("Ctrl+0"));
        m_scDefaultSize->setContext(Qt::ApplicationShortcut);
        m_scDefaultSize->setAutoRepeat(false);

        connect(m_scDefaultSize, &QShortcut::activated, this, [this, d] {
            m_previewFontSize = DEFAULT_FONT_SIZE;
            d->fontScaleSlider->setValue(DEFAULT_FONT_SIZE);
        });
    }

    //Show shortcut --> Ctrl+Shift+/
    if (nullptr == m_scShowAllSC) {
        m_scShowAllSC = new QShortcut(this);
        m_scShowAllSC->setKey(tr("Ctrl+Shift+/"));
        m_scShowAllSC->setContext(Qt::ApplicationShortcut);
        m_scShowAllSC->setAutoRepeat(false);

        connect(m_scShowAllSC, &QShortcut::activated, this, [this] {
            m_fontPreviewListView->syncRecoveryTabStatus();
            this->showAllShortcut();
        });
    }

    //Show previous page --> PageUp
    if (nullptr == m_scPageUp) {
        m_scPageUp = new QShortcut(this);
        m_scPageUp->setKey(tr("PgUp"));
        m_scPageUp->setContext(Qt::ApplicationShortcut);
        m_scPageUp->setAutoRepeat(false);

        connect(m_scPageUp, &QShortcut::activated, this, [this] {;
                                                                 //For: PageUP
                                                                 //Scrolling first visible item to bottom
                                                                 QModelIndex firstVisibleItem = this->m_fontPreviewListView->indexAt(QPoint(3, 3));

                                                                 if (firstVisibleItem.isValid())
    {
        m_fontPreviewListView->scrollTo(firstVisibleItem, QAbstractItemView::PositionAtBottom);
        }
                                                                });
    }

    //Show next page --> PageDown
    if (nullptr == m_scPageDown) {
        m_scPageDown = new QShortcut(this);
        m_scPageDown->setKey(tr("PgDown"));
        m_scPageDown->setContext(Qt::ApplicationShortcut);
        m_scPageDown->setAutoRepeat(false);

        connect(m_scPageDown, &QShortcut::activated, this, [this] {
            //For: PageDown
            //Scrolling last visible item to top
            QRect visibleRect = m_fontPreviewListView->geometry();

            QModelIndex lastVisibleItem = this->m_fontPreviewListView->indexAt(QPoint(3, visibleRect.height() - 3));
            if (lastVisibleItem.isValid())
            {
                m_fontPreviewListView->scrollTo(lastVisibleItem, QAbstractItemView::PositionAtTop);
            }
        });
    }

    //Resize Window --> Ctrl+Alt+F
    if (nullptr == m_scWndReize) {
        m_scWndReize = new QShortcut(this);
        m_scWndReize->setKey(tr("Ctrl+Alt+F"));
        m_scWndReize->setContext(Qt::ApplicationShortcut);
        m_scWndReize->setAutoRepeat(false);

        connect(m_scWndReize, &QShortcut::activated, this, [this] {
            if (this->windowState() & Qt::WindowMaximized)
            {
                this->showNormal();
            } else if (this->windowState() == Qt::WindowNoState)
            {
                this->showMaximized();
            }
        });
    }

    //Find font --> Ctrl+F
    if (nullptr == m_scFindFont) {
        m_scFindFont = new QShortcut(this);
        m_scFindFont->setKey(tr("Ctrl+F"));
        m_scFindFont->setContext(Qt::ApplicationShortcut);
        m_scFindFont->setAutoRepeat(false);

        connect(m_scFindFont, &QShortcut::activated, this, [d] {
            d->searchFontEdit->lineEdit()->setFocus(Qt::MouseFocusReason);
        });
    }

    //Delete font --> Delete
    if (nullptr == m_scDeleteFont) {
        m_scDeleteFont = new QShortcut(this);
        m_scDeleteFont->setKey(Qt::Key_Delete);
        m_scDeleteFont->setContext(Qt::ApplicationShortcut);
        m_scDeleteFont->setAutoRepeat(false);

        connect(m_scDeleteFont, &QShortcut::activated, this, [this] {
            //Only can't delete user font
            //first disable delete
            if (m_fIsInstalling)
                return;
            m_fontPreviewListView->syncRecoveryTabStatus();
            delCurrentFont(false);
        }, Qt::UniqueConnection);
    }

    //ShowMenu --> Alt+M//SP3--Alt+M菜单--快捷键
    if (nullptr == m_scShowMenu) {
        m_scShowMenu = new QShortcut(this);
        m_scShowMenu->setKey(tr("Alt+M"));
        m_scShowMenu->setContext(Qt::ApplicationShortcut);
        m_scShowMenu->setAutoRepeat(false);

        connect(m_scShowMenu, &QShortcut::activated, this, [this] {
//            DFontMgrMainWindow *mw = qobject_cast<DFontMgrMainWindow *>(this);
            D_D(DFontMgrMainWindow);
            if (d->searchFontEdit->lineEdit()->hasFocus())
            {
                if (!m_isSearchLineEditMenuPoped) {
                    QPoint GlobalPoint(d->searchFontEdit->mapToGlobal(QPoint(0, 0)));
                    QPoint position = d->searchFontEdit->lineEdit()->rect().center();
                    QPoint popPosition(GlobalPoint.x() + position.x(), GlobalPoint.y() + position.y() + 10);
                    QContextMenuEvent *eve = new QContextMenuEvent(QContextMenuEvent::Reason::Keyboard, popPosition, popPosition);
                    m_isSearchLineEditMenuPoped = QApplication::sendEvent(d->searchFontEdit->lineEdit(), eve);
                    qDebug() << m_isSearchLineEditMenuPoped;
                }

            } else if (d->textInputEdit->lineEdit()->hasFocus())
            {
                if (!m_isInputLineEditMunuPoped) {
                    QPoint GlobalPoint(d->textInputEdit->mapToGlobal(QPoint(0, 0)));
                    QPoint position = d->textInputEdit->lineEdit()->rect().center();
                    QPoint popPosition(GlobalPoint.x() + position.x(), GlobalPoint.y() + position.y() + 10);
                    QContextMenuEvent *eve = new QContextMenuEvent(QContextMenuEvent::Reason::Keyboard, popPosition, popPosition);
                    m_isInputLineEditMunuPoped = QApplication::sendEvent(d->textInputEdit->lineEdit(), eve);
                }
            } else
            {
                m_fontPreviewListView->syncRecoveryTabStatus();
                m_fontPreviewListView->setFontViewHasFocus(m_fontPreviewListView->hasFocus());
                m_fontPreviewListView->onRightMenuShortCutActivated();
                emit m_signalManager->onMenuHidden();
            }
        }, Qt::UniqueConnection);
    }

    //Add Font --> Ctrl+O
    if (nullptr == m_scAddNewFont) {
        m_scAddNewFont = new QShortcut(this);
        m_scAddNewFont->setKey(tr("Ctrl+O"));
        m_scAddNewFont->setContext(Qt::ApplicationShortcut);
        m_scAddNewFont->setAutoRepeat(false);

        connect(m_scAddNewFont, &QShortcut::activated, this, [ = ] {
            m_fontPreviewListView->syncRecoveryTabStatus();
            d->addFontButton->click();
        });
    }

    //Add or cancel favorite --> .
    if (nullptr == m_scAddOrCancelFavFont) {
        m_scAddOrCancelFavFont = new QShortcut(this);
        m_scAddOrCancelFavFont->setKey(/*tr(".")*/Qt::Key_Period);
        m_scAddOrCancelFavFont->setContext(Qt::ApplicationShortcut);
        m_scAddOrCancelFavFont->setAutoRepeat(false);

        connect(m_scAddOrCancelFavFont, &QShortcut::activated, this, [ = ] {
            if (m_fontPreviewListView->m_rightMenu->isVisible())
            {
                m_fontPreviewListView->m_rightMenu->close();
            } else
            {
                m_menuCurData = m_fontPreviewListView->currModelData();
                m_fontPreviewListView->selectedFonts(m_menuCurData, &m_menuDelCnt, &m_menuDisableSysCnt,
                                                     &m_menuSysCnt, &m_menuCurCnt, &m_menuDisableCnt,
                                                     &m_menuDelFontList, &m_menuAllIndexList,
                                                     &m_menuDisableIndexList, &m_menuAllMinusSysFontList);
            }

            if (!m_fontPreviewListView->isVisible() || m_menuAllIndexList.count() == 0)
                return;

            m_fontPreviewListView->onCollectBtnClicked(m_menuAllIndexList, !m_menuCurData.fontData.isCollected(),
                                                       filterGroup == DSplitListWidget::FontGroup::CollectFont);
        });
    }

    //Font information --> CTRL+I
    if (nullptr == m_scFontInfo) {
        m_scFontInfo = new QShortcut(this);
        m_scFontInfo->setKey(tr("CTRL+I"));
        m_scFontInfo->setContext(Qt::ApplicationShortcut);
        m_scFontInfo->setAutoRepeat(false);

        connect(m_scFontInfo, &QShortcut::activated, this, [this] {
            DFontPreviewItemData currItemData = m_fontPreviewListView->currModelData();
            if (m_fontPreviewListView->selectionModel()->selectedIndexes().length() < 1)
            {
                return ;
            }
            if (!currItemData.fontInfo.filePath.isEmpty())
            {
                QAction *fontInfoAction = DFontMenuManager::getInstance()->getActionByMenuAction(
                    DFontMenuManager::M_FontInfo, DFontMenuManager::MenuType::RightKeyMenu);
                m_fontPreviewListView->syncRecoveryTabStatus();
                fontInfoAction->trigger();
            }
        });
    }
}

/*************************************************************************
 <Function>      respondToValueChangedEvent
 <Description>   响应字体大小滑块大小改变
 <Author>        UT000539
 <Input>         value           Description:滑块值大小
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::respondToValueChanged(int value)
{
    D_D(DFontMgrMainWindow);
    m_previewFontSize = static_cast<qint8>(value);
    QString fontSizeText;
    fontSizeText.sprintf(FMT_FONT_SIZE, value);
    //d->fontSizeLabel->setText(fontSizeText);
    //调节右下角字体大小显示label显示内容/*UT000539*/
    autoLabelWidth(fontSizeText, d->fontSizeLabel, d->fontSizeLabel->fontMetrics());
    onFontSizeChanged(value);
}

/*************************************************************************
 <Function>      initTileBar
 <Description>   初始化titleBar
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::initTileBar()
{
    D_D(DFontMgrMainWindow);

    initTileFrame();

    d->toolBarMenu = DFontMenuManager::getInstance()->createToolBarSettingsMenu();
    titlebar()->setMenu(d->toolBarMenu);
    titlebar()->setContentsMargins(0, 0, 0, 0);

    titlebar()->setFixedHeight(FTM_TITLE_FIXED_HEIGHT);
}

/*************************************************************************
 <Function>      initTileFrame
 <Description>   初始化标题栏
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::initTileFrame()
{
    D_D(DFontMgrMainWindow);

    //Add logo
    titlebar()->setIcon(QIcon::fromTheme(DEEPIN_FONT_MANAGER));

    //Action area add a extra space
    d->titleActionArea = new QWidget(this);
    d->titleActionArea->setFixedSize(QSize(FTM_TITLE_FIXED_WIDTH, FTM_TITLE_FIXED_HEIGHT));

    QHBoxLayout *titleActionAreaLayout = new QHBoxLayout(d->titleActionArea);
    titleActionAreaLayout->setSpacing(0);
    titleActionAreaLayout->setContentsMargins(0, 0, 0, 0);

    // Add Font
    d->addFontButton = new DIconButton(DStyle::StandardPixmap::SP_IncreaseElement, this);
    d->addFontButton->setFixedSize(QSize(FTM_ADDBUTTON_PATAM, FTM_ADDBUTTON_PATAM));
    d->addFontButton->setFlat(false);
    d->addFontButton->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    titleActionAreaLayout->addWidget(d->addFontButton);

    // Search font
    d->searchFontEdit = new DSearchEdit(this);
    DFontSizeManager::instance()->bind(d->searchFontEdit, DFontSizeManager::T6);
    d->searchFontEdit->setFixedSize(QSize(FTM_SEARCH_BAR_W, FTM_SEARCH_BAR_H));
    d->searchFontEdit->setPlaceHolder(DApplication::translate("SearchBar", "Search"));

    titlebar()->addWidget(d->searchFontEdit, Qt::AlignCenter);
    titlebar()->addWidget(d->titleActionArea, Qt::AlignLeft | Qt::AlignVCenter);
}

/*************************************************************************
 <Function>      initMainVeiws
 <Description>   初始化主页面
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::initMainVeiws()
{
    D_D(DFontMgrMainWindow);
    setWindowIcon(QIcon::fromTheme(DEEPIN_FONT_MANAGER));

    d->mainWndSpliter = new DSplitter(Qt::Horizontal, this);
    m_fontLoadingSpinner = new DFontSpinnerWidget(this);
//    m_fontDeletingSpinner = new DFontSpinnerWidget(this);
    // For Debug
    // d->mainWndSpliter->setStyleSheet("QSplitter::handle { background-color: red }");

    initLeftSideBar();
    initRightFontView();

    //Disable spliter drag & resize
    QSplitterHandle *handle = d->mainWndSpliter->handle(1);
    if (handle) {
        handle->setFixedWidth(2);
        handle->setDisabled(true);

        DPalette pa = DApplicationHelper::instance()->palette(handle);
        QBrush splitBrush = pa.brush(DPalette::ItemBackground);
        pa.setBrush(DPalette::Background, splitBrush);
        handle->setPalette(pa);
        handle->setBackgroundRole(QPalette::Background);
        handle->setAutoFillBackground(true);
    }

    setCentralWidget(d->mainWndSpliter);
}

/*************************************************************************
 <Function>      initLeftSideBar
 <Description>   初始化页面左侧菜单部分
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::initLeftSideBar()
{
    D_D(DFontMgrMainWindow);

    d->leftBarHolder = new QWidget(d->mainWndSpliter);
    d->leftBarHolder->setObjectName("leftMainLayoutHolder");
    d->leftBarHolder->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    d->leftBarHolder->setFixedWidth(FTM_LEFT_SIDE_BAR_WIDTH);
    d->leftBarHolder->setContentsMargins(0, 0, 2, 0);
    d->leftBarHolder->setBackgroundRole(DPalette::Base);
    d->leftBarHolder->setAutoFillBackground(true);
    // d->leftBarHolder->setAttribute(Qt::WA_TranslucentBackground, true);

    QVBoxLayout *leftMainLayout = new QVBoxLayout();
    leftMainLayout->setContentsMargins(0, 0, 0, 0);
    leftMainLayout->setSpacing(0);

    // ToDo:
    //    Need use the custom QListView replace QListWidget
    d->leftSiderBar = new DSplitListWidget(this);
    // leftSiderBar->setAttribute(Qt::WA_TranslucentBackground, true);
    d->leftSiderBar->setFrameShape(DFrame::NoFrame);
    d->leftSiderBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    leftMainLayout->addSpacing(10);
    leftMainLayout->addWidget(d->leftSiderBar);
    d->leftBarHolder->setLayout(leftMainLayout);

    d->leftSiderBar->setFocus();

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    d->leftBarHolder->setStyleSheet("background: blue");
    d->leftSiderBar->setStyleSheet("background: yellow");
#endif
}

/*************************************************************************
 <Function>      initRightFontView
 <Description>   初始化页面右部分
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::initRightFontView()
{
    Q_D(DFontMgrMainWindow);

    // initialize state bar
    initStateBar();

    d->rightViewHolder = new QWidget(d->mainWndSpliter);
    d->rightViewHolder->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    d->rightViewHolder->setObjectName("rightMainLayoutHolder");
    d->rightViewHolder->setBackgroundRole(DPalette::Base);
    d->rightViewHolder->setAutoFillBackground(true);

    QVBoxLayout *rightMainLayout = new QVBoxLayout();
    rightMainLayout->setContentsMargins(0, 0, 0, 0);
    rightMainLayout->setSpacing(0);

    d->fontShowArea = new QWidget(this);
    //d->fontShowArea->setFrameShape(DFrame::NoFrame);
    d->fontShowArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    initFontPreviewListView(d->fontShowArea);

    // initialize statebar shadow line
    d->sbarShadowLine = new DHorizontalLine(this);
    d->sbarShadowLine->setFixedHeight(1);
    d->sbarShadowLine->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    rightMainLayout->addWidget(d->fontShowArea);
    rightMainLayout->addWidget(d->sbarShadowLine);
    rightMainLayout->addWidget(d->stateBar);

    d->rightViewHolder->setLayout(rightMainLayout);

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    d->fontShowArea->setStyleSheet("background: blue");
    m_fontPreviewListView->setStyleSheet("background: green");
    d->rightViewHolder->setStyleSheet("background: red");
#endif
}

/*************************************************************************
 <Function>      initFontPreviewListView
 <Description>   初始化字体预览页面ListView
 <Author>
 <Input>
    <param1>     parent          Description:父类控件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::initFontPreviewListView(QWidget *parent)
{
    Q_D(DFontMgrMainWindow);

    QVBoxLayout *listViewVBoxLayout = new QVBoxLayout();
    listViewVBoxLayout->setMargin(0);
    listViewVBoxLayout->setContentsMargins(0, 0, 0, 0);
    listViewVBoxLayout->setSpacing(0);

    parent->setLayout(listViewVBoxLayout);

    m_fontPreviewListView = new DFontPreviewListView(this);
    m_fontPreviewListView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_fontPreviewListView->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    m_fontPreviewListView->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    m_fontPreviewListView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_fontPreviewListView->setRightContextMenu(d->rightKeyMenu);

    listViewVBoxLayout->addWidget(m_fontPreviewListView);

    listViewVBoxLayout->addWidget(m_fontLoadingSpinner);

    m_fontLoadingSpinner->spinnerStart();
    m_fontPreviewListView->hide();
    d->stateBar->hide();
    m_fontLoadingSpinner->show();

    // 未搜索到结果view
    m_noResultListView = new DListView(this);

    DLabel *noResultLabel = new DLabel(m_noResultListView);
    noResultLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    noResultLabel->setFixedHeight(noResultLabel->fontMetrics().height() + 30);
    noResultLabel->setText(DApplication::translate("SearchBar", "No search results"));

    QFont labelFont = noResultLabel->font();
    labelFont.setWeight(QFont::ExtraLight);
    noResultLabel->setFont(labelFont);
    noResultLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    DFontSizeManager::instance()->bind(noResultLabel, DFontSizeManager::T4);

    QVBoxLayout *lblLayout = new QVBoxLayout;
    lblLayout->addWidget(noResultLabel);

    m_noResultListView->setLayout(lblLayout);
    listViewVBoxLayout->addWidget(m_noResultListView);

    m_noResultListView->hide();

    // 未安装字体view
    m_noInstallListView = new DListView(this);

    DLabel *noInstallLabel = new DLabel(m_noInstallListView);
    noInstallLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    noInstallLabel->setFixedHeight(noInstallLabel->fontMetrics().height() + 30);
    noInstallLabel->setText(DApplication::translate("SearchBar", "No fonts"));

    QFont labelFontNoInstall = noInstallLabel->font();
    labelFontNoInstall.setWeight(QFont::ExtraLight);
    noInstallLabel->setFont(labelFontNoInstall);
    noInstallLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    DFontSizeManager::instance()->bind(noInstallLabel, DFontSizeManager::T4);

    QVBoxLayout *lblLayoutNoInstall = new QVBoxLayout;
    lblLayoutNoInstall->addWidget(noInstallLabel);

    m_noInstallListView->setLayout(lblLayoutNoInstall);
    listViewVBoxLayout->addWidget(m_noInstallListView);

    m_noInstallListView->hide();
}

/*************************************************************************
 <Function>      initStateBar
 <Description>   初始化状态栏-包含预览输入框和字体大小调节滑块和字体大小标签
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::initStateBar()
{
    Q_D(DFontMgrMainWindow);

    QHBoxLayout *stateBarLayout = new QHBoxLayout();
    stateBarLayout->setContentsMargins(0, 0, 0, 0);
    stateBarLayout->setSpacing(0);

    d->stateBar = new QWidget(this);
    //d->stateBar->setFrameShape(DFrame::NoFrame);
    d->stateBar->setFixedHeight(FTM_SBAR_HEIGHT);
    d->stateBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    d->textInputEdit = new DLineEdit(this);
    //    QFont searchFont;
    //    searchFont.setPixelSize(14);
    //    d->textInputEdit->setFont(searchFont);
    //d->textInputEdit->setMinimumSize(QSize(FTM_SBAR_TXT_EDIT_W,FTM_SBAR_TXT_EDIT_H));
    DFontSizeManager::instance()->bind(d->textInputEdit, DFontSizeManager::T6);
    d->textInputEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    d->textInputEdit->setClearButtonEnabled(true);
    d->textInputEdit->lineEdit()->setPlaceholderText(DApplication::translate("StateBar", "Input preview text"));

    d->fontScaleSlider = new DSlider(Qt::Orientation::Horizontal, this);
    d->fontScaleSlider->setFixedSize(FTM_SBAR_SLIDER_W, FTM_SBAR_SLIDER_H);
    // d->fontScaleSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    //    d->fontScaleSlider->setTracking(true);
    //    d->fontScaleSlider->setTickPosition(QSlider::NoTicks);
    //    d->fontScaleSlider->setRange(MIN_FONT_SIZE, MAX_FONT_SIZE);
    d->fontScaleSlider->setMinimum(MIN_FONT_SIZE);
    d->fontScaleSlider->setMaximum(MAX_FONT_SIZE);
    //设置初始显示字体大小
    d->fontScaleSlider->setValue(DEFAULT_FONT_SIZE);

    d->fontSizeLabel = new DLabel(this);
    QFont fontScaleFont;
    fontScaleFont.setPixelSize(14);
    d->fontSizeLabel->setFont(fontScaleFont);
    d->fontSizeLabel->setFixedSize(FTM_SBAR_FSIZE_LABEL_W, FTM_SBAR_FSIZE_LABEL_H);
    d->fontSizeLabel->setAlignment(Qt::AlignCenter | Qt::AlignVCenter);

    //    QFont fontSize;
    //    fontSize.setPixelSize(14);
    //    d->fontSizeLabel->setFont(fontSize);
    DFontSizeManager::instance()->bind(d->fontSizeLabel, DFontSizeManager::T6);
    // d->fontSizeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    // Init the default font size
    QString defaultFontSize;
    defaultFontSize.sprintf(FMT_FONT_SIZE, DEFAULT_FONT_SIZE);

    //调节右下角字体大小显示label显示内容/*UT000539*/
    autoLabelWidth(defaultFontSize, d->fontSizeLabel, d->fontSizeLabel->fontMetrics());
    //    d->fontSizeLabel->setText(defaultFontSize);

    stateBarLayout->addSpacing(10);
    stateBarLayout->addWidget(d->textInputEdit, 1);
    stateBarLayout->addSpacing(20);
    stateBarLayout->addWidget(d->fontScaleSlider);
    stateBarLayout->addSpacing(10);
    stateBarLayout->addWidget(d->fontSizeLabel);
    stateBarLayout->addSpacing(20);

    d->stateBar->setLayout(stateBarLayout);

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    d->stateBar->setStyleSheet("background: green");
    d->textInputEdit->setStyleSheet("background: blue");
    d->fontScaleSlider->setStyleSheet("background: yellow");
    d->fontSizeLabel->setStyleSheet("background: yellow");
#endif
}

/*************************************************************************
 <Function>      handleAddFontEvent
 <Description>   添加字体文管页面处理函数
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::handleAddFontEvent()
{
    Q_D(DFontMgrMainWindow);
    //SP3--添加字体按钮取消安装后恢复选中状态--记录选中状态
    bool hasTabFocus = d->addFontButton->hasFocus();
    if (hasTabFocus)
        titlebar()->setFocus(Qt::TabFocusReason);
    DFileDialog dialog;
    dialog.setFileMode(DFileDialog::ExistingFiles);
    dialog.setNameFilter(Utils::suffixList());

    QString historyDir = d->settingsQsPtr->value("dir").toString();
    if (historyDir.isEmpty()) {
        historyDir = QDir::homePath();
    }
    if (!mhistoryDir.isEmpty()) {
        dialog.setDirectory(mhistoryDir);
    } else {
        dialog.setDirectory(historyDir);
    }

//    m_fontPreviewListView->refreshFocuses();
    const int mode = dialog.exec();

    // save the directory string to config file.
    d->settingsQsPtr->setValue("dir", dialog.directoryUrl().toLocalFile());

    // if click cancel button or close button.
    if (mode != QDialog::Accepted) {
        //SP3--添加字体按钮取消安装后恢复选中状态
        if (hasTabFocus) {
            QTimer::singleShot(10, [ = ] {
                d->addFontButton->setFocus(Qt::TabFocusReason);
            });
        }
        return;
    }

    QStringList filelist = dialog.selectedFiles();
    if (filelist.count() > 0) {
        mhistoryDir.clear();
        QStringList strlist;
        strlist = filelist.at(0).split("/");
        for (int i = 0; i < strlist.count(); i++) {
            if (i == 0) {
                mhistoryDir += strlist[i];
            } else  if (i == strlist.count() - 1) {

            } else {
                mhistoryDir += "/" + strlist[i];
            }
        }
    }

    m_previewText = d->textInputEdit->text();
    Q_EMIT fileSelected(filelist, hasTabFocus);
}

/*************************************************************************
 <Function>      handleMenuEvent
 <Description>   右键菜单弹出处理函数
 <Author>
 <Input>
    <param1>     action          Description:传参为菜单动作选项
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::handleMenuEvent(QAction *action)
{
    if (action->data().isValid()) {
        m_hasMenuTriggered = true;
        bool ok = false;
        int type = action->data().toInt(&ok);

        if (ok) {
            DFontMenuManager::MenuAction actionId = static_cast<DFontMenuManager::MenuAction>(type);

            // Add menu handler code here
            switch (actionId) {
            case DFontMenuManager::MenuAction::M_AddFont: {
                handleAddFontEvent();
            }
            break;
            case DFontMenuManager::MenuAction::M_FontInfo: {
                m_fontPreviewListView->setIsTabFocus(false);
                DFontPreviewItemData currItemData = m_fontPreviewListView->currModelData();
                DFontInfoDialog *fontInfoDlg = new DFontInfoDialog(&currItemData, this);
                fontInfoDlg->exec();
            }
            break;
            case DFontMenuManager::MenuAction::M_DeleteFont: {
                delCurrentFont();
            }
            break;
            case DFontMenuManager::MenuAction::M_ExportFont: {
                exportFont();
                m_fontPreviewListView->syncTabStatus();
            }
            break;
            case DFontMenuManager::MenuAction::M_EnableOrDisable: {
                m_fontPreviewListView->onEnableBtnClicked(m_menuDisableIndexList, m_menuSysCnt, m_menuCurCnt, !m_menuCurData.fontData.isEnabled(),
                                                          filterGroup == DSplitListWidget::FontGroup::ActiveFont);
                m_fontPreviewListView->syncTabStatus();
            }
            break;
            case DFontMenuManager::MenuAction::M_Faverator: {
                m_fontPreviewListView->onCollectBtnClicked(m_menuAllIndexList, !m_menuCurData.fontData.isCollected(),
                                                           filterGroup == DSplitListWidget::FontGroup::CollectFont);
                m_fontPreviewListView->syncTabStatus();
            }
            break;
            case DFontMenuManager::MenuAction::M_ShowFontPostion:
                m_fontPreviewListView->setIsTabFocus(false);
                showFontFilePostion();
                break;
            default:
                qDebug() << "handleMenuEvent->(id=" << actionId << ")";
                break;
            }
        }
    }
}

/*************************************************************************
 <Function>      installFont
 <Description>   安装字体入口函数
 <Author>
 <Input>
    <param1>     files           Description:待安装字体文件列表
 <Return>        bool            Description:return true 安装成功；return false 安装失败
 <Note>          null
*************************************************************************/
bool DFontMgrMainWindow::installFont(const QStringList &files, bool isAddBtnHasTabs)
{
    PerformanceMonitor::installFontStart();

    QStringList installFiles = checkFilesSpace(files);
    if (installFiles.count() == 0) {
        onShowMessage(0);
        return false;
    }

    if (m_fIsInstalling) {
        qDebug() << "Already exist a installtion flow";
        return false;
    }

    m_cacheFinish = false;
    m_installFinish = false;

    //m_fontPreviewListView->clearSelection();//取消安装不清空选中状态
    qDebug() << "installFont new DFInstallNormalWindow " << installFiles.size() << endl;
    m_installTm = QDateTime::currentMSecsSinceEpoch();
    m_dfNormalInstalldlg = new DFInstallNormalWindow(installFiles, this);
    connect(m_dfNormalInstalldlg, &DFInstallNormalWindow::destroyed, this, &DFontMgrMainWindow::onInstallWindowDestroyed);

    if (isAddBtnHasTabs)
        m_dfNormalInstalldlg->setAddBtnHasTabs(true);
    emit m_signalManager->setSpliteWidgetScrollEnable(true);//开始安装
    if (m_isQuickMode) {
        m_dfNormalInstalldlg->setSkipException(true);
    }

    //Set installtion flag
    /*
     * Add font from + ,menu, drag file to main view
     * to task bar can start a installtion flow, so must
     * to set flag avoid
     */
    m_fIsInstalling = true;
    m_installFontCnt = 0;
    Dtk::Widget::moveToCenter(m_dfNormalInstalldlg);
    m_dfNormalInstalldlg->exec();

    //m_dfNormalInstalldlg->setModal(true);

    //Clear installtion flag when NormalInstalltion window is closed
    return true;
}

/*************************************************************************
 <Function>      installFontFromSys
 <Description>   从系统中加载字体
 <Author>
 <Input>
    <param1>     files           Description:字体文件列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::installFontFromSys(const QStringList &files)
{
    this->m_isFromSys = true;

    QStringList reduceSameFiles;
    foreach (auto it, files) {
        if (!reduceSameFiles.contains(it)) {
            reduceSameFiles.append(it);
        }
    }

    if (!m_fontPreviewListView->isListDataLoadFinished()) {
        qDebug() << "Is loading ,quit";
        m_waitForInstall = reduceSameFiles;
        return;
    } else if (m_fIsDeleting) {
        qDebug() << "Is deleting ,quit";
        m_waitForInstall = reduceSameFiles;
        return;
    } else if (m_isPopInstallErrorDialog) {
        emit m_signalManager->installDuringPopErrorDialog(reduceSameFiles);
    } else {
        installFont(reduceSameFiles, false);
    }
}

/*************************************************************************
 <Function>      initRightKeyMenu
 <Description>   初始化右键菜单
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::initRightKeyMenu()
{
    Q_D(DFontMgrMainWindow);

    d->rightKeyMenu = DFontMenuManager::getInstance()->createRightKeyMenu();
}

/*************************************************************************
 <Function>      setQuickInstallMode
 <Description>   设置快速安装标志位
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::setQuickInstallMode(bool isQuick)
{
#ifdef QT_QML_DEBUG
    qDebug() << __FUNCTION__ << " isQuickMode=" << isQuick;
#endif
    m_isQuickMode = isQuick;
}

/*************************************************************************
 <Function>      hideQucikInstallWindow
 <Description>   隐藏快速安装页面
 <Author>        null
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::hideQucikInstallWindow()
{
    if (m_quickInstallWnd.get() != nullptr) {
        m_quickInstallWnd->setVisible(false);
    }
}

/*************************************************************************
 <Function>      InitQuickWindowIfNeeded
 <Description>   快速安装
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::InitQuickWindowIfNeeded()
{
    if (m_quickInstallWnd.get() == nullptr) {
        m_quickInstallWnd.reset(new DFQuickInstallWindow());

        // Quick install mode handle
        QObject::connect(this, &DFontMgrMainWindow::quickModeInstall, this,
        [this](const QStringList & files) {
            connect(m_quickInstallWnd.get(), &DFQuickInstallWindow::quickInstall, this,
            [this, files]() {
                this->installFont(files, false);
            });
            m_quickInstallWnd.get()->setWindowModality(Qt::WindowModal);
            m_quickInstallWnd->onFileSelected(files);
            m_quickInstallWnd->show();
            //Reative the window
            m_quickInstallWnd->raise();
            m_quickInstallWnd->activateWindow();

            Dtk::Widget::moveToCenter(m_quickInstallWnd.get());
        });
    }
}

/*************************************************************************
 <Function>      forceNoramlInstalltionQuitIfNeeded
 <Description>   安装进程被打断
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::forceNoramlInstalltionQuitIfNeeded()
{
    if (m_fIsInstalling) {
        qDebug() << "In normal installtion flow, force quit!";
        m_fontPreviewListView->setIsTabFocus(false);
        m_dfNormalInstalldlg->breakInstalltion();
    }
}

/*************************************************************************
 <Function>      setDeleteFinish
 <Description>   删除字体后恢复标志位
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::setDeleteFinish()
{
    m_fIsDeleting &= ~Delete_Deleting;
    qDebug() << __FUNCTION__ << m_fIsDeleting;
}

/*************************************************************************
 <Function>      cancelDelete
 <Description>   删除字体验证页面取消恢复标志位
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::cancelDelete()
{
    m_fIsDeleting = UnDeleting;
}

/*************************************************************************
 <Function>      onSearchTextChanged
 <Description>   根据搜索框输入内容刷新搜索结果
 <Author>
 <Input>
    <param1>     currStr         Description:搜索关键词
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::onSearchTextChanged(const QString &currStr)
{
    Q_D(DFontMgrMainWindow);
    if (!m_fontPreviewListView->isListDataLoadFinished()) {
        return;
    }

    //QString strSearchFontName = currStr;
    const QString strSearchFontName = currStr;
    qDebug() << "SearchFontName:" << strSearchFontName << endl;

    m_searchTextStatusIsEmpty = strSearchFontName.isEmpty();

    DFontPreviewProxyModel *filterModel = m_fontPreviewListView->getFontPreviewProxyModel();

    //根据搜索框内容实时过滤列表
    filterModel->setFilterKeyColumn(0);
    filterModel->setFilterFontNamePattern(strSearchFontName);
    //filterModel->setEditStatus(m_searchTextStatusIsEmpty);

    qDebug() << __FUNCTION__ << "filter Count:" << filterModel->rowCount() << endl;
    //记录搜索输入框焦点状态
    bool seachEditHasFocus = d->searchFontEdit->lineEdit()->hasFocus();
    onFontListViewRowCountChanged();
    //在字体列表为空时且清空搜索内容时会设置焦点在左侧菜单，此时焦点应保持在搜索栏,此操作旨在恢复输入框焦点状态,
    if (seachEditHasFocus)
        d->searchFontEdit->lineEdit()->setFocus(Qt::TabFocusReason);
    onPreviewTextChanged();
    m_fontPreviewListView->scrollToTop();
}

/*************************************************************************
 <Function>      onPreviewTextChanged
 <Description>   刷新预览内容
 <Author>
 <Input>
    <param1>     text            Description:预览内容
 <Return>        null            Description:null
 <Note>          onPreviewTextChanged()为调用函数更新预览内容
*************************************************************************/
void DFontMgrMainWindow::onPreviewTextChanged(const QString &text)
{
    qDebug() << __FUNCTION__ << text;
    m_previewText = text;

    onPreviewTextChanged();
}

/*************************************************************************
 <Function>      onFontSizeChanged
 <Description>   更新预览页面字号大小
 <Author>
 <Input>
    <param1>     fontSize        Description:预览字号大小
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::onFontSizeChanged(int fontSize)
{
    //Q_EMIT m_signalManager->refreshCurRect();
    if (!m_fontPreviewListView->isListDataLoadFinished()) {
        return;
    }

    DFontPreviewProxyModel *filterModel = m_fontPreviewListView->getFontPreviewProxyModel();
    qDebug() << __FUNCTION__ << "filter Count:" << filterModel->rowCount() << endl;

    for (int rowIndex = 0; rowIndex < filterModel->rowCount(); rowIndex++) {
        QModelIndex modelIndex = filterModel->index(rowIndex, 0);
        filterModel->setData(modelIndex, QVariant(fontSize), DFontPreviewItemDelegate::FontSizeRole);
        //filterModel->setEditStatus(m_searchTextStatusIsEmpty);
    }
    //Q_EMIT m_signalManager->prevFontChanged();
}

/*************************************************************************
 <Function>      showFontFilePostion
 <Description>   右键菜单：在文件管理器中显示字体文件位置
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::showFontFilePostion()
{
    DFontPreviewItemData currItemData = m_fontPreviewListView->currModelData();

    if (-1 != currItemData.strFontId && currItemData.fontInfo.filePath.length() > 0) {
        DDesktopServices::showFileItem(currItemData.fontInfo.filePath);
    }
}

/*************************************************************************
 <Function>      onLeftSiderBarItemClicked
 <Description>   左侧菜单项切换选项
 <Author>
 <Input>
    <param1>     index           Description:菜单项索引
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::onLeftSiderBarItemClicked(int index)
{
    if (!m_fontPreviewListView->isListDataLoadFinished()) {
        //save index to update
        m_leftIndex = static_cast<qint8>(index);
        return;
    }

    m_leftIndex = 0;

    qDebug() << __FUNCTION__ << index << endl;
    filterGroup = qvariant_cast<DSplitListWidget::FontGroup>(index);

    qDebug() << "filterGroup" << filterGroup << endl;

    DFontPreviewProxyModel *filterModel = m_fontPreviewListView->getFontPreviewProxyModel();
    filterModel->setFilterKeyColumn(0);
    filterModel->setFilterGroup(filterGroup);
    //filterModel->setEditStatus(m_searchTextStatusIsEmpty);

    qDebug() << __FUNCTION__ << "";
    onFontListViewRowCountChanged();
    onPreviewTextChanged();
    m_fontPreviewListView->clearSelection();
    if (m_fontPreviewListView->hasFocus() && m_fontPreviewListView->isVisible())
        m_fontPreviewListView->setFocus(Qt::MouseFocusReason);
}

/*************************************************************************
 <Function>      onUninstallFcCacheFinish
 <Description>   字体删除fc-cache操作后恢复标志位
 <Author>
 <Input>         null
 <Return>        null           Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::onUninstallFcCacheFinish()
{
    m_fIsDeleting &= ~Delete_Cacheing;
    qDebug() << __FUNCTION__ << m_fIsDeleting;
}

/*************************************************************************
 <Function>      onFontListViewRowCountChanged
 <Description>   切换字体菜单后判断FontListView的结果并显示对应状态
 <Author>
 <Input>
        dShow = 0 :查找到信息，显示正常
        dShow = 1 :未查到信息，显示“无搜索结果”
        dShow = 2 :未安装字体，显示“暂无字体”
        default   :默认有信息，显示正常
 <Return>        null           Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::onFontListViewRowCountChanged()
{
    Q_D(DFontMgrMainWindow);

    unsigned int bShow = 0;
    DFontPreviewProxyModel *filterModel = m_fontPreviewListView->getFontPreviewProxyModel();
    if (filterModel == nullptr)
        return;
    if (0 == filterModel->rowCount()) {
        if (m_searchTextStatusIsEmpty) {
            bShow = 2;
        } else {
            bShow = 1; //未找到字体
        }
    }
    bool isSpinnerHidden = m_fontLoadingSpinner->isHidden();
    switch (bShow) {
    case 0:
        if (isSpinnerHidden) {
            m_fontPreviewListView->show();
            m_noResultListView->hide();
            m_isNoResultViewShow = false;
            d->stateBar->show();
            if (m_noInstallListView->isVisible()) {
                m_noInstallListView->hide();
            }
        }
        return;
    case 1:
        if (!m_isNoResultViewShow) {
            if (!m_fIsInstalling) {
                m_fontPreviewListView->hide();
                //QTimer::singleShot(5, [ = ]() {
                m_noResultListView->show();
                m_isNoResultViewShow = true;
                //});
                d->stateBar->hide();
                if (m_noInstallListView->isVisible()) {
                    m_noInstallListView->hide();
                }
            }
            return;
        } else {
            return;
        }

    case 2:
        if (isSpinnerHidden) {
            m_fontPreviewListView->hide();
            d->stateBar->hide();
            if (m_noResultListView->isVisible()) {
                m_noResultListView->hide();
                m_isNoResultViewShow = false;
            }
            d->leftSiderBar->setIsHalfWayFocus(true);
            d->leftSiderBar->setFocus();

            m_noInstallListView->show();
        }
        return;
    default:
        m_fontPreviewListView->show();
        m_noResultListView->hide();
        m_isNoResultViewShow = false;
        d->stateBar->show();
        break;
    }
}

/*************************************************************************
 <Function>      onLoadStatus
 <Description>   刷新加载状态
 <Author>
 <Input>
    <param1>     type           Description:0 正在加载1 完成加载
 <Return>        null           Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::onLoadStatus(int type)
{
    D_D(DFontMgrMainWindow);
    if (0 == type || 1 == type) {
        switch (type) {
        case 0:
            m_fontPreviewListView->hide();
            if (m_noResultListView->isVisible()) {
                m_noResultListView->hide();
            }
            m_fontLoadingSpinner->spinnerStop();
            m_fontLoadingSpinner->spinnerStart();
            m_fontLoadingSpinner->show();
            break;
        case 1:
            if (m_fontPreviewListView->isListDataLoadFinished()) {
                m_fontLoadingSpinner->hide();
                m_fontLoadingSpinner->spinnerStop();
            }
            if (m_leftIndex >= 0) {
                onLeftSiderBarItemClicked(m_leftIndex);
            }
            //弹出之前判断是否已有无结果view 539 31107
            if (!m_noInstallListView->isVisible())
                m_fontPreviewListView->show();
            //不是删除过程造成的安装等待，安装时也需要对相关标志为进行设置
            waitForInsert();
            //第一次打开软件，正在加载数据时，搜索框的内容不为空，为做此操作 ut000794
            if (m_openfirst) {
                if (!d->searchFontEdit->text().isEmpty()) {
                    emit d->searchFontEdit->textChanged(d->searchFontEdit->text());
                }
                m_openfirst = false;

                PerformanceMonitor::loadFontFinish();
            }
            m_fontPreviewListView->onFontChanged(qApp->font());
            break;
        default:
            break;
        }
    }
    qDebug() << __FUNCTION__ << type;
    //if (type == 1 && !m_fileList.isEmpty()) {
    //    showInstalledFiles(m_fileList);
    //}
}

/*************************************************************************
 <Function>      onShowMessage
 <Description>   安装字体提示信息
 <Author>
 <Input>
    <param1>     successCount   Description:字体安装个数
 <Return>        null           Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::onShowMessage(int successCount)
{
    QString message;

    if (successCount == 1) {
        message = DApplication::translate("DFontMgrMainWindow", "%1 font installed").arg(successCount);
    } else if (successCount > 1) {
        message = DApplication::translate("DFontMgrMainWindow", "%1 fonts installed").arg(successCount);
    }

    DMessageManager::instance()->sendMessage(this, QIcon("://ok.svg"), message);

    PerformanceMonitor::installFontFinish(successCount);

    qDebug() << __FUNCTION__ << " pop toast message " << message << " total (ms) :" << QDateTime::currentMSecsSinceEpoch() - m_installTm;
}

/*************************************************************************
 <Function>      onShowSpinner
 <Description>   显示或停止旋转进度图标
 <Author>
 <Input>
    <param1>     bShow          Description:判断显示还是停止的标志位
    <param2>     force          Description:函数类传参，判断是否需要重绘的标志位
    <param3>     style          Description:显示模式
 <Return>        null           Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::onShowSpinner(bool bShow, bool force, DFontSpinnerWidget::SpinnerStyles style)
{
    qDebug() << __FUNCTION__ << bShow << force << "begin";
    if (bShow) {
        showSpinner(/*DFontSpinnerWidget::Delete*/style, force);
    } else {
        m_fontLoadingSpinner->spinnerStop();
        m_fontLoadingSpinner->hide();

        m_isNoResultViewShow = false;
        onFontListViewRowCountChanged();
        onPreviewTextChanged();
    }
    qDebug() << __FUNCTION__ << bShow << "end";
}

void DFontMgrMainWindow::onInstallWindowDestroyed(QObject *)
{
    m_installFontCnt = DFontPreviewListDataThread::instance()->m_installFontCnt;
    qDebug() << __FUNCTION__ << m_installFontCnt << m_fIsInstalling;
    if (m_fIsInstalling) {
        if (m_installFontCnt > 0) {
            showSpinner(DFontSpinnerWidget::Load);

            m_fontManager->setType(DFontManager::DoCache);
            m_fontManager->start();
        } else {
            //无安装字体，重置安装标志
            m_installFinish = true;
        }
    }
    hideSpinner();

    qDebug() << __FUNCTION__ << "end";
}

/*************************************************************************
 <Function>      delCurrentFont
 <Description>   字体删除处理函数
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::delCurrentFont(bool activatedByRightmenu)
{
    qDebug() << __FUNCTION__ << m_fIsDeleting;
    m_fontPreviewListView->setFontViewHasFocus(m_fontPreviewListView->hasFocus());
    if (m_fIsDeleting > UnDeleting)
        return;
    m_fIsDeleting = Deleting;
    if (!activatedByRightmenu)
        m_fontPreviewListView->selectedFonts(m_menuCurData, &m_menuDelCnt, &m_menuDisableSysCnt,
                                             &m_menuSysCnt, &m_menuCurCnt, nullptr, &m_menuDelFontList);
    if (m_menuDelCnt < 1) {
        m_fIsDeleting = UnDeleting;
        m_fontPreviewListView->setUserFontInUseSelected(false);
        return;
    }

    DFDeleteDialog *confirmDelDlg = new DFDeleteDialog(this, m_menuDelCnt, m_menuSysCnt, m_menuCurCnt > 0, this);

    connect(confirmDelDlg, &DFDeleteDialog::accepted, this, [ = ]() {
        PerformanceMonitor::deleteFontStart();

        //记录移除前位置
        m_fontPreviewListView->markPositionBeforeRemoved();
        DFontPreviewItemData currItemData = m_fontPreviewListView->currModelData();
        qDebug() << "Confirm delete:" << currItemData.fontInfo.filePath
                 << " is system font:" << currItemData.fontInfo.isSystemFont;
        //force delete all fonts
        //disable file system watcher
        onShowSpinner(true, false, DFontSpinnerWidget::Delete);
        Q_EMIT DFontPreviewListDataThread::instance(m_fontPreviewListView)->requestRemoveFileWatchers(m_menuDelFontList);
        DFontManager::instance()->setType(DFontManager::UnInstall);
        DFontManager::instance()->setUnInstallFile(m_menuDelFontList);
        DFontManager::instance()->start();
    });

    //confirmDelDlg->move((this->width() - confirmDelDlg->width() - 230 + mapToGlobal(QPoint(0, 0)).x()), (mapToGlobal(QPoint(0, 0)).y() + 180));
    confirmDelDlg->move(this->geometry().center() - confirmDelDlg->rect().center());
    confirmDelDlg->exec();
}

/*************************************************************************
 <Function>      exportFont
 <Description>   字体导出处理函数
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::exportFont()
{
    PerformanceMonitor::exportFontStart();
    QStringList exportFiles = checkFilesSpace(m_menuAllMinusSysFontList, false);
    if (exportFiles.count() == 0) {
        showExportFontMessage(0, m_menuAllMinusSysFontList.count());
        return;
    }

    Q_EMIT DFontPreviewListDataThread::instance()->requestExportFont(exportFiles);
}

/*************************************************************************
 <Function>      showExportFontMessage
 <Description>   显示导出字体时提示内容
 <Author>
 <Input>
    <param1>     successCount         Description:导出成功字体个数
    <param2>     abandonFilesCount    Description:导出失败字体个数
 <Return>        null                 Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::showExportFontMessage(int successCount, int abandonFilesCount)
{
    QString message;
    if (abandonFilesCount == 0) {
        if (successCount == 1) {
            message = DApplication::translate("Main", "The font exported to your desktop");
        } else {
            message = DApplication::translate("Main", "%1 fonts exported to your desktop").arg(successCount);
        }
        DMessageManager::instance()->sendMessage(this, QIcon("://ok.svg"), message);
    } else if (abandonFilesCount == 1) {
        message = DApplication::translate("Main", "Failed to export 1 font. There is not enough disk space.");
        DMessageManager::instance()->sendMessage(this, QIcon("://exception-logo.svg"), message);
    } else if (abandonFilesCount > 1) {
        message = DApplication::translate("Main", "Failed to export %1 fonts. There is not enough disk space.").arg(abandonFilesCount);
        DMessageManager::instance()->sendMessage(this, QIcon("://exception-logo.svg"), message);
    }
    qDebug() << __FUNCTION__ << " pop toast message " << message;
}

/*************************************************************************
 <Function>      dragEnterEvent
 <Description>   处理拖入事件
 <Author>
 <Input>
    <param1>     event          Description:事件对象
 <Return>        null           Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    //Q_D(DFontMgrMainWindow);

    if (event->mimeData()->hasUrls()) {
        QList<QUrl> dragFiles = event->mimeData()->urls();

        if (dragFiles.size() == 1) {
            //For one-drag check MIME,ignore non-font file
            if (Utils::isFontMimeType(dragFiles[0].path())) {
                event->accept();
                return;
            }
        } else {
            //Multi-drag just accept all file at start
            //will filter non-font files in drapEvent
            event->accept();
            return;
        }
    }
    qDebug() << __FUNCTION__ << "ignore";
    event->ignore();
}

/*************************************************************************
 <Function>      dropEvent
 <Description>   处理拖放事件
 <Author>
 <Input>
    <param1>     event          Description:事件对象
 <Return>        null           Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::dropEvent(QDropEvent *event)
{

    if (event->mimeData()->hasUrls()) {

        QStringList installFileList;

        QList<QUrl> dragFiles = event->mimeData()->urls();

        if (dragFiles.size() > 1) {
            foreach (auto it, event->mimeData()->urls()) {
                if (Utils::isFontMimeType(it.path())) {
                    installFileList.append(it.path());
                }
            }
        } else {
            if (Utils::isFontMimeType(dragFiles[0].path())) {
                installFileList.append(dragFiles[0].path());
            }
        }

        //Check if need to trigger installtion
        if (installFileList.size() > 0) {
            event->accept();
            Q_EMIT fileSelected(installFileList, false);
        } else {
            event->ignore();
        }
    } else {
        event->ignore();
    }
}

/*************************************************************************
 <Function>      resizeEvent
 <Description>   处理窗口大小变化事件
 <Author>
 <Input>
    <param1>     event          Description:事件对象
 <Return>        null           Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::resizeEvent(QResizeEvent *event)
{
//    Q_UNUSED(event)

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenRect =  screen->availableVirtualGeometry();

    if (this->rect() == screenRect) {
        setWindowState(Qt::WindowMaximized);
    }

    if (!windowState().testFlag(Qt::WindowFullScreen) && !windowState().testFlag(Qt::WindowMaximized)) {
        m_winHight = static_cast<short>(geometry().height());
        m_winWidth = static_cast<short>(geometry().width());
        m_IsWindowMax = false;
    } else {
        m_IsWindowMax = true;
    }

    DMainWindow::resizeEvent(event);
}

/*************************************************************************
 <Function>      showAllShortcut
 <Description>   调用显示快捷键的说明页面
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::showAllShortcut()
{
    QRect rect = window()->geometry();
    QPoint pos(rect.x() + rect.width() / 2,
               rect.y() + rect.height() / 2);

    QJsonObject shortcutObj;
    QJsonArray jsonGroups;

    QMap<QString, QString> shortcutKeymap = {
        {DApplication::translate("Shortcut", "Help"), "F1"},
        //        {"Zoom in",            "Ctrl+-"},
        //        {"Zoom out",           "Ctrl++"},
        //        {"Reset font",         "Ctrl+0"},
        //        {"Close window",       "Alt+F4"},
        {DApplication::translate("Shortcut", "Display shortcuts"),  "Ctrl+Shift+?"},
        {DApplication::translate("Shortcut", "Page up"), "PageUp"},
        {DApplication::translate("Shortcut", "Page down"), "PageDown"},
        //        {"Resize window",      "Ctrl+Alt+F"},
        //        {"Find",               "Ctrl+F"},
        {DApplication::translate("Shortcut", "Delete"), "Delete"},
        {DApplication::translate("Shortcut", "Add font"), "Ctrl+O"},
        {DApplication::translate("Shortcut", "Favorite"), "."},
        {DApplication::translate("Shortcut", "Unfavorite"), "."},
        {DApplication::translate("Shortcut", "Font info"), "Ctrl+I"},
    };

    QJsonObject fontMgrJsonGroup;
    fontMgrJsonGroup.insert("groupName", DApplication::translate("Main", "Font Manager"));
    QJsonArray fontJsonItems;

    for (QMap<QString, QString>::iterator it = shortcutKeymap.begin();
            it != shortcutKeymap.end(); ++it) {
        QJsonObject jsonItem;
        jsonItem.insert("name", DApplication::translate("Shortcuts", it.key().toUtf8()));
        jsonItem.insert("value", it.value().replace("Meta", "Super"));
        fontJsonItems.append(jsonItem);
    }

    fontMgrJsonGroup.insert("groupItems", fontJsonItems);
    jsonGroups.append(fontMgrJsonGroup);

    shortcutObj.insert("shortcut", jsonGroups);

    QJsonDocument doc(shortcutObj);

    QStringList shortcutString;
    QString param1 = "-j=" + QString(doc.toJson().data());
    QString param2 = "-p=" + QString::number(pos.x()) + "," + QString::number(pos.y());
    shortcutString << param1 << param2;

    QProcess *shortcutViewProcess = new QProcess();
    shortcutViewProcess->startDetached("deepin-shortcut-viewer", shortcutString);

    connect(shortcutViewProcess, SIGNAL(finished(int)), shortcutViewProcess, SLOT(deleteLater()));
}

/*************************************************************************
 <Function>      showInstalledFiles
 <Description>   安装完成后切换至用户字体页面
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::showInstalledFiles()
{
    D_D(DFontMgrMainWindow);
    d->leftSiderBar->setCurrentIndex(d->leftSiderBar->model()->index(DSplitListWidget::UserFont, 0));
    onLeftSiderBarItemClicked(DSplitListWidget::UserFont);
    d->leftSiderBar->setLastPageNumber(DSplitListWidget::UserFont);
}

/*************************************************************************
 <Function>      showSpinner
 <Description>   通过styles来决定标签显示内容
 <Author>
 <Input>
    <param1>     styles         Description:标签枚举类型
    <param2>     force          Description:是否需要重绘的标志位
 <Return>        null           Description:
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::showSpinner(DFontSpinnerWidget::SpinnerStyles styles, bool force)
{
    D_D(DFontMgrMainWindow);
    qDebug() << __FUNCTION__ << styles << force  << QThread::currentThreadId();
    m_noInstallListView->hide();
    m_fontPreviewListView->hide();
    m_noResultListView->hide();
    d->stateBar->hide();

    m_fontLoadingSpinner->setStyles(styles);

    if (force) {
        m_fontLoadingSpinner->spinnerStart();
        m_fontLoadingSpinner->repaint();
        return;
    }
    m_fontLoadingSpinner->spinnerStart();
    m_fontLoadingSpinner->show();
}

/*************************************************************************
 <Function>      hideSpinner
 <Description>   判断是否需要隐藏旋转进度图标
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::hideSpinner()
{
    qDebug() << __FUNCTION__ << "m_cacheFinish : " << m_cacheFinish << m_installFinish;
    if (!m_cacheFinish || !m_installFinish || !m_fIsInstalling) {
        return;
    }

    m_fontLoadingSpinner->spinnerStop();
    m_fontLoadingSpinner->hide();
    m_isNoResultViewShow = false;

    //安装刷新完成后启用菜单滚动功能
    emit m_signalManager->setSpliteWidgetScrollEnable(false);

    qDebug() << __func__ << "install finish" << endl;
    m_fIsInstalling = false;

    //如果无新装字体，不做后续操作，直接return
    if (m_installFontCnt == 0)
        return;
    onShowMessage(m_installFontCnt);
    m_installFontCnt = 0;

    onFontListViewRowCountChanged();
    onPreviewTextChanged();
    //更新选中位置和设置滚动
    m_fontPreviewListView->scrollWithTheSelected();
    m_fontPreviewListView->refreshFocuses();
}

/*************************************************************************
 <Function>      waitForInsert
 <Description>   更新待安装列表
 <Author>        exa
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontMgrMainWindow::waitForInsert()
{
    if (m_waitForInstall.isEmpty())
        return;

    if (installFont(m_waitForInstall, false))
        m_waitForInstall.clear();
}

/*************************************************************************
 <Function>      onPreviewTextChanged
 <Description>   根据输入内容调整预览字体
 <Author>
 <Input>         Null
 <Return>        Null            Description:Null
 <Note>          Null
*************************************************************************/
void DFontMgrMainWindow::onPreviewTextChanged()
{
    if (!m_fontPreviewListView->isListDataLoadFinished()) {
        return;
    }

    DFontPreviewProxyModel *filterModel = m_fontPreviewListView->getFontPreviewProxyModel();
    int total = filterModel->rowCount();
    qDebug() << __FUNCTION__ << "filter Count:" << filterModel->rowCount() << endl;

    for (int rowIndex = 0; rowIndex < total; rowIndex++) {
        QModelIndex modelIndex = filterModel->index(rowIndex, 0);
        QString itemPreviewTxt = filterModel->data(modelIndex, DFontPreviewItemDelegate::FontPreviewRole).toString();
        if (m_previewText != itemPreviewTxt)
            filterModel->setData(modelIndex, QVariant(m_previewText), DFontPreviewItemDelegate::FontPreviewRole);
        if (m_previewFontSize != filterModel->data(modelIndex, DFontPreviewItemDelegate::FontSizeRole).toInt())
            filterModel->setData(modelIndex, QVariant(m_previewFontSize), DFontPreviewItemDelegate::FontSizeRole);
    }
}

/*************************************************************************
 <Function>      getDiskSpace
 <Description>   获得硬盘剩余空间
 <Author>        UT000442
 <Input>
    <param1>     m_bInstall     Description:标志位
 <Return>        qint64         Description:返回盈余空间大小
 <Note>          Null
*************************************************************************/
qint64 DFontMgrMainWindow::getDiskSpace(bool m_bInstall)
{
    QStorageInfo storage;
    if (m_bInstall) {
        storage = QStorageInfo(QDir::homePath());
        qDebug() << __FUNCTION__ << "storage.bytesAvailable:" << storage.bytesAvailable();
    } else {
        QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        storage = QStorageInfo(desktopPath);
    }
    //不用转换直接用bytes更加准确
    qint64 m_remainSpace = storage.bytesAvailable()/*/1000/1000*/;
    return m_remainSpace;
}

/*************************************************************************
 <Function>      checkFilesSpace
 <Description>   检查空间盈余
 <Author>        UT000442
 <Input>
    <param1>     files          Description:文件路径列表
    <param2>     m_bInstall     Description:标志位
 <Return>        QStringList    Description:返回路径列表
 <Note>          Null
*************************************************************************/
QStringList DFontMgrMainWindow::checkFilesSpace(const QStringList &files, bool m_bInstall)
{
    if (files.isEmpty())
        return files;

    qint64 totalSelectSpace = 0;
    qint64 currentDiskSpace = getDiskSpace(m_bInstall);

    QMap<qint64, QString> map;
    for (const QString &file : files) {
        QFileInfo fileInfo(file);
        totalSelectSpace = totalSelectSpace + fileInfo.size();
        if (totalSelectSpace >= currentDiskSpace) {
            break;
        } else {
            map.insertMulti(fileInfo.size(), file);
        }
    }

    qDebug() << currentDiskSpace << totalSelectSpace << endl;

    return map.values();
}

/**
*  @brief  对主窗口中的focusout事件进行检查后对相关控件标志位等进行处理
*  @param[in]  触发事件的控件
*  @param[in]  触发事件
*/
void DFontMgrMainWindow::mainwindowFocusOutCheck(QObject *obj, QEvent *event)
{
    D_D(DFontMgrMainWindow);

    // bug 39973 焦点因为窗口激活原因移出去之前判断左侧listview的状态，如果为Tabfocus的状态记录下此时的各个状态留
    //到焦点移回来之后给左侧listview重新设置状态。
    if (obj == d->leftSiderBar) {
        QFocusEvent *focusEvent = dynamic_cast<QFocusEvent *>(event);
        if (focusEvent->reason() == Qt::ActiveWindowFocusReason) {
            m_leftListViewTabFocus =  d->leftSiderBar->IsTabFocus();
            if (m_leftListViewTabFocus) {
                m_currentStatus =  d->leftSiderBar->getStatus();
            }
        }
    }

    if (obj == m_fontPreviewListView) {
        QFocusEvent *focusEvent = dynamic_cast<QFocusEvent *>(event);
        if (focusEvent->reason() == Qt::ActiveWindowFocusReason) {
            m_previewListViewTabFocus  = m_fontPreviewListView->getIsTabFocus();
        }
    }

    if (obj == d->fontScaleSlider)
        m_fontPreviewListView->setIsGetFocusFromSlider(true);
}

/**
*  @brief  对主窗口中的focusin事件进行检查后对相关控件标志位等进行处理
*  @param[in]  触发事件的控件
*  @param[in]  触发事件
*/
void DFontMgrMainWindow::mainwindowFocusInCheck(QObject *obj, QEvent *event)
{
    D_D(DFontMgrMainWindow);

    //bug 39973 左侧listview因为窗口激活原因获取焦点时，根据之前获取的状态进行判断，并重新给listview
    //设置状态。
    if (obj == d->leftSiderBar) {
        QFocusEvent *focusEvent = dynamic_cast<QFocusEvent *>(event);
        if (focusEvent->reason() == Qt::ActiveWindowFocusReason) {
            if (m_leftListViewTabFocus) {
                d->leftSiderBar->setCurrentStatus(m_currentStatus);
                m_leftListViewTabFocus = false;
            } else {
                d->leftSiderBar->setIsHalfWayFocus(true);
            }
        }
    } else {
        emit m_signalManager->setLostFocusState(false);
    }

    if (obj == m_fontPreviewListView) {
        QFocusEvent *focusEvent = dynamic_cast<QFocusEvent *>(event);
        if (focusEvent->reason() == Qt::ActiveWindowFocusReason) {
            m_fontPreviewListView->syncTabStatus(false);
//            else {
//                m_fontPreviewListView->setIsTabFocus(m_previewListViewTabFocus);
//            }
        }
    } else {
        m_fontPreviewListView->setIsGetFocusFromSlider(false);
    }

    //设置searchEdit的tab方式
    if (obj == d->searchFontEdit->lineEdit()) {
        d->searchFontEdit->lineEdit()->setFocusPolicy(Qt::StrongFocus);
        m_isSearchLineEditMenuPoped  = false;
    }

    if (obj == d->textInputEdit->lineEdit())
        m_isInputLineEditMunuPoped  = false;
}

/*************************************************************************
 <Function>      autoLabelWidth
 <Description>   调节右下角字体大小显示label显示内容
 <Author>        UT000539
 <Input>
    <param1>     text           Description:文本字符串
    <param2>     lab            Description:label对象
    <param3>     fm             Description:当前字体信息
 <Return>        Null           Description:Null
 <Note>          Null
*************************************************************************/
void DFontMgrMainWindow::autoLabelWidth(QString text, DLabel *lab, QFontMetrics fm)
{
    QString str = text;
    if (fm.width(text) <= 65) {
        lab->setFixedWidth(65);
    } else if (fm.width(text) > 45) {
        lab->setFixedWidth(80);
        for (int i = 0; i < text.size(); i++) {
            str = str.left(str.length() - 1);
            if (str.length() == 1) {
                break;
            }
            if (fm.width(str) < 80) {
                break;
            }
        }
    }
    lab->setText(str);
}

/*************************************************************************
 <Function>
Event
 <Description>   处理键盘press事件
 <Author>        UT000539
 <Input>
    <param1>     event          Description:事件对象
 <Return>        Null           Description:Null
 <Note>          Null
*************************************************************************/\
void DFontMgrMainWindow::keyPressEvent(QKeyEvent *event)
{
    //UT000539 判断slider是否聚焦，调整预览字体大小
    D_D(DFontMgrMainWindow);

    qDebug() << event->key() << endl;
    if (Qt::Key_Left == event->key() || Qt::Key_Down == event->key()) {
        if (d->fontScaleSlider->hasFocus()) {
            d->fontScaleSlider->setValue(d->fontScaleSlider->value() - 1);
        } else if (Qt::Key_Left == event->key() && (m_fontPreviewListView->hasFocus() || m_noInstallListView->hasFocus())) {
            emit m_signalManager->setLostFocusState(false);
            d->leftSiderBar->setFocus(Qt::MouseFocusReason);
        }
    }
    if (Qt::Key_Right == event->key() || Qt::Key_Up == event->key()) {
        if (d->fontScaleSlider->hasFocus()) {
            d->fontScaleSlider->setValue(d->fontScaleSlider->value() + 1);
            //焦点在leftSliderBar同时右侧listview有字体显示时,实现右键切换焦点至previewlistview,切换后选中首个 UT000539
        } else if (Qt::Key_Right == event->key() && d->leftSiderBar->hasFocus() && m_fontPreviewListView->isVisible()) {
            m_fontPreviewListView->setIsTabFocus(true);
            m_fontPreviewListView->setFocus(Qt::MouseFocusReason);
            m_fontPreviewListView->scrollWithTheSelected();
        }
    }

    DWidget::keyPressEvent(event);
}

/*************************************************************************
 <Function>      eventFilter
 <Description>   事件过滤器：用于过滤事件
 <Author>        UT000539
 <Input>
    <param1>     obj            Description:事件接收对象
    <param2>     event          Description:事件对象
 <Return>        bool           Description:返回事件处理结果
 <Note>          Null
*************************************************************************/
bool DFontMgrMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    D_D(DFontMgrMainWindow);
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Tab) {
            if (obj == d->searchFontEdit->lineEdit()) {
                setNextTabFocus(obj);
                //下个控件为titlebar时需要多执行一次keyPressEvent
                DWidget::keyPressEvent(keyEvent);
            }  else if (obj == d->textInputEdit->lineEdit()) {
                d->fontScaleSlider->setFocus(Qt::TabFocusReason);
                QWidget::keyPressEvent(keyEvent);
            } else {
                setNextTabFocus(obj);
            }
            return true;
        } else if (keyEvent->key() == Qt::Key_Down || keyEvent->key() == Qt::Key_Up
                   || keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right) {
            //添加字体按钮聚焦，屏蔽上下左右键
            if (obj == d->addFontButton)
                return true;
        } else if (keyEvent->key() == Qt::Key_Escape) {
            if (obj == d->textInputEdit->lineEdit())
                d->textInputEdit->lineEdit()->clear();
            if (obj == d->searchFontEdit->lineEdit())
                d->searchFontEdit->lineEdit()->clear();
        }
        return QWidget::eventFilter(obj, event);
    }

    if (event->type() == QEvent::FocusOut) {
        mainwindowFocusOutCheck(obj, event);
    }

    if (event->type() == QEvent::FocusIn) {
        mainwindowFocusInCheck(obj, event);
    }

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::RightButton) {
            //搜索输入框和预览输入框右击时，设置快捷键菜单弹出判断标志位为true，表示已有菜单弹出
            if (obj == d->searchFontEdit->lineEdit())
                m_isSearchLineEditMenuPoped = true;
            else if (obj == d->textInputEdit->lineEdit()) {
                m_isInputLineEditMunuPoped = true;
            }
            //屏蔽添加字体按钮的右键效果
            else if (obj == d->addFontButton) {
                return true;
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}

/*************************************************************************
 <Function>      setNextTabFocus
 <Description>   设置tab聚焦顺序
 <Author>        UT000539
 <Input>
    <param1>     obj            Description:接收对象
 <Return>        Null           Description:Null
 <Note>          Null
*************************************************************************/
void DFontMgrMainWindow::setNextTabFocus(QObject *obj)
{
    D_D(DFontMgrMainWindow);
    //因setTabOrder无法实现功能，所以手动设置顺序
    if (obj == d->addFontButton) {
        d->searchFontEdit->lineEdit()->setFocus(Qt::TabFocusReason);
    } else if (obj == d->searchFontEdit->lineEdit()) {
        titlebar()->setFocus(Qt::TabFocusReason);
        d->addFontButton->setFocusPolicy(Qt::NoFocus);
        d->searchFontEdit->lineEdit()->setFocusPolicy(Qt::ClickFocus);
    } else if (obj == m_fontPreviewListView) {
        d->textInputEdit->lineEdit()->setFocus(Qt::TabFocusReason);
    }/* else if (obj == d->textInputEdit->lineEdit()) {
        d->fontScaleSlider->setFocus(Qt::TabFocusReason);
    }*/ else if (obj == d->leftSiderBar) {
        //如果预览窗口不可见，则直接切换焦点至添加字体按钮
        if (m_fontPreviewListView->isVisible()) {
            m_fontPreviewListView->setIsTabFocus(true);
            //设置预览窗口焦点
            m_fontPreviewListView->scrollWithTheSelected();
            m_fontPreviewListView->setFocus(Qt::TabFocusReason);
        } else {
            d->addFontButton->setFocus(Qt::TabFocusReason);
        }
    } else if (obj == d->fontScaleSlider) {
        d->addFontButton->setFocus(Qt::TabFocusReason);
    }
    //如果点击设置了无字体页面焦点，则Tab切换至添加字体按钮
    else {
        d->addFontButton->setFocus(Qt::TabFocusReason);
    }
}

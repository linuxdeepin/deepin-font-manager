
#include "views/dfontmgrmainwindow.h"
#include "dfinstallnormalwindow.h"
#include "dsplitlistwidget.h"
#include "globaldef.h"
#include "interfaces/dfontmenumanager.h"
#include "utils.h"
#include "views/dfdeletedialog.h"
#include "views/dfontinfodialog.h"
#include "views/dfquickinstallwindow.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QResizeEvent>
#include <QSlider>
#include <QSplitter>

#include <DApplication>
#include <DFileDialog>
#include <DLineEdit>
#include <DListWidget>
#include <DLog>
#include <DSearchEdit>
#include <DSlider>
#include <DTitlebar>
#include <QMessageBox>

class DFontMgrMainWindowPrivate
{
public:
    DFontMgrMainWindowPrivate(DFontMgrMainWindow *q)
        : settingsQsPtr(new QSettings(QDir(Utils::getConfigPath()).filePath("config.conf"),
                                      QSettings::IniFormat))
        , q_ptr(q)
    {
    }

    //~DFontMgrMainWindowPrivate() {}

    QFrame *titleFrame {nullptr};

    QLabel *logoLabel {nullptr};

    QFrame *toolbar {nullptr};
    DPushButton *addFontButton {nullptr};
    DSearchEdit *searchFontEdit {nullptr};

    QFrame *fontShowArea {nullptr};

    QFrame *stateBar {nullptr};
    DLineEdit *textInputEdit {nullptr};
    DSlider *fontScaleSlider {nullptr};
    QLabel *fontSizeLabel {nullptr};

    QSplitter *mainWndSpliter {nullptr};
    QFrame *leftBarHolder {nullptr};
    QFrame *rightViewHolder {nullptr};

    // Menu
    QMenu *toolBarMenu {nullptr};
    QMenu *rightKeyMenu {nullptr};

    QScopedPointer<QSettings> settingsQsPtr;
    DFontMgrMainWindow *q_ptr;
    Q_DECLARE_PUBLIC(DFontMgrMainWindow)
};

DFontMgrMainWindow::DFontMgrMainWindow(bool isQuickMode, QWidget *parent)
    : DMainWindow(parent)
    , m_isQuickMode(isQuickMode)
    , d_ptr(new DFontMgrMainWindowPrivate(this))
{
    // setWindowFlags(windowFlags() | (Qt::FramelessWindowHint | Qt::WindowMaximizeButtonHint));

    initUI();
    initConnections();
}

DFontMgrMainWindow::~DFontMgrMainWindow() {}

void DFontMgrMainWindow::initData()
{
    D_D(DFontMgrMainWindow);
}
void DFontMgrMainWindow::initUI()
{
    setWindowRadius(18);  // debug
    initTileBar();
    initRightKeyMenu();
    initMainVeiws();
}
void DFontMgrMainWindow::initConnections()
{
    D_D(DFontMgrMainWindow);

    // Add Font button event
    QObject::connect(d->addFontButton, &DPushButton::clicked, this,
                     &DFontMgrMainWindow::handleAddFontEvent);

    QObject::connect(this, &DFontMgrMainWindow::fileSelected, this,
                     [this](const QStringList &files) { this->installFont(files); });
    // Menu event
    QObject::connect(d->toolBarMenu, &QMenu::triggered, this, &DFontMgrMainWindow::handleMenuEvent);

    // Right Key menu
    QObject::connect(d->rightKeyMenu, &QMenu::triggered, this,
                     &DFontMgrMainWindow::handleMenuEvent);
    // Initialize rigth menu it state
    QObject::connect(d->rightKeyMenu, &QMenu::aboutToShow, this, [=]() {
        DFontPreviewItemData currItemData = m_fontPreviewListView->currModelData();

        // Disable delete menu for system font
        QAction *delAction = DFontMenuManager::getInstance()->getActionByMenuAction(
            DFontMenuManager::M_DeleteFont, DFontMenuManager::MenuType::RightKeyMenu);

        if (nullptr != delAction && currItemData.pFontInfo->isSystemFont) {
            delAction->setDisabled(true);
        }
    });

    // State bar event
    QObject::connect(d->fontScaleSlider, &DSlider::valueChanged, this, [this, d](int value) {
        QString fontSizeText;
        fontSizeText.sprintf(FMT_FONT_SIZE, value);
        d->fontSizeLabel->setText(fontSizeText);

        onFontSizeChanged(value);
    });

    // Quick install mode handle
    QObject::connect(this, &DFontMgrMainWindow::quickModeInstall, this,
                     [this](const QStringList &files) {
                         DFQuickInstallWindow quickInstallDlg(files);
                         connect(&quickInstallDlg, &DDialog::accepted, this,
                                 [this, &files]() { this->installFont(files); });
                         quickInstallDlg.exec();
                     });

    // Search text changed
    QObject::connect(d->searchFontEdit, SIGNAL(textChanged(const QString &)), this,
                     SLOT(onSearchTextChanged(const QString &)));

    QObject::connect(d->textInputEdit, SIGNAL(textChanged(const QString &)), this,
                     SLOT(onPreviewTextChanged(const QString &)));
}

void DFontMgrMainWindow::initTileBar()
{
    D_D(DFontMgrMainWindow);

    initTileFrame();

    d->toolBarMenu = DFontMenuManager::getInstance()->createToolBarSettingsMenu();

    bool isDXcbPlatform = true;

    if (isDXcbPlatform) {
        // d->toolbar->getSettingsButton()->hide();
        titlebar()->setMenu(d->toolBarMenu);
        titlebar()->setContentsMargins(0, 0, 0, 0);

        titlebar()->setFixedHeight(FTM_TITLE_FIXED_HEIGHT);
        titlebar()->setCustomWidget(d->titleFrame, false);
    }
}

void DFontMgrMainWindow::initTileFrame()
{
    D_D(DFontMgrMainWindow);

    d->logoLabel = new QLabel();
    d->logoLabel->setObjectName("LogoLabel");
    d->logoLabel->setFixedSize(QSize(32, 32));
    d->logoLabel->setFocusPolicy(Qt::NoFocus);
    d->logoLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    d->logoLabel->setPixmap(QPixmap(":/images/deepin-font-manager.svg"));

    d->titleFrame = new QFrame(this);
    d->titleFrame->setObjectName("TitleBar");
    // d->titleFrame->setStyleSheet("background: yellow");  // debug
    d->titleFrame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

    initToolBar();

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setMargin(0);
    titleLayout->setSpacing(0);
    titleLayout->addSpacing(8);
    titleLayout->addWidget(d->logoLabel, 0, Qt::AlignLeft | Qt::AlignVCenter);
    titleLayout->addSpacing(14);
    titleLayout->addWidget(d->toolbar);
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    d->titleFrame->setLayout(titleLayout);

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    d->logoLabel->setStyleSheet("background: red");
    d->titleFrame->setStyleSheet("background: black");
#endif
}

void DFontMgrMainWindow::initToolBar()
{
    D_D(DFontMgrMainWindow);

    d->toolbar = new QFrame(this);
    d->toolbar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->toolbar->setObjectName("ToolBar");

    QHBoxLayout *toolBarLayout = new QHBoxLayout();

    // Add Font
    d->addFontButton = new DPushButton(this);
    d->addFontButton->setFixedSize(QSize(36, 36));
    //    d->addFontButton->setNormalPic(QString(":/images/add-font-normal.svg"));
    //    d->addFontButton->setHoverPic(QString(":/images/add-font-hover.svg"));
    //    d->addFontButton->setPressPic(QString(":/images/add-font-press.svg"));
    d->addFontButton->setIcon(QIcon(":/images/add-font-normal.svg"));
    d->addFontButton->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    d->searchFontEdit = new DSearchEdit();
    d->searchFontEdit->setFixedSize(QSize(FTM_SEARCH_BAR_W, FTM_SEARCH_BAR_H));
    d->searchFontEdit->setPlaceHolder(QString("搜索"));

    // toolBarLayout->addSpacing(12);
    toolBarLayout->setSpacing(0);
    toolBarLayout->addWidget(d->addFontButton, 0, Qt::AlignLeft | Qt::AlignVCenter);
    toolBarLayout->addSpacing(62);
    toolBarLayout->addStretch();
    toolBarLayout->addWidget(d->searchFontEdit);
    toolBarLayout->addStretch();
    toolBarLayout->setContentsMargins(0, 0, 0, 0);

    d->toolbar->setLayout(toolBarLayout);

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    d->toolbar->setStyleSheet("background: green");
    d->addFontButton->setStyleSheet("background: red");
    d->searchFontEdit->setStyleSheet("background: yellow");
#endif
}

void DFontMgrMainWindow::initMainVeiws()
{
    D_D(DFontMgrMainWindow);

    d->mainWndSpliter = new QSplitter(Qt::Horizontal, this);
    // For Debug
    d->mainWndSpliter->setStyleSheet("QSplitter::handle { background-color: red }");

    initLeftSideBar();
    initRightFontView();

    setCentralWidget(d->mainWndSpliter);
}

void DFontMgrMainWindow::initLeftSideBar()
{
    D_D(DFontMgrMainWindow);

    d->leftBarHolder = new QFrame(d->mainWndSpliter);
    d->leftBarHolder->setObjectName("leftMainLayoutHolder");
    d->leftBarHolder->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    d->leftBarHolder->setFixedWidth(FTM_LEFT_SIDE_BAR_WITHD);
    d->leftBarHolder->setContentsMargins(10, 0, 10, 0);
    // d->leftBarHolder->setAttribute(Qt::WA_TranslucentBackground, true);

    QVBoxLayout *leftMainLayout = new QVBoxLayout();
    leftMainLayout->setContentsMargins(0, 0, 0, 0);
    leftMainLayout->setSpacing(0);

    // ToDo:
    //    Need use the custom QListView replace QListWidget
    DSplitListWidget *leftSiderBar = new DSplitListWidget(this);
    // leftSiderBar->setAttribute(Qt::WA_TranslucentBackground, true);
    leftSiderBar->setFrameShape(QFrame::NoFrame);
    leftSiderBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    leftMainLayout->addWidget(leftSiderBar);
    d->leftBarHolder->setLayout(leftMainLayout);

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    d->leftBarHolder->setStyleSheet("background: blue");
    leftSiderBar->setStyleSheet("background: yellow");
#endif
}

void DFontMgrMainWindow::initRightFontView()
{
    Q_D(DFontMgrMainWindow);

    d->rightViewHolder = new QFrame(d->mainWndSpliter);
    d->rightViewHolder->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    d->rightViewHolder->setObjectName("rightMainLayoutHolder");

    QVBoxLayout *rightMainLayout = new QVBoxLayout();
    rightMainLayout->setContentsMargins(0, 0, 0, 0);
    rightMainLayout->setSpacing(0);

    d->fontShowArea = new QFrame(this);
    d->fontShowArea->setFrameShape(QFrame::NoFrame);
    d->fontShowArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    initFontPreviewListView(d->fontShowArea);

    // initialize state bar
    initStateBar();

    rightMainLayout->addWidget(d->fontShowArea);
    rightMainLayout->addWidget(d->stateBar);

    d->rightViewHolder->setLayout(rightMainLayout);

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    d->fontShowArea->setStyleSheet("background: blue");
    m_fontPreviewListView->setStyleSheet("background: green");
    d->rightViewHolder->setStyleSheet("background: red");
#endif
}

//初始化字体预览ListView
void DFontMgrMainWindow::initFontPreviewListView(QFrame *parent)
{
    Q_D(DFontMgrMainWindow);

    QVBoxLayout *listViewVBoxLayout = new QVBoxLayout();
    listViewVBoxLayout->setMargin(0);
    listViewVBoxLayout->setContentsMargins(0, 0, 0, 0);
    listViewVBoxLayout->setSpacing(0);
    parent->setLayout(listViewVBoxLayout);

    m_fontPreviewListView = new DFontPreviewListView;
    m_fontPreviewListView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_fontPreviewListView->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    m_fontPreviewListView->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

    m_fontPreviewListView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    m_fontPreviewListView->setRightContextMenu(d->rightKeyMenu);

    listViewVBoxLayout->addWidget(m_fontPreviewListView);
}

void DFontMgrMainWindow::initStateBar()
{
    Q_D(DFontMgrMainWindow);

    QHBoxLayout *stateBarLayout = new QHBoxLayout();
    stateBarLayout->setContentsMargins(0, 0, 0, 0);
    stateBarLayout->setSpacing(0);

    d->stateBar = new QFrame(this);
    d->stateBar->setFrameShape(QFrame::NoFrame);
    d->stateBar->setFixedHeight(FTM_SBAR_HEIGHT);
    d->stateBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    d->textInputEdit = new DLineEdit(this);
    d->textInputEdit->setFixedHeight(FTM_SBAR_TXT_EDIT_H);
    d->textInputEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    d->textInputEdit->setClearButtonEnabled(true);
    d->textInputEdit->setPlaceholderText("输入文本内容进行预览");

    d->fontScaleSlider = new DSlider(Qt::Orientation::Horizontal, this);
    d->fontScaleSlider->setFixedSize(FTM_SBAR_SLIDER_W, FTM_SBAR_SLIDER_H);
    // d->fontScaleSlider->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    d->fontScaleSlider->setTracking(true);
    d->fontScaleSlider->setTickPosition(QSlider::NoTicks);
    d->fontScaleSlider->setRange(MIN_FONT_SIZE, MAX_FONT_SIZE);
    //设置初始显示字体大小
    d->fontScaleSlider->setValue(DEFAULT_FONT_SIZE);

    d->fontSizeLabel = new QLabel(this);
    d->fontSizeLabel->setFixedSize(FTM_SBAR_FSIZE_LABEL_W, FTM_SBAR_FSIZE_LABEL_H);
    d->fontSizeLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    // d->fontSizeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    // Init the default font size
    QString defaultFontSize;
    defaultFontSize.sprintf(FMT_FONT_SIZE, DEFAULT_FONT_SIZE);
    d->fontSizeLabel->setText(defaultFontSize);

    stateBarLayout->setAlignment(Qt::AlignVCenter | Qt::AlignVCenter);
    stateBarLayout->addSpacing(10);
    stateBarLayout->addWidget(d->textInputEdit);
    stateBarLayout->addSpacing(20);
    stateBarLayout->addWidget(d->fontScaleSlider, 0, Qt::AlignVCenter);
    stateBarLayout->addSpacing(20);
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

void DFontMgrMainWindow::resizeEvent(QResizeEvent *event)
{
    Q_D(DFontMgrMainWindow);
    DMainWindow::resizeEvent(event);

    d->titleFrame->setFixedWidth(event->size().width() - titlebar()->buttonAreaWidth());

#ifdef QT_QML_DEBUG
    qDebug() << "Windiw resize(" << event->size().width() << ", " << event->size().height() << ") "
             << "Button AreaWtidth=" << titlebar()->buttonAreaWidth();
#endif
}
void DFontMgrMainWindow::handleAddFontEvent()
{
    Q_D(DFontMgrMainWindow);

    DFileDialog dialog;
    dialog.setFileMode(DFileDialog::ExistingFiles);
    dialog.setNameFilter(Utils::suffixList());

    QString historyDir = d->settingsQsPtr->value("dir").toString();
    if (historyDir.isEmpty()) {
        historyDir = QDir::homePath();
    }
    dialog.setDirectory(historyDir);

    const int mode = dialog.exec();

    // save the directory string to config file.
    d->settingsQsPtr->setValue("dir", dialog.directoryUrl().toLocalFile());

    // if click cancel button or close button.
    if (mode != QDialog::Accepted) {
        return;
    }

    Q_EMIT fileSelected(dialog.selectedFiles());
}

void DFontMgrMainWindow::handleMenuEvent(QAction *action)
{
    if (action->data().isValid()) {
        bool ok = false;
        int type = action->data().toInt(&ok);

        if (ok) {
            DFontMenuManager::MenuAction actionId = static_cast<DFontMenuManager::MenuAction>(type);

            // Add menu handler code here
            switch (actionId) {
            case DFontMenuManager::MenuAction::M_AddFont: {
                handleAddFontEvent();
            } break;
            case DFontMenuManager::MenuAction::M_ThemeDark:
                switchAppTheme(Theme::Dark);
                break;
            case DFontMenuManager::MenuAction::M_ThemeLight:
                switchAppTheme(Theme::Light);
                break;
            case DFontMenuManager::MenuAction::M_ThemeFollowSystem:
                switchAppTheme(Theme::FollowSystem);
                break;
            case DFontMenuManager::MenuAction::M_FontInfo: {
                DFontPreviewItemData currItemData = m_fontPreviewListView->currModelData();
                DFontInfoDialog fontInfoDlg(&currItemData);
                fontInfoDlg.exec();
            } break;
            case DFontMenuManager::MenuAction::M_DeleteFont: {
                DFDeleteDialog confirmDelDlg;
                connect(&confirmDelDlg, &DFDeleteDialog::accepted, this, [this]() {
                    // Add Delete font code Here
                    DFontPreviewItemData currItemData = m_fontPreviewListView->currModelData();
                    qDebug() << "Confirm delete:" << currItemData.pFontInfo->filePath
                             << " is system font:" << currItemData.pFontInfo->isSystemFont;
                });

                confirmDelDlg.exec();

            } break;
            case DFontMenuManager::MenuAction::M_ShowFontPostion:
                showFontFilePostion();
                break;
            case DFontMenuManager::MenuAction::M_Help: {
            } break;
            default:
                qDebug() << "handleMenuEvent->(id=" << actionId << ")";
            }
        }
    }
}

void DFontMgrMainWindow::switchAppTheme(int type)
{
    DApplication *app = qobject_cast<DApplication *>(qApp);

    if (app) {
        switch (type) {
        case Theme::Dark: {
            app->setTheme("dark");
        } break;
        case Theme::Light: {
            app->setTheme("light");
        } break;
        case Theme::FollowSystem: {
            // Not implementated
        } break;
        default:
            qDebug() << "Unknow Theme type = " << type;
        }
    }
}

void DFontMgrMainWindow::installFont(const QStringList &files)
{
    qDebug() << __FUNCTION__ << files;
    DFInstallNormalWindow dfNormalInstalldlg(files, this);
    if (m_isQuickMode) {
        dfNormalInstalldlg.setSkipException(true);
    }

    dfNormalInstalldlg.exec();
}

void DFontMgrMainWindow::initRightKeyMenu()
{
    Q_D(DFontMgrMainWindow);

    d->rightKeyMenu = DFontMenuManager::getInstance()->createRightKeyMenu();
}

void DFontMgrMainWindow::setQuickInstallMode(bool isQuick)
{
#ifdef QT_QML_DEBUG
    qDebug() << __FUNCTION__ << " isQuickMode=" << isQuick;
#endif
    m_isQuickMode = isQuick;
}

void DFontMgrMainWindow::onSearchTextChanged(const QString &currStr)
{
    Q_D(DFontMgrMainWindow);

    QString strSearchFontName = currStr;
    qDebug() << strSearchFontName << endl;

    QString strFontSize = d->fontSizeLabel->text();
    int iFontSize = strFontSize.remove("px").toInt();
    //    qDebug() << "Font size: " << strFontSize << "\t" << iFontSize << endl;

    QSortFilterProxyModel *filterModel = m_fontPreviewListView->getFontPreviewProxyModel();

    //根据搜索框内容实时过滤列表
    filterModel->setFilterKeyColumn(0);
    filterModel->setFilterRegExp(strSearchFontName);

    //    qDebug() << "filter Count:" << filterModel->rowCount() << endl;

    QString previewText = d->textInputEdit->text();
    onPreviewTextChanged(previewText);

    QString strPreviewText = d->textInputEdit->text();
    if (strPreviewText.length() > 0) {
        for (int rowIndex = 0; rowIndex < filterModel->rowCount(); rowIndex++) {
            QModelIndex modelIndex = filterModel->index(rowIndex, 0);
            filterModel->setData(modelIndex, QVariant(strPreviewText), Qt::UserRole + 1);
        }
    }
}

void DFontMgrMainWindow::onPreviewTextChanged(const QString &currStr)
{
    Q_D(DFontMgrMainWindow);

    QString previewText = currStr;
    if (0 == currStr.length()) {
        previewText = FTM_DEFAULT_PREVIEW_TEXT;
    }

    QString strFontSize = d->fontSizeLabel->text();
    int iFontSize = strFontSize.remove("px").toInt();
    //    qDebug() << "onPreviewTextChanged Font size: " << strFontSize << "\t" << iFontSize <<
    //    endl;

    QSortFilterProxyModel *filterModel = m_fontPreviewListView->getFontPreviewProxyModel();

    for (int rowIndex = 0; rowIndex < filterModel->rowCount(); rowIndex++) {
        QModelIndex modelIndex = filterModel->index(rowIndex, 0);
        filterModel->setData(modelIndex, QVariant(previewText), Qt::UserRole + 1);
        filterModel->setData(modelIndex, QVariant(iFontSize), Qt::UserRole + 2);
    }
}

void DFontMgrMainWindow::onFontSizeChanged(int fontSize)
{
    QSortFilterProxyModel *filterModel = m_fontPreviewListView->getFontPreviewProxyModel();

    for (int rowIndex = 0; rowIndex < filterModel->rowCount(); rowIndex++) {
        QModelIndex modelIndex = filterModel->index(rowIndex, 0);
        filterModel->setData(modelIndex, QVariant(fontSize), Qt::UserRole + 2);
    }
}

void DFontMgrMainWindow::showFontFilePostion()
{
    DFontPreviewItemData currItemData = m_fontPreviewListView->currModelData();

    if (nullptr != currItemData.pFontInfo) {
        QUrl url =
            QUrl::fromLocalFile(QFileInfo(currItemData.pFontInfo->filePath).dir().absolutePath());
        qDebug() << QUrl::fromLocalFile(currItemData.pFontInfo->filePath).toString();
        QUrlQuery query;
        query.addQueryItem("selectUrl",
                           QUrl::fromLocalFile(currItemData.pFontInfo->filePath).toString());
        url.setQuery(query);

        QProcess::startDetached(DEEPIN_FILE_MANAGE_NAME, QStringList(url.toString()));
    }
}

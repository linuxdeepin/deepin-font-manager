
#include "views/dfontmgrmainwindow.h"
#include "dfinstallnormalwindow.h"
#include "globaldef.h"
#include "interfaces/dfontmenumanager.h"
#include "utils.h"

#include <DSearchEdit>
#include <DSlider>
#include <DLineEdit>
#include <DLog>
#include <DTitlebar>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QResizeEvent>
#include <QSlider>
#include <QSplitter>
#include <DFileDialog>


#include <DListWidget>

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
    DImageButton *addFontButton {nullptr};
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

DFontMgrMainWindow::DFontMgrMainWindow(QWidget *parent)
    : DMainWindow(parent)
    , d_ptr(new DFontMgrMainWindowPrivate(this))
{
    setWindowFlags(windowFlags() | (Qt::FramelessWindowHint | Qt::WindowMaximizeButtonHint));

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
    initMainVeiws();
}
void DFontMgrMainWindow::initConnections()
{
    D_D(DFontMgrMainWindow);

    // Add Font button event
    QObject::connect(d->addFontButton, &DImageButton::clicked, this,
                     &DFontMgrMainWindow::handleAddFontEvent);

    QObject::connect(this, &DFontMgrMainWindow::fileSelected, this,
    [this](const QStringList & files) {
        this->installFont(files);
    });
    // Menu event
    QObject::connect(d->toolBarMenu, &QMenu::triggered, this, &DFontMgrMainWindow::handleMenuEvent);
    // State bar event
    QObject::connect(d->fontScaleSlider, &DSlider::valueChanged, this, [this, d](int value) {
        QString fontSizeText;
        fontSizeText.sprintf(FMT_FONT_SIZE, value);
        d->fontSizeLabel->setText(fontSizeText);
    });
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
        titlebar()->setCustomWidget(d->titleFrame, Qt::AlignLeft);
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

    QHBoxLayout *titleLayout = new QHBoxLayout(this);
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

    d->toolbar = new QFrame();
    d->toolbar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->toolbar->setObjectName("ToolBar");

    QHBoxLayout *toolBarLayout = new QHBoxLayout();

    // Add Font
    d->addFontButton = new DImageButton();
    d->addFontButton->setFixedSize(QSize(36, 36));
    d->addFontButton->setNormalPic(QString(":/images/deepin-font-manager.svg"));

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

    QVBoxLayout *leftMainLayout = new QVBoxLayout(this);
    leftMainLayout->setContentsMargins(0, 0, 0, 0);
    leftMainLayout->setSpacing(0);

    // ToDo:
    //    Need use the custom QListView replace QListWidget
    QListWidget *leftSiderBar = new QListWidget(this);
    // leftSiderBar->setAttribute(Qt::WA_TranslucentBackground, true);
    leftSiderBar->setFrameShape(QFrame::NoFrame);
    leftSiderBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    leftMainLayout->addWidget(leftSiderBar);

#if defined(FTM_DEBUG_DATA_ON)
    QListWidgetItem *item1 = new QListWidgetItem("所有字体");

    item1->setSizeHint(QSize(120, 40));
    item1->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QListWidgetItem *item2 = new QListWidgetItem("系统字体");
    item2->setSizeHint(QSize(120, 40));
    item2->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QListWidgetItem *item3 = new QListWidgetItem("系统字体");
    item3->setSizeHint(QSize(120, 40));
    item3->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QListWidgetItem *item4 = new QListWidgetItem("收藏字体");
    item4->setSizeHint(QSize(120, 40));
    item4->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QListWidgetItem *itemSperator = new QListWidgetItem();

    itemSperator->setSizeHint(QSize(120, 40));
    itemSperator->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    QListWidgetItem *item5 = new QListWidgetItem("中文");
    item5->setSizeHint(QSize(100, 40));
    item5->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    QListWidgetItem *item6 = new QListWidgetItem("等宽");
    item6->setSizeHint(QSize(100, 40));
    item6->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    leftSiderBar->addItem(item1);
    leftSiderBar->addItem(item2);
    leftSiderBar->addItem(item3);
    leftSiderBar->addItem(item4);
    leftSiderBar->addItem(itemSperator);
    leftSiderBar->addItem(item5);
    leftSiderBar->addItem(item6);
#endif

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

    QVBoxLayout *rightMainLayout = new QVBoxLayout(this);
    rightMainLayout->setContentsMargins(0, 0, 0, 0);
    rightMainLayout->setSpacing(0);

    d->fontShowArea = new QFrame(this);
    d->fontShowArea->setFrameShape(QFrame::NoFrame);
    d->fontShowArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // initialize state bar
    initStateBar();

    rightMainLayout->addWidget(d->fontShowArea);
    rightMainLayout->addWidget(d->stateBar);

    d->rightViewHolder->setLayout(rightMainLayout);

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    d->rightViewHolder->setStyleSheet("background: red");
#endif
}

void DFontMgrMainWindow::initStateBar()
{
    Q_D(DFontMgrMainWindow);

    QHBoxLayout *stateBarLayout = new QHBoxLayout(this);
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
            }
            break;
            default:
                qDebug() << "handleMenuEvent->(id=" << actionId << ")";
            }
        }
    }
}

void DFontMgrMainWindow::switchAppTheme(int type) {}

void DFontMgrMainWindow::installFont(const QStringList &files)
{
    DFInstallNormalWindow dlg(files, this);
    dlg.exec();
}

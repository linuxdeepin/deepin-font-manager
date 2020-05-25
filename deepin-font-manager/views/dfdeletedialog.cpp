#include "dfdeletedialog.h"
#include "utils.h"
#include "globaldef.h"
#include "dfontmgrmainwindow.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QScrollBar>

#include <DGuiApplicationHelper>
#include <DApplication>
#include <DApplicationHelper>
#include <DFrame>
#include <DFontSizeManager>
#include <DTipLabel>
#include <DScrollArea>

DFDeleteDialog::DFDeleteDialog(DFontMgrMainWindow *win, int deleteCnt, int systemCnt, QWidget *parent)
    : DFontBaseDialog(parent)
    , m_mainWindow(nullptr)
    , m_deleteCnt(deleteCnt)
    , m_systemCnt(systemCnt)
    , m_deleting(false)
{
    initUI();
    setMainwindow(win);
    initConnections();
    setTheme();
}

void DFDeleteDialog::setMainwindow(DFontMgrMainWindow *win)
{
    if (m_mainWindow != win && win != nullptr) {
        m_mainWindow = win;
    }
}

void DFDeleteDialog::initUI()
{
    setFixedWidth(DEFAULT_WINDOW_W);

    initMessageTitle();
    initMessageDetail();
    QLayout *buttonsLayout = initBottomButtons();

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(10, 0, 10, 10);
    mainLayout->addWidget(messageTitle, 0, Qt::AlignCenter);
    mainLayout->addSpacing(6);
    mainLayout->addWidget(messageDetail, 0, Qt::AlignCenter);
    mainLayout->addSpacing(16);
    mainLayout->addLayout(buttonsLayout);

    QWidget *mainFrame = new QWidget(this);
    mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    messageDetail->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainFrame->setLayout(mainLayout);

    //#000794 解决显示不全的问题
    messageTitle->setMinimumWidth(DEFAULT_WINDOW_W - 20);
    messageDetail->setMinimumWidth(DEFAULT_WINDOW_W - 20);

    addContent(mainFrame);
}

void DFDeleteDialog::initConnections()
{
    connect(m_cancelBtn, &DPushButton::clicked, this, [ = ]() {
        emit signal_delete(false);
        reject();
        close();
        emit m_signalManager->cancelDel();
    });
    connect(m_confirmBtn, &DPushButton::clicked, this, [ = ]() {
        emit signal_delete(true);
        if (m_deleting)
            return;
        m_deleting = true;
        accept();
        close();
    });

    connect(this, &DFDeleteDialog::closed, this, [ = ]() {
        if (m_deleting) {
            emit SignalManager::instance()->popUninstallDialog();//弹出删除进度框
        } else if (m_mainWindow != nullptr) {
            m_mainWindow->setDeleteFinish();
        }
    });
    connect(qApp, &DApplication::fontChanged, this, &DFDeleteDialog::onFontChanged);

    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, &DFDeleteDialog::setTheme);
}

void DFDeleteDialog::initMessageTitle()
{
    messageTitle = new DLabel(this);
    if (m_deleteCnt <= 1) {
        messageTitle->setText(DApplication::translate("DeleteConfirmDailog", "Are you sure you want to delete this font?"));
    } else {
        messageTitle->setText(DApplication::translate("DeleteConfirmDailog", "Are you sure you want to delete %1 fonts?").arg(m_deleteCnt));
    }
    /* Bug#21515 UT000591*/
    messageTitle->setFixedWidth(DEFAULT_WINDOW_W - 22);
    messageTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    messageTitle->setWordWrap(true);
    messageTitle->setAlignment(Qt::AlignCenter);

    DFontSizeManager::instance()->bind(messageTitle, DFontSizeManager::T6, QFont::Medium);

    DPalette pa = DApplicationHelper::instance()->palette(messageTitle);
    QColor color = pa.toolTipText().color();
    color.setAlphaF(0.9);
    pa.setColor(DPalette::WindowText, color);
    DApplicationHelper::instance()->setPalette(messageTitle, pa);

//    QFontMetrics fm(DFontSizeManager::instance()->t6());
}

void DFDeleteDialog::initMessageDetail()
{
    messageDetail = new DLabel(this);
    if (m_deleteCnt <= 1) {
        if (m_systemCnt == 0) {
            messageDetail->setText(DApplication::translate("DeleteConfirmDailog", "This font will not be available to applications"));
        } else {
            messageDetail->setText(DApplication::translate("DeleteConfirmDailog", "This font will not be available to applications, and the other %1 system fonts cannot be deleted").arg(m_systemCnt));
        }
    } else {
        if (m_systemCnt == 0) {
            messageDetail->setText(DApplication::translate("DeleteConfirmDailog", "These fonts will not be available to applications"));
        } else {
            messageDetail->setText(DApplication::translate("DeleteConfirmDailog", "These fonts will not be available to applications, and the other %1 system fonts cannot be deleted").arg(m_systemCnt));
        }
    }
    /* Bug#21515 UT000591*/
    messageDetail->setFixedWidth(DEFAULT_WINDOW_W - 22);
    messageDetail->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    messageDetail->setWordWrap(true);
    messageDetail->setAlignment(Qt::AlignCenter);

    DFontSizeManager::instance()->bind(messageDetail, DFontSizeManager::T6, QFont::Normal);
    DPalette pa = DApplicationHelper::instance()->palette(messageDetail);
    QColor color = pa.toolTipText().color();
    color.setAlphaF(0.7);
    pa.setColor(DPalette::WindowText, color);
    DApplicationHelper::instance()->setPalette(messageDetail, pa);
}

QLayout *DFDeleteDialog::initBottomButtons()
{
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    m_cancelBtn = new DPushButton(this);
    m_cancelBtn->setFixedSize(170, 36);
    m_cancelBtn->setText(DApplication::translate("DFDeleteDialog", "Cancel"));

    m_confirmBtn = new DWarningButton(this);
    m_confirmBtn->setFixedSize(170, 36);
    m_confirmBtn->setText(DApplication::translate("DFDeleteDialog", "Confirm"));

    DVerticalLine *verticalSplite = new DVerticalLine(this);
    DPalette pa = DApplicationHelper::instance()->palette(verticalSplite);
    QColor splitColor = pa.color(DPalette::ItemBackground);
    pa.setBrush(DPalette::Background, splitColor);
    verticalSplite->setPalette(pa);
    verticalSplite->setBackgroundRole(QPalette::Background);
    verticalSplite->setAutoFillBackground(true);
    verticalSplite->setFixedSize(3, 28);

    layout->addWidget(m_cancelBtn);
    layout->addSpacing(9);
    layout->addWidget(verticalSplite);
    layout->addSpacing(9);
    layout->addWidget(m_confirmBtn);

    return layout;
}

void DFDeleteDialog::onFontChanged(const QFont &font)
{
    Q_UNUSED(font);
    /* Bug#20953 #21069  UT000591 */
    messageDetail->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    resize(sizeHint());
}

void DFDeleteDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        reject();
        close();
    }
}

void DFDeleteDialog::setTheme()
{
    if (DApplicationHelper::DarkType == DApplicationHelper::instance()->themeType()) {
        DPalette pa = DApplicationHelper::instance()->palette(this);
        pa.setColor(DPalette::Background, QColor(25, 25, 25, 80));
        DApplicationHelper::instance()->setPalette(this, pa);
    } else {
        DPalette pa = DApplicationHelper::instance()->palette(this);
        pa.setColor(DPalette::Background, QColor(247, 247, 247, 80));
        DApplicationHelper::instance()->setPalette(this, pa);
    }
}

#include "dfdeletedialog.h"
#include "utils.h"
#include "globaldef.h"
#include "dfontmgrmainwindow.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>

#include <DApplication>
#include <DApplicationHelper>
#include <DFrame>
#include <DFontSizeManager>
#include <DTipLabel>

DFDeleteDialog::DFDeleteDialog(DFontMgrMainWindow *win, int deleteCnt, int systemCnt, QWidget *parent)
    : DFontBaseDialog(parent)
    , m_mainWindow(nullptr)
    , m_deleteCnt(deleteCnt)
    , m_systemCnt(systemCnt)
{
    initUI();
    setMainwindow(win);
    initConnections();
}

DFDeleteDialog::~DFDeleteDialog()
{
}

void DFDeleteDialog::setMainwindow(DFontMgrMainWindow *win)
{
    if (m_mainWindow != win && win != nullptr) {
        m_mainWindow = win;
        quitConn = connect(m_mainWindow, &DFontMgrMainWindow::requestDeleted, [ = ]() {
            if (isVisible()) {
                accept();
                close();
            }
        });
    }
}

void DFDeleteDialog::initUI()
{
    if (m_systemCnt <= 0) {
        setFixedSize(QSize(DEFAULT_WINDOW_W, DEFAULT_WINDOW_H));
    } else {
        setFixedSize(QSize(DEFAULT_WINDOW_W, DEFAULT_WINDOW_H + 40));
    }
    //setWindowOpacity(0.5);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(10, 0, 10, 10);

    QWidget *mainFrame = new QWidget(this);
    mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_messageA = new DLabel(this);
    if (m_deleteCnt <= 1) {
        m_messageA->setText(DApplication::translate("DeleteConfirmDailog", "Are you sure you want to delete this font?"));
    } else {
        m_messageA->setText(DApplication::translate("DeleteConfirmDailog", "Are you sure you want to delete %1 fonts").arg(m_deleteCnt));
//        m_messageA->setText(tr("Are you sure you want to delete %1 fonts").arg(m_deleteCnt));
    }
    m_messageA->setFixedHeight(20);
    m_messageA->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    DFontSizeManager::instance()->bind(m_messageA, DFontSizeManager::T6);

    m_messageB = new DLabel(this);
    if (m_deleteCnt <= 1) {
        m_messageB->setText(DApplication::translate("DeleteConfirmDailog", "This font will not be available to applications"));
    } else {
        m_messageB->setText(DApplication::translate("DeleteConfirmDailog", "These fonts will not be available to applications"));
//        m_messageB->setText(tr("These fonts will not be available to applications"));
    }
    m_messageB->setFixedHeight(20);
    m_messageB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QFont bfont = m_messageB->font();
    bfont.setWeight(QFont::Bold);
    m_messageB->setFont(bfont);
    DFontSizeManager::instance()->bind(m_messageB, DFontSizeManager::T6);
    //Set MessageB color to TextTips
    DPalette paMsgB = DApplicationHelper::instance()->palette(m_messageB);
    paMsgB.setBrush(DPalette::WindowText, paMsgB.color(DPalette::TextTips));
    m_messageB->setPalette(paMsgB);

    QHBoxLayout *actionBarLayout = new QHBoxLayout();
    actionBarLayout->setSpacing(0);
    actionBarLayout->setContentsMargins(0, 0, 0, 0);

    m_cancelBtn = new DPushButton(this);
    m_cancelBtn->setFixedHeight(38);
    m_cancelBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_cancelBtn->setText(DApplication::translate("DeleteConfirmDailog", "Cancel"));

    m_confirmBtn = new DWarningButton(this);
    m_confirmBtn->setFixedHeight(38);
    m_confirmBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_confirmBtn->setText(DApplication::translate("DeleteConfirmDailog", "Delete"));

    DVerticalLine *verticalSplite = new DVerticalLine(this);
    DPalette pa = DApplicationHelper::instance()->palette(verticalSplite);
    QColor splitColor = pa.color(DPalette::ItemBackground);
    pa.setBrush(DPalette::Background, splitColor);
    verticalSplite->setPalette(pa);
    verticalSplite->setBackgroundRole(QPalette::Background);
    verticalSplite->setAutoFillBackground(true);
    verticalSplite->setFixedSize(3, 28);

    actionBarLayout->addWidget(m_cancelBtn);
    actionBarLayout->addSpacing(8);
    actionBarLayout->addWidget(verticalSplite);
    actionBarLayout->addSpacing(8);
    actionBarLayout->addWidget(m_confirmBtn);

    mainLayout->addWidget(m_messageA, 0, Qt::AlignCenter);
    mainLayout->addSpacing(8);
    mainLayout->addWidget(m_messageB, 0, Qt::AlignCenter);
    if (m_systemCnt > 0) {
        DTipLabel *messageC = new DTipLabel;
        messageC->setFont(bfont);
        messageC->setText(tr("The other %2 system fonts cannot be deleted").arg(m_systemCnt));
        DFontSizeManager::instance()->bind(messageC, DFontSizeManager::T6);
        mainLayout->addSpacing(8);
        mainLayout->addWidget(messageC, 0, Qt::AlignCenter);
    }
    mainLayout->addStretch();
    mainLayout->addLayout(actionBarLayout);
    mainFrame->setLayout(mainLayout);

    addContent(mainFrame);
}

void DFDeleteDialog::initConnections()
{
    connect(m_cancelBtn, &DPushButton::clicked, this, [ = ]() {
        reject();
        close();
    });
    connect(m_confirmBtn, &DPushButton::clicked, this, [ = ]() { Q_EMIT requestDelete(); });
    connect(this, &DFDeleteDialog::closed, this, [ = ]() {
        disconnect(quitConn);
        if (m_mainWindow != nullptr)
            m_mainWindow->setDeleteFinish();
    });
}

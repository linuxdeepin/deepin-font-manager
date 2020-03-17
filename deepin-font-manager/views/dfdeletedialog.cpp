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
    , m_deleting(false)
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

void DFDeleteDialog::paintEvent(QPaintEvent *event)
{
    m_messageB->setMinimumHeight(m_messageB->fontMetrics().height() * 2);
}

void DFDeleteDialog::initUI()
{
    if (m_systemCnt <= 0) {
        resize(QSize(DEFAULT_WINDOW_W, DEFAULT_WINDOW_H));
    } else {
        resize(QSize(DEFAULT_WINDOW_W, DEFAULT_WINDOW_H + 40));
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
//    m_messageA->setFixedHeight(20);
    m_messageA->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    DFontSizeManager::instance()->bind(m_messageA, DFontSizeManager::T6, QFont::Medium);

    DPalette paA = DApplicationHelper::instance()->palette(m_messageA);
    //pa1 = m_messageA->palette();
    QColor color = paA.toolTipText().color();
    color.setAlphaF(0.9);
    paA.setColor(DPalette::WindowText, color);
    //pa1.setColor(DPalette::WindowText, "#000000");
    DApplicationHelper::instance()->setPalette(m_messageA, paA);


    m_messageB = new DLabel(this);
    if (m_deleteCnt <= 1) {
        m_messageB->setText(DApplication::translate("DeleteConfirmDailog", "This font will not be available to applications"));
    } else {
        if (m_systemCnt == 0) {
            m_messageB->setText(DApplication::translate("DeleteConfirmDailog", "These fonts will not be available to applications"));
        } else {
            m_messageB->setText(DApplication::translate("DeleteConfirmDailog", "These fonts will not be available to applications, and the other %1 system fonts cannot be deleted").arg(m_systemCnt));
        }
//        m_messageB->setText(tr("These fonts will not be available to applications"));
    }
//    m_messageB->setFixedHeight(20);
    m_messageB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

//
//    bfont.setWeight(QFont::Normal);
//    m_messageB->setFont(bfont);
    DFontSizeManager::instance()->bind(m_messageB, DFontSizeManager::T6, QFont::Normal);
    DPalette paB = DApplicationHelper::instance()->palette(m_messageB);
    //pa1 = m_messageA->palette();
    QColor colorB = paB.toolTipText().color();
    color.setAlphaF(0.7);
    paB.setColor(DPalette::WindowText, color);
    DApplicationHelper::instance()->setPalette(m_messageB, paB);
    m_messageB->setWordWrap(true);
    m_messageB->setAlignment(Qt::AlignHCenter);
    m_messageB->setMinimumHeight(m_messageB->fontMetrics().height() * 2);

    QFontMetrics fm(DFontSizeManager::instance()->t6());
    m_messageA->setFixedHeight(fm.height());
//    m_messageB->setFixedHeight(fm.height() + 10);
    if (fm.width(m_messageA->text()) > fm.width(m_messageB->text())) {
        m_old_width = fm.width(m_messageA->text());
    } else {
        m_old_width = fm.width(m_messageB->text());
    }
    m_old_height = fm.height();
    m_w_wd = this->width();
    m_w_ht = this->height();

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
    mainLayout->addSpacing(8);
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
    connect(m_confirmBtn, &DPushButton::clicked, this, [ = ]() {
        if (m_deleting)
            return;
        m_deleting = true;
        Q_EMIT requestDelete();
    });
    connect(this, &DFDeleteDialog::closed, this, [ = ]() {
        disconnect(quitConn);
        if (m_mainWindow != nullptr)
            m_mainWindow->setDeleteFinish();
    });
    connect(qApp, &DApplication::fontChanged, this, &DFDeleteDialog::onFontChanged);
}

void DFDeleteDialog::onFontChanged(const QFont &font)
{
    if (m_count == 0) {

        QFontMetrics fm(font);
        QFontInfo fontInfo(font);
        qDebug() << fontInfo.family() << fontInfo.pixelSize();

        m_messageA->setFixedHeight(fm.height());
        m_messageB->setFixedHeight(fm.height() + 10);

        auto wd = 0;
        if (fm.width(m_messageA->text()) > fm.width(m_messageB->text())) {
            wd = fm.width(m_messageA->text());
        } else {
            wd = fm.width(m_messageB->text());
        }

        auto w_wd = static_cast<int>((static_cast<double>(m_w_wd) / static_cast<double>(m_old_width)) * wd);
        auto w_ht = static_cast<int>((static_cast<double>(m_w_ht) / static_cast<double>(m_old_height)) * fm.height());
        m_w_wd = w_wd;
        m_w_ht = w_ht;

        resize(w_wd, w_ht);

        m_old_width = wd;
        m_old_height = fm.height();
    } else {
        m_count = 0;
        return;
    }
    ++m_count;
}

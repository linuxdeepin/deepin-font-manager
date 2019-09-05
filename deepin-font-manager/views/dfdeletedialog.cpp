#include "dfdeletedialog.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <DApplication>
#include <DFrame>

DFDeleteDialog::DFDeleteDialog(QWidget *parent)
    : DDialog(parent)
{
    initUI();
    initConnections();
}

void DFDeleteDialog::initUI()
{
    setFixedSize(QSize(DEFAULT_WINDOW_W, DEFAULT_WINDOW_H));

    //    setIconPixmap(QPixmap(":/images/deepin-font-manager.svg"));
    //    setTitle("确认删除窗口标题");
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    DFrame *mainFrame = new DFrame(this);
    /// mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainFrame->setFixedSize(QSize(DEFAULT_WINDOW_W, DEFAULT_WINDOW_H));

    m_fontLogo = new DLabel(this);
    m_fontLogo->setObjectName("logoLabel");
    m_fontLogo->setFixedSize(QSize(32, 32));
    m_fontLogo->setFocusPolicy(Qt::NoFocus);
    m_fontLogo->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_fontLogo->setPixmap(QPixmap(":/images/deepin-font-manager.svg"));

    m_messageA = new DLabel(this);
    m_messageA->setText(
        DApplication::translate("DeleteConfirmDailog", "Are you sure to delete this font?"));
    m_messageA->setFixedHeight(20);
    m_messageA->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_messageB = new DLabel(this);
    m_messageB->setText(DApplication::translate(
        "DeleteConfirmDailog", "This font will not be available to applications."));
    m_messageB->setFixedHeight(20);
    m_messageB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QHBoxLayout *actionBarLayout = new QHBoxLayout();
    actionBarLayout->setSpacing(0);
    actionBarLayout->setContentsMargins(0, 0, 0, 0);

    m_cancelBtn = new DPushButton(this);
    m_cancelBtn->setFixedSize(QSize(170, 36));
    m_cancelBtn->setText(DApplication::translate("DeleteConfirmDailog", "Cancel"));

    m_confirmBtn = new DPushButton(this);
    m_confirmBtn->setFixedSize(QSize(170, 36));
    m_confirmBtn->setText(DApplication::translate("DeleteConfirmDailog", "Confirm"));

    actionBarLayout->addWidget(m_cancelBtn);
    actionBarLayout->addWidget(m_confirmBtn);

    mainLayout->addWidget(m_fontLogo, 0, Qt::AlignLeft | Qt::AlignTop);
    mainLayout->addStretch();
    mainLayout->addWidget(m_messageA, 0, Qt::AlignCenter);
    mainLayout->setSpacing(8);
    mainLayout->addWidget(m_messageB, 0, Qt::AlignCenter);
    mainLayout->addStretch();
    mainLayout->addLayout(actionBarLayout);
    mainFrame->setLayout(mainLayout);
    // mainFrame->setStyleSheet("background: blue");
}

void DFDeleteDialog::initConnections()
{
    connect(m_cancelBtn, &DPushButton::clicked, this, [=]() { this->reject(); });
    connect(m_confirmBtn, &DPushButton::clicked, this, [=]() { this->accept(); });
}

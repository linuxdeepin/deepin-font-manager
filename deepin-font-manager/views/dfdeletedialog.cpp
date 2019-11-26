#include "dfdeletedialog.h"
#include "utils.h"
#include "globaldef.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

#include <DApplication>
#include <DApplicationHelper>
#include <DFrame>
#include <DFontSizeManager>

DFDeleteDialog::DFDeleteDialog(QWidget *parent)
    : DFontBaseDialog(parent)
{
    initUI();
    initConnections();
}

void DFDeleteDialog::initUI()
{
    setFixedSize(QSize(DEFAULT_WINDOW_W, DEFAULT_WINDOW_H));
    //setWindowOpacity(0.5);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(10, 0, 10, 10);

    QWidget *mainFrame = new QWidget(this);
    mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QString msgFontFamily = Utils::loadFontFamilyFromFiles(":/images/SourceHanSansCN-Medium.ttf");
    QString msgBFontFamily = Utils::loadFontFamilyFromFiles(":/images/SourceHanSansCN-Bold.ttf");

    m_messageA = new DLabel(this);
    m_messageA->setText(
        DApplication::translate("DeleteConfirmDailog", "Are you sure you want to delete this font?"));
    m_messageA->setFixedHeight(20);
    m_messageA->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QFont msgAFont(msgFontFamily);
    m_messageA->setFont(msgAFont);
    DFontSizeManager::instance()->bind(m_messageA, DFontSizeManager::T6);

    m_messageB = new DLabel(this);
    m_messageB->setText(DApplication::translate(
        "DeleteConfirmDailog", "This font will not be available to applications"));
    m_messageB->setFixedHeight(20);
    m_messageB->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QFont msgBFont(msgBFontFamily);
    msgBFont.setWeight(QFont::Bold);
    m_messageB->setFont(msgBFont);
    DFontSizeManager::instance()->bind(m_messageB, DFontSizeManager::T6);
    //Set MessageB color to TextTips
    DPalette paMsgB = DApplicationHelper::instance()->palette(m_messageB);
    paMsgB.setBrush(DPalette::WindowText, paMsgB.color(DPalette::TextTips));
    m_messageB->setPalette(paMsgB);

    QString fontFamilyName = Utils::loadFontFamilyFromFiles(":/images/SourceHanSansCN-Medium.ttf");
    QFont btnFont(fontFamilyName);

    QHBoxLayout *actionBarLayout = new QHBoxLayout();
    actionBarLayout->setSpacing(0);
    actionBarLayout->setContentsMargins(0, 0, 0, 0);

    m_cancelBtn = new DPushButton(this);
    m_cancelBtn->setFixedHeight(38);
    m_cancelBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_cancelBtn->setText(DApplication::translate("DeleteConfirmDailog", "Cancel"));
    m_cancelBtn->setFont(btnFont);

    m_confirmBtn = new DWarningButton(this);
    m_confirmBtn->setFixedHeight(38);
    m_confirmBtn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    m_confirmBtn->setText(DApplication::translate("DeleteConfirmDailog", "Delete"));
    m_confirmBtn->setFont(btnFont);

    DVerticalLine *verticalSplite = new DVerticalLine(this);
    DPalette pa = DApplicationHelper::instance()->palette(verticalSplite);
    QColor splitColor = pa.color(DPalette::ItemBackground);
    pa.setBrush(DPalette::Background, splitColor);
    verticalSplite->setPalette(pa);
    verticalSplite->setBackgroundRole(QPalette::Background);
    verticalSplite->setAutoFillBackground(true);
    verticalSplite->setFixedSize(3,28);

    actionBarLayout->addWidget(m_cancelBtn);
    actionBarLayout->addSpacing(8);
    actionBarLayout->addWidget(verticalSplite);
    actionBarLayout->addSpacing(8);
    actionBarLayout->addWidget(m_confirmBtn);

    mainLayout->addWidget(m_messageA, 0, Qt::AlignCenter);
    mainLayout->addSpacing(8);
    mainLayout->addWidget(m_messageB, 0, Qt::AlignCenter);
    mainLayout->addStretch();
    mainLayout->addLayout(actionBarLayout);
    mainFrame->setLayout(mainLayout);

    addContent(mainFrame);
}

void DFDeleteDialog::initConnections()
{
    connect(m_cancelBtn, &DPushButton::clicked, this, [=]() { this->reject(); });
    connect(m_confirmBtn, &DPushButton::clicked, this, [=]() { this->accept(); });
}

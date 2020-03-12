#include "dfontbasedialog.h"
#include "globaldef.h"

#include <QVBoxLayout>

#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE

DFontBaseDialog::DFontBaseDialog(QWidget *parent)
    : DAbstractDialog(parent)
{
    initUI();
    InitConnections();
}

void DFontBaseDialog::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(0);
    titleLayout->setContentsMargins(10, 0, 0, 0);

    m_titleBar = new QWidget(this);
    m_titleBar->setFixedHeight(50);
    m_titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    m_titleBar->setLayout(titleLayout);

    m_logoIcon = new DLabel(this);
    m_logoIcon->setFixedSize(QSize(32, 32));
    m_logoIcon->setFocusPolicy(Qt::NoFocus);
    m_logoIcon->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_logoIcon->setPixmap(QIcon::fromTheme("deepin-font-manager").pixmap(QSize(32, 32)));

    m_closeButton = new DWindowCloseButton(this);
    m_closeButton->setFocusPolicy(Qt::NoFocus);
    m_closeButton->setIconSize(QSize(50, 50));

    m_tileText = new DLabel(this);
    m_tileText->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_tileText->setAlignment(Qt::AlignCenter);
    DFontSizeManager::instance()->bind(m_tileText, DFontSizeManager::T6);

    titleLayout->addWidget(m_logoIcon, 0, Qt::AlignLeft | Qt::AlignVCenter);
    titleLayout->addWidget(m_tileText);
    titleLayout->addWidget(m_closeButton, 0, Qt::AlignRight | Qt::AlignVCenter);

    //Dialog content
    m_contentLayout = new QVBoxLayout();
    m_contentLayout->setSpacing(0);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);

    m_content = new QWidget(this);
    m_content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_content->setLayout(m_contentLayout);

    mainLayout->addWidget(m_titleBar);
    mainLayout->addWidget(m_content);
    setLayout(mainLayout);
}

void DFontBaseDialog::InitConnections()
{
    connect(m_closeButton, &DWindowCloseButton::clicked, this, [this]() {
        this->close();
    });
}

void DFontBaseDialog::setLogoVisable(bool visible)
{
    if (nullptr != m_logoIcon) {
        m_logoIcon->setVisible(visible);
    }
}

void DFontBaseDialog::setTitle(const QString &title)
{
    if (nullptr != m_tileText) {
        m_tileText->setText(title);
    }
}

QLayout *DFontBaseDialog::getContentLayout()
{
    return m_contentLayout;
}

void DFontBaseDialog::addContent(QWidget *content)
{
    Q_ASSERT(nullptr != getContentLayout());

    getContentLayout()->addWidget(content);
}

void DFontBaseDialog::setIconPixmap(const QPixmap &iconPixmap)
{
    if (nullptr != m_logoIcon) {
        m_logoIcon->setPixmap(iconPixmap);
    }
}

void DFontBaseDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)

    done(-1);

    Q_EMIT closed();
}

QWidget *DFontBaseDialog::getContent() const
{
    return m_content;
}

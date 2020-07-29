#include "dfontspinnerwidget.h"
#include "dfontspinner.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <DApplication>
#include <QDebug>

DWIDGET_USE_NAMESPACE

DFontSpinnerWidget::DFontSpinnerWidget(QWidget *parent,  SpinnerStyles styles) : DWidget(parent)
{
    Q_UNUSED(styles)
//    setStyles(styles);
    initUI();
}

void DFontSpinnerWidget::initUI()
{
    m_spinner = new DFontSpinner(this);
    m_spinner->setFixedSize(QSize(32, 32));
    m_label = new QLabel(this);
    m_label->setText(DApplication::translate("Main", "Loading fonts, please wait..."));

    QHBoxLayout *hLoadingView = new QHBoxLayout;
    hLoadingView->addStretch(1);

    QVBoxLayout *vLoadingView = new QVBoxLayout;
    vLoadingView->addStretch(1);
    vLoadingView->addWidget(m_spinner, 0, Qt::AlignCenter);
    vLoadingView->addWidget(m_label, 0, Qt::AlignCenter);
    vLoadingView->addStretch(1);

    hLoadingView->addLayout(vLoadingView);
    hLoadingView->addStretch(1);

    this->setLayout(hLoadingView);
}

DFontSpinnerWidget::~DFontSpinnerWidget()
{
}

void DFontSpinnerWidget::setStyles(SpinnerStyles styles)
{
    m_Styles = styles;
    if (m_Styles == SpinnerStyles::Load || m_Styles == SpinnerStyles::StartupLoad) {
        m_label->setText(DApplication::translate("Main", "Loading fonts, please wait..."));
    } else if (m_Styles == SpinnerStyles::Delete) {
        m_label->setText(DApplication::translate("Dfuninstalldialog", "Deleting fonts, please wait..."));
    } else {
        m_label->setText(QString());
    }
}
void DFontSpinnerWidget::spinnerStart()
{
    m_spinner->start();
}

void DFontSpinnerWidget::spinnerStop()
{
    m_spinner->stop();
}

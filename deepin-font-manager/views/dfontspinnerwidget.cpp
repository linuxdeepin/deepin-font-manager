#include "dfontspinnerwidget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <DApplication>
#include <QDebug>

DWIDGET_USE_NAMESPACE

DFontSpinnerWidget::DFontSpinnerWidget(QWidget *parent) : DWidget(parent)
{
    initUI();
}

void DFontSpinnerWidget::initUI()
{
    m_spinner = new DSpinner(this);
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
    delete m_spinner;
    delete m_label;
    m_spinner = nullptr;
    m_label = nullptr;
}

void DFontSpinnerWidget::spinnerStart()
{
    m_spinner->start();
}

void DFontSpinnerWidget::spinnerStop()
{
    m_spinner->stop();
}
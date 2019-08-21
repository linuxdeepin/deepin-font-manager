#include "dfinstallerroritemwidget.h"

#include <DLog>
#include <QVBoxLayout>

DFInstallErrorItemModel::DFInstallErrorItemModel()
{
    bChecked = false;
    bSelectable = true;
    strFontFileName = "";
    strFontFilePath = "";
    strFontInstallStatus = "";
}

DFInstallErrorItemWidget::DFInstallErrorItemWidget(DFInstallErrorItemModel *itemModel)
    : m_itemModel(itemModel)
{
    initUI();
}

void DFInstallErrorItemWidget::initUI()
{
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->setContentsMargins(10, 0, 15, 0);
    this->setLayout(layout);

    QVBoxLayout *checkLayout = new QVBoxLayout;
    m_chooseCheck = new DCheckBox(this);
    m_chooseCheck->setChecked(m_itemModel->bChecked);
    m_chooseCheck->setFixedWidth(30);
    m_chooseCheck->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    checkLayout->addWidget(m_chooseCheck);

    m_fontFileNameLabel = new DLabel(this);
    m_fontFileNameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_fontFileNameLabel->setText(m_itemModel->strFontFileName);
    m_fontFileNameLabel->setStyleSheet(
        "font-family:SourceHanSansSC-Medium;font-size:14px;color:#2C4767;");

    m_fontInstallStatusLabel = new DLabel(this);
    m_fontInstallStatusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_fontInstallStatusLabel->setText(m_itemModel->strFontInstallStatus);
    m_fontInstallStatusLabel->setStyleSheet(
        "font-family:SourceHanSansSC-Medium;font-size:11px;color:#FF6D6D;");

    layout->addLayout(checkLayout);
    layout->addWidget(m_fontFileNameLabel);
    layout->addWidget(m_fontInstallStatusLabel);

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    m_chooseCheck->setStyleSheet("background-color:yellow");
    m_fontFileNameLabel->setStyleSheet("background-color:red");
    m_fontInstallStatusLabel->setStyleSheet("background-color:blue");
#endif
}

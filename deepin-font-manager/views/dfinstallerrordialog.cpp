#include "dfinstallerrordialog.h"

#include <DCheckBox>
#include <DListWidget>
#include <DLog>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QVBoxLayout>

DFInstallErrorDialog::DFInstallErrorDialog(QWidget *parent)
    : DDialog(parent)
{
    initUI();
}

DFInstallErrorDialog::~DFInstallErrorDialog() {}

void DFInstallErrorDialog::initUI()
{
    initMainFrame();
    initTitleBar();

    connect(this, &DFInstallErrorDialog::closed, this, &DFInstallErrorDialog::onCancelInstall);
}

void DFInstallErrorDialog::resizeEvent(QResizeEvent *event)
{
    DDialog::resizeEvent(event);
    m_mainFrame->resize(event->size().width(), event->size().height());
}

void DFInstallErrorDialog::initMainFrame()
{
    this->setFixedSize(448, 302);

    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    m_mainFrame = new DFrame(this);
    m_mainFrame->setContentsMargins(0, 0, 0, 0);
    m_mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_mainFrame->setLayout(m_mainLayout);

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    m_mainFrame->setStyleSheet("background-color:purple");
#endif
}

void DFInstallErrorDialog::initTitleBar()
{
    titleFrame = new DFrame(this);
    titleFrame->setFixedHeight(FTM_TITLE_FIXED_HEIGHT);
    titleFrame->setContentsMargins(0, 0, 0, 0);
    titleFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    logoLabel = new DLabel;
    logoLabel->setFixedSize(QSize(32, 32));
    logoLabel->setFocusPolicy(Qt::NoFocus);
    logoLabel->setPixmap(QPixmap(":/images/exception-logo.svg"));

    titleLabel = new DLabel;
    titleLabel->setText(QString("安装出错"));
    titleLabel->setStyleSheet("font-family:SourceHanSansSC-Medium;font-size:14px;color:#2C4767;");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->setMargin(0);
    titleLayout->setAlignment(Qt::AlignVCenter);
    titleLayout->setContentsMargins(5, 0, 5 + 32, 0);
    titleLayout->setSpacing(0);
    titleLayout->addWidget(logoLabel);
    titleLayout->addWidget(titleLabel);

    titleFrame->setLayout(titleLayout);

    m_mainLayout->addWidget(titleFrame, 0, Qt::AlignTop);

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    titleFrame->setStyleSheet("background-color:red");
#endif
}

void DFInstallErrorDialog::initInstallErrorFontViews(
    QList<DFInstallErrorItemModel *> installErrorFontModelList)
{
    m_installErrorFontModelList = installErrorFontModelList;

    contentFrame = new QFrame(this);
    contentFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->setMargin(0);
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setMargin(0);
    buttonLayout->setSpacing(0);
    buttonLayout->setContentsMargins(10, 0, 10, 10);

    QButtonGroup *btnGroup = new QButtonGroup(this);
    btnGroup->setExclusive(true);

    m_quitInstallBtn = new DPushButton;
    m_quitInstallBtn->setText(QString("退出安装"));
    m_quitInstallBtn->setStyleSheet("font-family:SourceHanSansSC-Medium;font-size:14px;");
    m_quitInstallBtn->setFixedSize(204, 36);

    m_continueInstallBtn = new DPushButton;
    m_continueInstallBtn->setText(QString("继续安装"));
    m_continueInstallBtn->setStyleSheet("font-family:SourceHanSansSC-Medium;font-size:14px;");
    m_continueInstallBtn->setFixedSize(204, 36);

    btnGroup->addButton(m_quitInstallBtn, 0);
    btnGroup->addButton(m_continueInstallBtn, 1);

    m_quitInstallBtn->setCheckable(true);
    m_quitInstallBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    m_continueInstallBtn->setCheckable(true);
    m_continueInstallBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    connect(btnGroup, SIGNAL(buttonClicked(int)), this, SLOT(onControlButtonClicked(int)));

    buttonLayout->addWidget(m_quitInstallBtn);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(m_continueInstallBtn);

    m_installErrorListWidget = new DSplitListWidget(this);
    m_installErrorListWidget->setAttribute(Qt::WA_TranslucentBackground, true);
    m_installErrorListWidget->setFrameShape(QFrame::NoFrame);
    m_installErrorListWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_installErrorListWidget->setAutoScroll(false);
    for (int i = 0; i < m_installErrorFontModelList.size(); i++) {
        DFInstallErrorItemModel *itemModel = m_installErrorFontModelList.at(i);

        QListWidgetItem *listItem = new QListWidgetItem(m_installErrorListWidget);
        if (!itemModel->bSelectable) {
            listItem->setFlags(listItem->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
        }

        listItem->setSizeHint(QSize(0, 48));
        m_installErrorListWidget->addItem(listItem);
        DFInstallErrorItemWidget *itemWidget = new DFInstallErrorItemWidget(itemModel);
        m_installErrorListWidget->setItemWidget(listItem, itemWidget);
    }

    contentLayout->addWidget(m_installErrorListWidget);
    contentLayout->addLayout(buttonLayout);

    contentFrame->setLayout(contentLayout);

    m_mainLayout->addWidget(contentFrame);

    connect(m_installErrorListWidget, SIGNAL(itemClicked(QListWidgetItem *)), this,
            SLOT(onListItemClicked(QListWidgetItem *)));

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    contentFrame->setStyleSheet("background-color:green");
#endif
}

void DFInstallErrorDialog::onListItemClicked(QListWidgetItem *item)
{
    DFInstallErrorItemWidget *widget =
        qobject_cast<DFInstallErrorItemWidget *>(m_installErrorListWidget->itemWidget(item));
    DFInstallErrorItemModel *itemModel = widget->m_itemModel;
    if (!itemModel->bSelectable) {
        return;
    }
    DCheckBox *chooseCheck = widget->m_chooseCheck;
    bool bChecked = false;
    if (Qt::CheckState::Checked == chooseCheck->checkState()) {
        bChecked = false;
        chooseCheck->setChecked(false);
    } else {
        bChecked = true;
        chooseCheck->setChecked(true);
    }
    widget->m_itemModel->bChecked = bChecked;
}

void DFInstallErrorDialog::onControlButtonClicked(int btnIndex)
{
    if (0 == btnIndex) {
        //退出安装
        this->accept();

        emit onCancelInstall();
    } else {
        //继续安装
        QStringList continueInstallFontFileList;

        //根据用户勾选情况添加到继续安装列表中
        for (int i = 0; i < m_installErrorFontModelList.size(); i++) {
            DFInstallErrorItemModel *itemModel = m_installErrorFontModelList.at(i);
            if (itemModel->bChecked) {
                continueInstallFontFileList.push_back(itemModel->strFontFilePath);
            }
        }
        this->accept();

        emit onContinueInstall(continueInstallFontFileList);
    }
}

#include "dfinstallerrordialog.h"
#include "dfontinfomanager.h"
#include "utils.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <DApplication>
#include <DApplicationHelper>
#include <DCheckBox>
#include <DLog>

DFInstallErrorDialog::DFInstallErrorDialog(QWidget *parent, QStringList errorInstallFontFileList)
    : DDialog(parent)
    , m_errorInstallFiles(errorInstallFontFileList)
{
    initData();
    initUI();

    resetContinueInstallBtnStatus();
}

DFInstallErrorDialog::~DFInstallErrorDialog() {}

void DFInstallErrorDialog::initData()
{
    DFontInfo fontInfo;
    DFontInfoManager *fontInfoManager = DFontInfoManager::instance();
    m_installErrorFontModelList.clear();
    foreach (auto it, m_errorInstallFiles) {
        fontInfo = fontInfoManager->getFontInfo(it);

        DFInstallErrorItemModel itemModel;
        if (fontInfo.isError) {
            QFileInfo fileInfo(it);
            itemModel.bSelectable = false;
            itemModel.bChecked = false;
            itemModel.strFontFileName = fileInfo.fileName();
            itemModel.strFontFilePath = fileInfo.filePath();
            itemModel.strFontInstallStatus = DApplication::translate("ExceptionWindow", "File Error");

            m_installErrorFontModelList.push_back(itemModel);
            qDebug() << "verifyFontFiles->" << it << " :Damaged file";
        } else if (fontInfo.isInstalled) {
            QFileInfo fileInfo(it);
            itemModel.bSelectable = true;
            //默认勾选已安装字体
            itemModel.bChecked = true;
            itemModel.strFontFileName = fileInfo.fileName();
            itemModel.strFontFilePath = fileInfo.filePath();
            itemModel.strFontInstallStatus = DApplication::translate("ExceptionWindow", "Installed");

            m_installErrorFontModelList.push_back(itemModel);
            qDebug() << "verifyFontFiles->" << it << " :Installed file";
        } else {
            qDebug() << "verifyFontFiles->" << it << " :new file";
        }
    }
}

void DFInstallErrorDialog::initUI()
{
    initMainFrame();
    initTitleBar();
    initInstallErrorFontViews();

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
    logoLabel->setPixmap(Utils::renderSVG(":/images/exception-logo.svg", logoLabel->size()));

    titleLabel = new DLabel;
    titleLabel->setText(DApplication::translate("ExceptionWindow", "Install Error"));
    QFont titleFont;
    titleFont.setPixelSize(14);
    titleLabel->setFont(titleFont);
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

int DFInstallErrorDialog::getErrorFontCheckedCount()
{
    int checkedCount = 0;
    QStandardItemModel *sourceModel = m_installErrorListView->getErrorListSourceModel();
    for(int i=0; i<sourceModel->rowCount(); i++) {
        QModelIndex index = sourceModel->index(i, 0);
        DFInstallErrorItemModel itemModel = qvariant_cast<DFInstallErrorItemModel>(
                    sourceModel->data(index));
        if (itemModel.bChecked) {
            ++checkedCount;
        }
    }
    return checkedCount;
}

void DFInstallErrorDialog::initInstallErrorFontViews()
{
    contentFrame = new DFrame(this);
    contentFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *contentLayout = new QVBoxLayout;
    contentLayout->setMargin(0);
    contentLayout->setSpacing(0);
    contentLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *listViewLayout = new QVBoxLayout;
    listViewLayout->setMargin(0);
    listViewLayout->setSpacing(0);
    listViewLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setMargin(0);
    buttonLayout->setSpacing(0);
    buttonLayout->setContentsMargins(10, 0, 10, 10);

    QButtonGroup *btnGroup = new QButtonGroup(this);
    btnGroup->setExclusive(true);

    QFont btnFont = Utils::loadFontFamilyFromFiles(":/images/SourceHanSansCN-Medium.ttf");
    btnFont.setPixelSize(14);

    m_quitInstallBtn = new DPushButton;
    m_quitInstallBtn->setFont(btnFont);
    m_quitInstallBtn->setText(DApplication::translate("ExceptionWindow", "Exit"));
    m_quitInstallBtn->setFixedSize(204, 36);

    m_continueInstallBtn = new DPushButton;
    m_continueInstallBtn->setFont(btnFont);
    m_continueInstallBtn->setText(DApplication::translate("ExceptionWindow", "Continue"));
    m_continueInstallBtn->setFixedSize(204, 36);

    btnGroup->addButton(m_quitInstallBtn, 0);
    btnGroup->addButton(m_continueInstallBtn, 1);

    connect(btnGroup, SIGNAL(buttonClicked(int)), this, SLOT(onControlButtonClicked(int)));

    buttonLayout->addWidget(m_quitInstallBtn);
    buttonLayout->addSpacing(20);
    buttonLayout->addWidget(m_continueInstallBtn);

    m_installErrorListView = new DFInstallErrorListView(m_installErrorFontModelList, this);
    m_installErrorListView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_installErrorListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //设置背景透明
    m_installErrorListView->setAttribute(Qt::WA_TranslucentBackground, true);

    listViewLayout->addWidget(m_installErrorListView);
    contentLayout->addLayout(listViewLayout);
    contentLayout->addLayout(buttonLayout);

    contentFrame->setLayout(contentLayout);

    m_mainLayout->addWidget(contentFrame);

    connect(m_installErrorListView, SIGNAL(onClickErrorListItem(QModelIndex)), this,
            SLOT(onListItemClicked(QModelIndex)));

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    m_installErrorListView->setStyleSheet("background-color:cyan");
    contentFrame->setStyleSheet("background-color:blue");
#endif
}

void DFInstallErrorDialog::resetContinueInstallBtnStatus()
{
    //所有字体都未勾选时，禁止点击"继续安装"
    if (0 == getErrorFontCheckedCount()) {
        m_continueInstallBtn->setEnabled(false);
        m_continueInstallBtn->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    }
    else {
        m_continueInstallBtn->setEnabled(true);
        m_continueInstallBtn->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    }
}

void DFInstallErrorDialog::onListItemClicked(QModelIndex index)
{
    DFInstallErrorItemModel itemModel =
        qvariant_cast<DFInstallErrorItemModel>(m_installErrorListView->getErrorListSourceModel()->data(index));
    itemModel.bChecked = !itemModel.bChecked;
    m_installErrorListView->getErrorListSourceModel()->setData(index, QVariant::fromValue(itemModel), Qt::DisplayRole);

    resetContinueInstallBtnStatus();
}

void DFInstallErrorDialog::onControlButtonClicked(int btnIndex)
{
    if (0 == btnIndex) {
        //退出安装
        emit onCancelInstall();
    } else {
        //继续安装
        QStringList continueInstallFontFileList;

        //根据用户勾选情况添加到继续安装列表中
        for (int i = 0; i < m_installErrorFontModelList.size(); i++) {
            DFInstallErrorItemModel itemModel = m_installErrorFontModelList.at(i);
            if (itemModel.bChecked) {
                continueInstallFontFileList.push_back(itemModel.strFontFilePath);
            }
        }

        emit onContinueInstall(continueInstallFontFileList);
    }

    this->accept();
}

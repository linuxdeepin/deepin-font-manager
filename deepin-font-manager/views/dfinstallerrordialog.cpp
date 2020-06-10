#include "dfinstallerrordialog.h"
#include "dfontinfomanager.h"
#include "utils.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStylePainter>

#include <DApplication>
#include <DStyleHelper>
#include <DGuiApplicationHelper>
#include <DApplicationHelper>
#include <DStyleOptionButton>
#include <DVerticalLine>
#include <DCheckBox>
#include <DLog>

#include "dstyleoption.h"

DWIDGET_USE_NAMESPACE

DFMSuggestButton::DFMSuggestButton(QWidget *parent)
    : QPushButton(parent)
{

}

DFMSuggestButton::DFMSuggestButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
}

void DFMSuggestButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QStylePainter p(this);
    DStyleOptionButton option;
    initStyleOption(&option);
    option.init(this);
    option.features |= QStyleOptionButton::ButtonFeature(DStyleOptionButton::SuggestButton);

    DGuiApplicationHelper *appHelper = DGuiApplicationHelper::instance();
    DPalette pa = appHelper->standardPalette(appHelper->themeType());
    option.palette.setColor(QPalette::ButtonText, pa.color(QPalette::HighlightedText));
    option.palette.setBrush(QPalette::Light, pa.brush(DPalette::LightLively));
    option.palette.setBrush(QPalette::Dark, pa.brush(DPalette::DarkLively));

    p.drawControl(QStyle::CE_PushButton, option);
}

DFInstallErrorDialog::DFInstallErrorDialog(QWidget *parent, const QStringList &errorInstallFontFileList,
                                           const  QStringList &systemFontFileListPsname, const QStringList &systemFontFileListFamilyname)
    : DFontBaseDialog(parent)
    , m_errorInstallFiles(errorInstallFontFileList)
    , m_systemFilesPsname(systemFontFileListPsname)
    , m_systemFilesFamilyname(systemFontFileListFamilyname)
{
//    setWindowOpacity(0.3); //Debug
    initData();
    initUI();
    emit m_signalManager->popInstallErrorDialog();
    connect(m_signalManager, &SignalManager::updateInstallErrorListview, this, &DFInstallErrorDialog::addData);
    resetContinueInstallBtnStatus();


}

DFInstallErrorDialog::~DFInstallErrorDialog()
{
    qDebug() << __func__ << "start" << endl;
    emit m_signalManager->hideInstallErrorDialog();
    m_errorInstallFiles.clear();
    m_systemFilesPsname.clear();
    m_systemFilesFamilyname.clear();
    m_installErrorFontModelList.clear();
    qDebug() << __func__ << "end" << endl;
}

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
            itemModel.strFontInstallStatus = DApplication::translate("DFInstallErrorDialog", "Broken file");

            m_installErrorFontModelList.push_back(itemModel);
        } else if (fontInfo.isInstalled && !isSystemFont(fontInfo)) {
            QFileInfo fileInfo(it);
            itemModel.bSelectable = true;
            //默认勾选已安装字体
            itemModel.bChecked = true;
            itemModel.strFontFileName = fileInfo.fileName();
            itemModel.strFontFilePath = fileInfo.filePath();
            itemModel.strFontInstallStatus = DApplication::translate("DFInstallErrorDialog", "Same version installed");

            m_installErrorFontModelList.push_back(itemModel);
        } else if (isSystemFont(fontInfo)) {
            QFileInfo fileInfo(it);
//            m_SystemFontCount++;
            itemModel.bSelectable = false;
            itemModel.bChecked = false;
            itemModel.bSystemFont = true;
            itemModel.strFontFileName = fileInfo.fileName();
            itemModel.strFontFilePath = fileInfo.filePath();
            itemModel.strFontInstallStatus = DApplication::translate("DFInstallErrorDialog", "System Font");
            m_installErrorFontModelList.push_back(itemModel);
        } else {
//            qDebug() << "verifyFontFiles->" << it << " :new file";
        }
    }
}

void DFInstallErrorDialog::initUI()
{
    initMainFrame();
    initTitleBar();
    initInstallErrorFontViews();
    connect(this, &DFInstallErrorDialog::closeBtnClicked, this, &DFInstallErrorDialog::onCancelInstall);
}

void DFInstallErrorDialog::resizeEvent(QResizeEvent *event)
{
    DFontBaseDialog::resizeEvent(event);
    m_mainFrame->resize(event->size().width(), event->size().height());
}

void DFInstallErrorDialog::initMainFrame()
{
    this->setFixedSize(448, 302);

    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    m_mainFrame = new QWidget(this);
    m_mainFrame->setContentsMargins(0, 0, 0, 0);
    m_mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_mainFrame->setLayout(m_mainLayout);

    addContent(m_mainFrame);
    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    m_mainFrame->setStyleSheet("background-color:purple");
#endif
}

void DFInstallErrorDialog::initTitleBar()
{
    setIconPixmap(Utils::renderSVG("://exception-logo.svg", QSize(32, 32)));
    setTitle(DApplication::translate("ExceptionWindow", "Font Verification"));
}

int DFInstallErrorDialog::getErrorFontCheckedCount()
{
    int checkedCount = 0;
    QStandardItemModel *sourceModel = m_installErrorListView->getErrorListSourceModel();
    for (int i = 0; i < sourceModel->rowCount(); i++) {
        QModelIndex index = sourceModel->index(i, 0);
        DFInstallErrorItemModel itemModel = qvariant_cast<DFInstallErrorItemModel>(
                                                sourceModel->data(index));
        if (itemModel.bChecked) {
            ++checkedCount;
        }
    }
    return checkedCount;
}

bool DFInstallErrorDialog::isSystemFont(DFontInfo &f)
{
    QString fontFullPsname;

    if (!f.psname.compare("")) {
        fontFullPsname = f.familyName + f.styleName;
    } else {
        fontFullPsname = f.psname + f.styleName;
    }

    QString fontFullFamliyName = f.familyName + f.styleName;
//    foreach (auto it, m_AllSysFiles) {
//        if (!it.compare(fontFullName)) {
//            return true;
//        }
//    }
    if (m_systemFilesPsname.contains(fontFullPsname) || m_systemFilesFamilyname.contains(fontFullFamliyName)) {
        return true;
    } else {
        return false;
    }
}

void DFInstallErrorDialog::initInstallErrorFontViews()
{
    //contentFrame = new QWidget(this);
    //contentFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *listViewLayout = new QVBoxLayout;
    listViewLayout->setMargin(0);
    listViewLayout->setSpacing(0);
    listViewLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setMargin(0);
    buttonLayout->setSpacing(0);
    buttonLayout->setContentsMargins(10, 0, 10, 0);

    int btnHeight = 38;
    QWidget *btnFrame = new QWidget;
    btnFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    btnFrame->setFixedHeight(btnHeight + 15);
    btnFrame->setLayout(buttonLayout);

    QButtonGroup *btnGroup = new QButtonGroup(this);

    btnGroup->setExclusive(true);

    QFont btnFont = font();
    //btnFont.setPixelSize(14);

    m_quitInstallBtn = new DPushButton;
    m_quitInstallBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_quitInstallBtn->setFont(btnFont);
    //    m_quitInstallBtn->setFixedSize(204, btnHeight);
    m_quitInstallBtn->setMinimumSize(204, btnHeight);
    m_quitInstallBtn->setMaximumSize(204, btnHeight + 5);
    m_quitInstallBtn->setText(DApplication::translate("ExceptionWindow", "Exit"));

    m_continueInstallBtn = new DSuggestButton;
    m_continueInstallBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    //    m_continueInstallBtn->setFixedSize(204, btnHeight);
    m_continueInstallBtn->setMinimumSize(204, btnHeight);
    m_continueInstallBtn->setMaximumSize(204, btnHeight + 5);
    m_continueInstallBtn->setFont(btnFont);
    m_continueInstallBtn->setText(DApplication::translate("ExceptionWindow", "Continue"));

    btnGroup->addButton(m_quitInstallBtn, 0);
    btnGroup->addButton(m_continueInstallBtn, 1);

    connect(btnGroup, SIGNAL(buttonClicked(int)), this, SLOT(onControlButtonClicked(int)));

    DVerticalLine *verticalSplit = new DVerticalLine;
    verticalSplit->setFixedWidth(1);
    verticalSplit->setFixedHeight(28);
    DPalette pa = DApplicationHelper::instance()->palette(verticalSplit);
    QBrush splitBrush = pa.brush(DPalette::ItemBackground);
    pa.setBrush(DPalette::Background, splitBrush);
    verticalSplit->setPalette(pa);
    verticalSplit->setBackgroundRole(QPalette::Background);
    verticalSplit->setAutoFillBackground(true);

    buttonLayout->addWidget(m_quitInstallBtn);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(verticalSplit);
    buttonLayout->addSpacing(9);
    buttonLayout->addWidget(m_continueInstallBtn);

    m_installErrorListView = new DFInstallErrorListView(m_installErrorFontModelList, this);
    m_installErrorListView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_installErrorListView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    listViewLayout->addWidget(m_installErrorListView);
    m_mainLayout->addWidget(m_installErrorListView);
    m_mainLayout->addWidget(btnFrame);
    m_mainLayout->addStretch();

    connect(m_installErrorListView, SIGNAL(onClickErrorListItem(QModelIndex)), this,
            SLOT(onListItemClicked(QModelIndex)));
    connect(this->getCloseButton(), &DWindowCloseButton::clicked, this, [ = ] {
        m_SystemFontCount = 0;
    });

    // Debug layout code
#ifdef FTM_DEBUG_LAYOUT_COLOR
    m_installErrorListView->setStyleSheet("background-color:cyan");
    //contentFrame->setStyleSheet("background-color:blue");
#endif
}

void DFInstallErrorDialog::resetContinueInstallBtnStatus()
{

    qDebug() << m_SystemFontCount << endl;
    //所有字体都未勾选时，禁止点击"继续安装"
    if (0 == getErrorFontCheckedCount()) {
//        m_continueInstallBtn->setEnabled(false);
        if (m_errorInstallFiles.count() > 0) {
            m_continueInstallBtn->setToolTip(DApplication::translate("ExceptionWindow", "No fonts to be installed"));
        }

        m_continueInstallBtn->setDisabled(true);
//        m_continueInstallBtn->setDisabled(true);
//        m_continueInstallBtn->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    } else {
        m_continueInstallBtn->setEnabled(true);
//        m_continueInstallBtn->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    }


}

void DFInstallErrorDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        emit onCancelInstall();
        this->close();
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


/*参数1 正常筛选后筛选出的错误字体  参数2 字体验证框弹出过程中安装的所有字体
  参数3 新添加到字体验证框中的字体  参数4 之前添加到字体验证框中的字体   */
void DFInstallErrorDialog::addData(QStringList &errorFileList, QStringList &halfInstalledFilelist,
                                   QStringList &addHalfInstalledFiles, QStringList &oldHalfInstalledFiles)
{
    DFontInfo fontInfo;
    DFontInfoManager *fontInfoManager = DFontInfoManager::instance();
    QList<DFInstallErrorItemModel> m_updateInstallErrorFontModelList;
    QStringList m_halfInstalledFilelist;

    m_installErrorFontModelList.clear();

    foreach (auto it, errorFileList) {
        fontInfo = fontInfoManager->getFontInfo(it);

        DFInstallErrorItemModel itemModel;
        if (fontInfo.isError) {
            QFileInfo fileInfo(it);
            itemModel.bSelectable = false;
            itemModel.bChecked = false;
            itemModel.strFontFileName = fileInfo.fileName();
            itemModel.strFontFilePath = fileInfo.filePath();
            itemModel.strFontInstallStatus = DApplication::translate("DFInstallErrorDialog", "Broken file");

            m_updateInstallErrorFontModelList.push_back(itemModel);
        } else if (fontInfo.isInstalled && !isSystemFont(fontInfo)) {
            QFileInfo fileInfo(it);
            itemModel.bSelectable = true;
            //默认勾选已安装字体
            itemModel.bChecked = true;
            itemModel.strFontFileName = fileInfo.fileName();
            itemModel.strFontFilePath = fileInfo.filePath();
            itemModel.strFontInstallStatus = DApplication::translate("DFInstallErrorDialog", "Same version installed");

            m_updateInstallErrorFontModelList.push_back(itemModel);
        } else if (isSystemFont(fontInfo)) {
            QFileInfo fileInfo(it);
//            m_SystemFontCount++;
            itemModel.bSelectable = false;
            itemModel.bChecked = false;
            itemModel.bSystemFont = true;
            itemModel.strFontFileName = fileInfo.fileName();
            itemModel.strFontFilePath = fileInfo.filePath();
            itemModel.strFontInstallStatus = DApplication::translate("DFInstallErrorDialog", "System Font");
            m_updateInstallErrorFontModelList.push_back(itemModel);
        } else {
//            qDebug() << "verifyFontFiles->" << it << " :new file";
        }
    }

    if (halfInstalledFilelist.size() > 0) {
        foreach (auto it, halfInstalledFilelist) {
            QFileInfo fileInfo(it);
            DFInstallErrorItemModel itemModel;
            itemModel.bSelectable = true;
            //默认勾选已安装字体
            itemModel.bChecked = true;
            itemModel.strFontFileName = fileInfo.fileName();
            itemModel.strFontFilePath = fileInfo.filePath();
            itemModel.strFontInstallStatus = DApplication::translate("DFInstallErrorDialog", "Same version installed");
            m_updateInstallErrorFontModelList.push_back(itemModel);
        }
    }

    m_installErrorListView->addErrorListData(m_updateInstallErrorFontModelList);
    m_installErrorListView->checkScrollToIndex(addHalfInstalledFiles, oldHalfInstalledFiles, errorFileList);
    addHalfInstalledFiles.append(oldHalfInstalledFiles);
    addHalfInstalledFiles.append(errorFileList);
    m_installErrorListView->setSelectStatus(addHalfInstalledFiles);
    resetContinueInstallBtnStatus();

}

void DFInstallErrorDialog::onControlButtonClicked(int btnIndex)
{
    if (0 == btnIndex) {
        //退出安装
        emit onCancelInstall();
        emit m_signalManager->setSpliteWidgetScrollEnable(false);//设置菜单滚动可用
        this->close();
    } else {
        //继续安装
        QStringList continueInstallFontFileList;

        //根据用户勾选情况添加到继续安装列表中
        for (int i = 0; i < m_installErrorListView->model()->rowCount(); i++) {
            DFInstallErrorItemModel itemModel = qvariant_cast<DFInstallErrorItemModel>(m_installErrorListView->model()->data(m_installErrorListView->model()->index(i, 0)));
            if (itemModel.bChecked && !itemModel.bSystemFont)
                continueInstallFontFileList.push_back(itemModel.strFontFilePath);
        }

        m_SystemFontCount = 0;

        emit onContinueInstall(continueInstallFontFileList);
        this->close();
    }
}

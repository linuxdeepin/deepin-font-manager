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

#define LISTVIEW_LEFT_SPACING 2

DWIDGET_USE_NAMESPACE
/*************************************************************************
 <Function>      DFMSuggestButton
 <Description>   内部类构造函数
 <Author>
 <Input>
    <param1>     parent          Description:父对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFMSuggestButton::DFMSuggestButton(QWidget *parent)
    : QPushButton(parent)
{

}

/*************************************************************************
 <Function>      DFMSuggestButton
 <Description>   内部类构造函数
 <Author>
 <Input>
    <param1>     text            Description:构建文本信息
    <param1>     parent          Description:父对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFMSuggestButton::DFMSuggestButton(const QString &text, QWidget *parent)
    : QPushButton(text, parent)
{
}

/*************************************************************************
 <Function>      paintEvent
 <Description>   绘图事件-绘制页面颜色
 <Author>
 <Input>
    <param1>     event           Description:事件对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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

/*************************************************************************
 <Function>      DFInstallErrorDialog
 <Description>   构造函数-构造安装验证框页面
 <Author>        null
 <Input>
    <param1>     parent                       Description:父对象
    <param2>     errorInstallFontFileList     Description:安装时重复字体列表
    <param3>     systemFontFileListPsname     Description:安装时系统字体Psname列表
    <param4>     systemFontFileListFamilyname Description:安装时系统字体Familyname列表
 <Return>        null                         Description:null
 <Note>          null
*************************************************************************/
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
//    m_installErrorListView->setFocus();
}

/*************************************************************************
 <Function>      ~DFInstallErrorDialog
 <Description>   析构函数-析构字体验证框页面
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFInstallErrorDialog::~DFInstallErrorDialog()
{
    qDebug() << __func__ << "start" << endl;
    emit m_signalManager->hideInstallErrorDialog();
    m_errorInstallFiles.clear();
    m_systemFilesPsname.clear();
    m_systemFilesFamilyname.clear();
    m_installErrorFontModelList.clear();
//    m_NeedSelectFiles.clear();
    qDebug() << __func__ << "end" << endl;
}

/*************************************************************************
 <Function>      initData
 <Description>   构造时初始化字体信息列表
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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
            itemModel.bIsNormalUserFont = true;//SP3--安装验证页面，回车取消/选中--设置字体状态
            itemModel.strFontInstallStatus = DApplication::translate("DFInstallErrorDialog", "Same version installed");
//            m_NeedSelectFiles.append(it);
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

/*************************************************************************
 <Function>      initUI
 <Description>   字体验证框主页面
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorDialog::initUI()
{
    initMainFrame();
    initTitleBar();
    initInstallErrorFontViews();

    connect(this, &DFInstallErrorDialog::closeBtnClicked, this, &DFInstallErrorDialog::onCancelInstall);
}

/*************************************************************************
 <Function>      resizeEvent
 <Description>   字体验证框大小重绘页面
 <Author>
 <Input>
    <param1>     event           Description:事件对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorDialog::resizeEvent(QResizeEvent *event)
{
    DFontBaseDialog::resizeEvent(event);
    m_mainFrame->resize(event->size().width(), event->size().height());
}

/*************************************************************************
 <Function>      initMainFrame
 <Description>   初始化字体验证框页面框架
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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
}

/*************************************************************************
 <Function>      initTitleBar
 <Description>   初始化标题栏信息
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorDialog::initTitleBar()
{
    setIconPixmap(Utils::renderSVG("://exception-logo.svg", QSize(32, 32)));
    setTitle(DApplication::translate("ExceptionWindow", "Font Verification"));
}

/*************************************************************************
 <Function>      getErrorFontCheckedCount
 <Description>   获取已勾选继续安装项个数
 <Author>
 <Input>         null
 <Return>        int             Description:选中项个数
 <Note>          null
*************************************************************************/
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

/*************************************************************************
 <Function>      isSystemFont
 <Description>   判断当前字体是否为系统字体
 <Author>
 <Input>
    <param1>     f               Description:当前字体信息
 <Return>        bool            Description:是否为系统字体，true：系统字体；false：非系统字体
 <Note>          null
*************************************************************************/
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

/*************************************************************************
 <Function>      initInstallErrorFontViews
 <Description>   初始化字体验证列表视图布局
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorDialog::initInstallErrorFontViews()
{
    //contentFrame = new QWidget(this);
    //contentFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_mainLayout->setMargin(0);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

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
    m_installErrorListView->setFixedWidth(width() - LISTVIEW_LEFT_SPACING);
    m_installErrorListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_installErrorListView->setSelectionMode(QAbstractItemView::ExtendedSelection);


    m_mainLayout->addWidget(m_installErrorListView);
    m_mainLayout->addWidget(btnFrame);
    m_mainLayout->addStretch();

//    m_installErrorListView->setSelectStatus(m_NeedSelectFiles);

    connect(m_installErrorListView, SIGNAL(onClickErrorListItem(QModelIndex)), this,
            SLOT(onListItemClicked(QModelIndex)));
    connect(this->getCloseButton(), &DWindowCloseButton::clicked, this, [ = ] {
        m_SystemFontCount = 0;
    });

}

/*************************************************************************
 <Function>      resetContinueInstallBtnStatus
 <Description>   刷新继续按钮的状态-选中数量大于1时，继续按钮可用
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorDialog::resetContinueInstallBtnStatus()
{

//    qDebug() << m_SystemFontCount << endl;
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

/*************************************************************************
 <Function>      keyPressEvent
 <Description>   重写键盘press事件
 <Author>
 <Input>
    <param1>     event           Description:事件对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        this->close();
    }
    //SP3--安装验证页面，回车取消/选中
    else if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) {
        if (m_installErrorListView->hasFocus()) {
            if (m_installErrorListView->selectionModel()->selectedIndexes().count() == 1) {
                onListItemClicked(m_installErrorListView->currentIndex());
                return;
            } else if (m_installErrorListView->selectionModel()->selectedIndexes().count() > 1) {
                onListItemsClicked(m_installErrorListView->selectionModel()->selectedIndexes());
            }
        }
        //默认字体列表无焦点情况下，回车执行“继续”或“取消”按钮
        else {
            if (m_continueInstallBtn->isEnabled())
                emit m_continueInstallBtn->click();
            else {
                m_quitInstallBtn->click();
            }
        }
    }
    QWidget::keyPressEvent(event);
}

/*************************************************************************
 <Function>      closeEvent
 <Description>   重写关闭事件-发送取消继续安装信号
 <Author>
 <Input>
    <param1>     event           Description:事件对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorDialog::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event)
    emit onCancelInstall();
}

/*************************************************************************
 <Function>      onListItemClicked
 <Description>   勾选按钮点击或回车选中事件
 <Author>
 <Input>
    <param1>     index           Description:当前选中项
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFInstallErrorDialog::onListItemClicked(QModelIndex index)
{
    DFInstallErrorItemModel itemModel =
        qvariant_cast<DFInstallErrorItemModel>(m_installErrorListView->getErrorListSourceModel()->data(index));
    //SP3--安装验证页面，回车取消/选中(539)--正常字体可操作
    if (itemModel.bIsNormalUserFont) {
        itemModel.bChecked = !itemModel.bChecked;
        m_installErrorListView->ifNeedScrollTo(index);
        m_installErrorListView->getErrorListSourceModel()->setData(index, QVariant::fromValue(itemModel), Qt::DisplayRole);
        m_installErrorListView->updateErrorFontModelList(index.row(), itemModel);
        resetContinueInstallBtnStatus();
    }
}

/*************************************************************************/
/*                                                                       */
/* <Function>    onListItemsClicked                                      */
/*                                                                       */
/* <Description> 用于处理字体验证框中选择多个字体后使用快捷键改变选中字体的选择状态 */
/*                                                                       */
/* <para>       所有选中项的index构成的indexlist                            */
/*                                                                       */
/* <Return>     无返回值                                                  */
void DFInstallErrorDialog::onListItemsClicked(QModelIndexList indexList)
{
    m_installErrorListView->sortModelIndexList(indexList);


    DFInstallErrorItemModel itemModel =
        qvariant_cast<DFInstallErrorItemModel>(m_installErrorListView->getErrorListSourceModel()->data(indexList.last()));

    foreach (auto it, indexList) {
        qDebug() << it.row() << "++++++++++++++++++++++++++++++" << endl;
        DFInstallErrorItemModel itemModel2 =
            qvariant_cast<DFInstallErrorItemModel>(m_installErrorListView->getErrorListSourceModel()->data(it));
        if (itemModel2.bIsNormalUserFont) {
            itemModel2.bChecked = !itemModel.bChecked;
            //m_installErrorListView->ifNeedScrollTo(it);
            m_installErrorListView->getErrorListSourceModel()->setData(it, QVariant::fromValue(itemModel2), Qt::DisplayRole);
            m_installErrorListView->updateErrorFontModelList(it.row(), itemModel2);
        }

    }
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
            itemModel.bIsNormalUserFont = true;//SP3--安装验证页面，回车取消/选中(539)--设置字体状态
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
            itemModel.bIsNormalUserFont = true;
            itemModel.strFontInstallStatus = DApplication::translate("DFInstallErrorDialog", "Same version installed");
            m_updateInstallErrorFontModelList.push_back(itemModel);
        }
    }

//    m_installErrorListView->getErrorListSourceModel();
    QModelIndexList list = m_installErrorListView->selectionModel()->selectedIndexes();
    m_installErrorListView->addErrorListData(m_updateInstallErrorFontModelList);
    m_installErrorListView->checkScrollToIndex(addHalfInstalledFiles, oldHalfInstalledFiles, errorFileList);

    addHalfInstalledFiles.append(oldHalfInstalledFiles);
    addHalfInstalledFiles.append(errorFileList);

    m_installErrorListView->setSelectStatus(addHalfInstalledFiles, list);

//    m_installErrorListView->setFocus();

    resetContinueInstallBtnStatus();

}

/*************************************************************************
 <Function>      onControlButtonClicked
 <Description>   按钮点击事件
 <Author>
 <Input>
    <param1>     btnIndex        Description:按钮索引
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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
//        this->close();
        this->reject();
    }
}

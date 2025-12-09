// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfinstallerrordialog.h"
#include "dfontinfomanager.h"
#include "dfinstallnormalwindow.h"
#include "utils.h"
#include <QDebug>

#include <DApplication>
#if QT_VERSION_MAJOR <= 5
#include <DApplicationHelper>
#endif
#include <DVerticalLine>

#include <QButtonGroup>
#include <QFileInfo>

DWIDGET_USE_NAMESPACE
#define LISTVIEW_LEFT_SPACING 10

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
DFInstallErrorDialog::DFInstallErrorDialog(QWidget *parent, const QStringList &errorInstallFontFileList)
    : DDialog(parent)
    , m_parent(qobject_cast<DFInstallNormalWindow *>(parent))
    , m_signalManager(SignalManager::instance())
    , m_errorInstallFiles(errorInstallFontFileList)
{
    qDebug() << "DFInstallErrorDialog created with" << errorInstallFontFileList.size()
             << "error font files";
//    setWindowOpacity(0.3); //Debug
    initData();

    initUI();
    emit m_signalManager->popInstallErrorDialog();
    connect(m_signalManager, &SignalManager::updateInstallErrorListview, this, &DFInstallErrorDialog::addData);
    resetContinueInstallBtnStatus();

    connect(this, &DFInstallErrorDialog::buttonClicked, this, &DFInstallErrorDialog::onControlButtonClicked);

#ifdef DTKWIDGET_CLASS_DSizeMode
    slotSizeModeChanged(DGuiApplicationHelper::instance()->sizeMode());
    connect(DGuiApplicationHelper::instance(),&DGuiApplicationHelper::sizeModeChanged,this, &DFInstallErrorDialog::slotSizeModeChanged);
#endif
    qDebug() << "Exiting function: DFInstallErrorDialog::DFInstallErrorDialog";
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
    qDebug() << "DFInstallErrorDialog destructor start";
    emit m_signalManager->hideInstallErrorDialog();
    m_errorInstallFiles.clear();
    m_installErrorFontModelList.clear();
//    m_NeedSelectFiles.clear();
    qDebug() << "DFInstallErrorDialog destructor end";
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
    qDebug() << "Entering function: DFInstallErrorDialog::initData with" << m_errorInstallFiles.size() << "error font files";
    DFontInfo fontInfo;
    DFontInfoManager *fontInfoManager = DFontInfoManager::instance();
    m_installErrorFontModelList.clear();


    for (auto &it : m_errorInstallFiles) {
        fontInfo = fontInfoManager->getFontInfo(it);

        DFInstallErrorItemModel itemModel;
        if (fontInfo.isError) {
            // qDebug() << "Processing broken font file:" << it;
            QFileInfo fileInfo(it);
            itemModel.bSelectable = false;
            itemModel.bChecked = false;
            itemModel.strFontFileName = fileInfo.fileName();
            itemModel.strFontFilePath = fileInfo.filePath();
            itemModel.strFontInstallStatus = DApplication::translate("DFInstallErrorDialog", "Broken file");
            qWarning() << "Found broken font file:" << fileInfo.fileName();
            m_installErrorFontModelList.push_back(itemModel);
        } else if (fontInfo.isInstalled && !m_parent->isSystemFont(fontInfo)) {
            // qDebug() << "Processing already installed font file:" << it;
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
        } else if (m_parent->isSystemFont(fontInfo)) {
            // qDebug() << "Processing system font file:" << it;
            QFileInfo fileInfo(it);
//            m_SystemFontCount++;
            itemModel.bSelectable = false;
            itemModel.bChecked = false;
            itemModel.bSystemFont = true;
            itemModel.strFontFileName = fileInfo.fileName();
            itemModel.strFontFilePath = fileInfo.filePath();
            itemModel.strFontInstallStatus = DApplication::translate("DFInstallErrorDialog", "System Font");
            qInfo() << "Found system font:" << fileInfo.fileName();
            m_installErrorFontModelList.push_back(itemModel);
        } else {
//            qDebug() << "verifyFontFiles->" << it << " :new file";
        }
    }

    qDebug() << "Exiting function: DFInstallErrorDialog::initData";
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
    qDebug() << "Entering function: DFInstallErrorDialog::initUI";
    setContentsMargins(0, 0, 0, 0);
#if QT_VERSION_MAJOR > 5
    setIcon(Utils::renderSVG("://exception-logo.svg", QSize(32, 32)));
#else
    setIconPixmap(Utils::renderSVG("://exception-logo.svg", QSize(32, 32)));
#endif
    setWindowTitle(DApplication::translate("ExceptionWindow", "Font Verification"));

    this->setFixedSize(448, 302);

    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    m_mainFrame = new QWidget(this);
    m_mainFrame->setContentsMargins(0, 0, 0, 0);
    m_mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_mainFrame->setLayout(m_mainLayout);
    addContent(m_mainFrame, Qt::AlignHCenter);

    DVerticalLine *verticalSplit = new DVerticalLine;
    verticalSplit->setFixedWidth(1);
    verticalSplit->setFixedHeight(28);
#if QT_VERSION_MAJOR > 5
    DPalette pa = verticalSplit->palette();
#else
    DPalette pa = DApplicationHelper::instance()->palette(verticalSplit);
#endif
    QBrush splitBrush = pa.brush(DPalette::ItemBackground);
    pa.setBrush(DPalette::Window, splitBrush);
    verticalSplit->setPalette(pa);
    verticalSplit->setBackgroundRole(QPalette::Window);
    verticalSplit->setAutoFillBackground(true);

    m_installErrorListView = new DFInstallErrorListView(m_installErrorFontModelList, this);
    m_installErrorListView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_installErrorListView->setFixedWidth(width() - LISTVIEW_LEFT_SPACING * 2);
    m_installErrorListView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_installErrorListView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_mainLayout->addWidget(m_installErrorListView);

    insertButton(0, DApplication::translate("ExceptionWindow", "Exit"), false, ButtonNormal);
    insertButton(1, DApplication::translate("ExceptionWindow", "Continue"), true, ButtonRecommend);

    connect(m_installErrorListView, SIGNAL(clickedErrorListItem(QModelIndex)), this,
            SLOT(onListItemClicked(QModelIndex)));
    qDebug() << "Exiting function: DFInstallErrorDialog::initUI";
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
    qDebug() << "Entering function: DFInstallErrorDialog::getErrorFontCheckedCount";
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
    qDebug() << "Exiting function: DFInstallErrorDialog::getErrorFontCheckedCount";
    return checkedCount;
}

/*************************************************************************
 <Function>      getErrorFontSelectableCount
 <Description>   获取可选项个数
 <Author>
 <Input>         null
 <Return>        int             Description:选中项个数
 <Note>          null
*************************************************************************/
int DFInstallErrorDialog::getErrorFontSelectableCount()
{
    qDebug() << "Entering function: DFInstallErrorDialog::getErrorFontSelectableCount";
    int checkedCount = 0;
    QStandardItemModel *sourceModel = m_installErrorListView->getErrorListSourceModel();
    for (int i = 0; i < sourceModel->rowCount(); i++) {
        QModelIndex index = sourceModel->index(i, 0);
        DFInstallErrorItemModel itemModel = qvariant_cast<DFInstallErrorItemModel>(
                                                sourceModel->data(index));
        if (itemModel.bSelectable) {
            ++checkedCount;
        }
    }
    qDebug() << "Exiting function: DFInstallErrorDialog::getErrorFontSelectableCount";
    return checkedCount;
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
    qDebug() << "Entering function: DFInstallErrorDialog::resetContinueInstallBtnStatus";
    //所有字体都未勾选时，禁止点击"继续安装"
    if (0 == getErrorFontCheckedCount()) {
        qDebug() << "No fonts checked, disabling continue button";
        if (m_errorInstallFiles.count() > 0) {
            getButton(1)->setToolTip(DApplication::translate("ExceptionWindow", "No fonts to be installed"));
        }
        getButton(1)->setDisabled(true);
        getButton(1)->setFocusPolicy(Qt::NoFocus);
    } else {
        qDebug() << "Fonts checked, enabling continue button";
        getButton(1)->setEnabled(true);
        getButton(1)->setFocusPolicy(Qt::TabFocus);
    }

    if (getErrorFontSelectableCount() > 0) {
        qDebug() << "Selectable fonts available, enabling list view focus";
        m_installErrorListView->setFocusPolicy(Qt::TabFocus);
    } else {
        qDebug() << "No selectable fonts, disabling list view focus";
        m_installErrorListView->setFocusPolicy(Qt::NoFocus);
    }

    qDebug() << "Exiting function: DFInstallErrorDialog::resetContinueInstallBtnStatus";
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
    // qDebug() << "Entering function: DFInstallErrorDialog::keyPressEvent with key:" << event->key();

    if (event->key() == Qt::Key_Escape) {
        // qDebug() << "Escape key pressed - closing dialog";
        this->close();
    } else if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) {
        // qDebug() << "Return/Enter key pressed - processing list focus";
        //SP3--安装验证页面，回车取消/选中
        if (m_installErrorListView->hasFocus()) {
            // qDebug() << "List view has focus - processing selection";
            if (m_installErrorListView->selectionModel()->selectedIndexes().count() == 1) {
                // qDebug() << "Single item selected - calling onListItemClicked";
                onListItemClicked(m_installErrorListView->selectionModel()->selectedIndexes().first());
                return;
            } else if (m_installErrorListView->selectionModel()->selectedIndexes().count() > 1) {
                // qDebug() << "Multiple items selected - calling onListItemsClicked";
                onListItemsClicked(m_installErrorListView->selectionModel()->selectedIndexes());
            }
        } else {
            // qDebug() << "List view has no focus - processing button clicks";
            //默认字体列表无焦点情况下，回车执行“继续”或“取消”按钮
            if (getButton(1)->isEnabled()) {
                // qDebug() << "Continue button enabled - clicking continue button";
                emit getButton(1)->click();
            } else {
                // qDebug() << "Continue button disabled - clicking cancel button";
                emit getButton(0)->click();
            }
        }
    } else if (event->key() == Qt::Key_Home && event->modifiers() == Qt::NoModifier) {
        // qDebug() << "Home key pressed - navigating to top";
        m_installErrorListView->responseToHomeAndEnd(true);
    } else if (event->key() == Qt::Key_End && event->modifiers() == Qt::NoModifier) {
        // qDebug() << "End key pressed - navigating to bottom";
        m_installErrorListView->responseToHomeAndEnd(false);
    } else if (event->key() == Qt::Key_PageUp && event->modifiers() == Qt::NoModifier) {
        // qDebug() << "PageUp key pressed - navigating up one page";
        m_installErrorListView->responseToPageUpAndPageDown(true);
    } else if (event->key() == Qt::Key_PageDown && event->modifiers() == Qt::NoModifier) {
        // qDebug() << "PageDown key pressed - navigating down one page";
        m_installErrorListView->responseToPageUpAndPageDown(false);
    }
    QWidget::keyPressEvent(event);
    // qDebug() << "Exiting function: DFInstallErrorDialog::keyPressEvent";
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
    // qDebug() << "Entering function: DFInstallErrorDialog::closeEvent";
    Q_UNUSED(event)
    //设置菜单滚动可用
    emit m_signalManager->setSpliteWidgetScrollEnable(false);

    //关闭窗口时发送取消安装信号
    emit onCancelInstall();

    DDialog::closeEvent(event);
    // qDebug() << "Exiting function: DFInstallErrorDialog::closeEvent";
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
void DFInstallErrorDialog::onListItemClicked(const QModelIndex &index)
{
    qDebug() << "Entering function: DFInstallErrorDialog::onListItemClicked";
    DFInstallErrorItemModel itemModel =
        qvariant_cast<DFInstallErrorItemModel>(m_installErrorListView->getErrorListSourceModel()->data(index));
    //SP3--安装验证页面，回车取消/选中(539)--正常字体可操作
    if (itemModel.bIsNormalUserFont) {
        qDebug() << "Normal user font clicked, toggling checked state";
        itemModel.bChecked = !itemModel.bChecked;
        m_installErrorListView->ifNeedScrollTo(index);
        m_installErrorListView->getErrorListSourceModel()->setData(index, QVariant::fromValue(itemModel), Qt::DisplayRole);
        m_installErrorListView->updateErrorFontModelList(index.row(), itemModel);
        resetContinueInstallBtnStatus();
    }
    qDebug() << "Exiting function: DFInstallErrorDialog::onListItemClicked";
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
void DFInstallErrorDialog::onListItemsClicked(const QModelIndexList &indexList)
{
    qDebug() << "Entering function: DFInstallErrorDialog::onListItemsClicked";
    QModelIndex firstIndex;
    for (auto &it : indexList) {
        if (!firstIndex.isValid() || firstIndex.row() > it.row())
            firstIndex = it;
    }

    DFInstallErrorItemModel itemModel =
        qvariant_cast<DFInstallErrorItemModel>(m_installErrorListView->getErrorListSourceModel()->data(firstIndex));

    for (auto &it : indexList) {
        qDebug() << it.row() << "++++++++++++++++++++++++++++++" << "\n";
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
    qDebug() << "Exiting function: DFInstallErrorDialog::onListItemsClicked";
}


/*参数1 正常筛选后筛选出的错误字体  参数2 字体验证框弹出过程中安装的所有字体
  参数3 新添加到字体验证框中的字体  参数4 之前添加到字体验证框中的字体   */
void DFInstallErrorDialog::addData(QStringList &errorFileList, QStringList &halfInstalledFilelist,
                                   QStringList &addHalfInstalledFiles, QStringList &oldHalfInstalledFiles)
{
    qDebug() << "Entering function: DFInstallErrorDialog::addData";
    DFontInfo fontInfo;
    DFontInfoManager *fontInfoManager = DFontInfoManager::instance();
    QList<DFInstallErrorItemModel> m_updateInstallErrorFontModelList;
    QStringList m_halfInstalledFilelist;

    m_installErrorFontModelList.clear();

    for (auto &it : errorFileList) {
        fontInfo = fontInfoManager->getFontInfo(it);
        qDebug() << "getfontinfo success!!!!!!!!!!1" << "\n";
        DFInstallErrorItemModel itemModel;
        if (fontInfo.isError) {
            QFileInfo fileInfo(it);
            itemModel.bSelectable = false;
            itemModel.bChecked = false;
            itemModel.strFontFileName = fileInfo.fileName();
            itemModel.strFontFilePath = fileInfo.filePath();
            itemModel.strFontInstallStatus = DApplication::translate("DFInstallErrorDialog", "Broken file");

            m_updateInstallErrorFontModelList.push_back(itemModel);
        } else if (fontInfo.isInstalled && !m_parent->isSystemFont(fontInfo)) {
            QFileInfo fileInfo(it);
            itemModel.bSelectable = true;
            //默认勾选已安装字体
            itemModel.bChecked = true;
            itemModel.strFontFileName = fileInfo.fileName();
            itemModel.strFontFilePath = fileInfo.filePath();
            itemModel.bIsNormalUserFont = true;//SP3--安装验证页面，回车取消/选中(539)--设置字体状态
            itemModel.strFontInstallStatus = DApplication::translate("DFInstallErrorDialog", "Same version installed");

            m_updateInstallErrorFontModelList.push_back(itemModel);
        } else if (m_parent->isSystemFont(fontInfo)) {
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

    for (auto &it : halfInstalledFilelist) {
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

//    m_installErrorListView->getErrorListSourceModel();
    QModelIndexList list = m_installErrorListView->selectionModel()->selectedIndexes();
    m_installErrorListView->addErrorListData(m_updateInstallErrorFontModelList);
    m_installErrorListView->checkScrollToIndex(addHalfInstalledFiles, oldHalfInstalledFiles, errorFileList);

    addHalfInstalledFiles.append(oldHalfInstalledFiles);
    addHalfInstalledFiles.append(errorFileList);

    m_installErrorListView->setSelectStatus(addHalfInstalledFiles, list);

//    m_installErrorListView->setFocus();

    resetContinueInstallBtnStatus();

    qDebug() << "Exiting function: DFInstallErrorDialog::addData";
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
    qDebug() << "Entering function: DFInstallErrorDialog::onControlButtonClicked with button index:" << btnIndex;
    if (0 == btnIndex) {
        //退出安装
        qInfo() << "User canceled font installation";
        this->close();
    } else {
        qDebug() << "Continue installation";
        //继续安装
        QStringList continueInstallFontFileList;

        //根据用户勾选情况添加到继续安装列表中
        for (int i = 0; i < m_installErrorListView->model()->rowCount(); i++) {
            DFInstallErrorItemModel itemModel = qvariant_cast<DFInstallErrorItemModel>(m_installErrorListView->model()->data(m_installErrorListView->model()->index(i, 0)));
            if (itemModel.bChecked && !itemModel.bSystemFont)
                continueInstallFontFileList.push_back(itemModel.strFontFilePath);
        }

        emit onContinueInstall(continueInstallFontFileList);
        qInfo() << "User continued installation with" << continueInstallFontFileList.size()
               << "selected fonts";
        this->reject();
    }
    qDebug() << "Exiting function: DFInstallErrorDialog::onControlButtonClicked";
}

#ifdef DTKWIDGET_CLASS_DSizeMode
void DFInstallErrorDialog::slotSizeModeChanged(DGuiApplicationHelper::SizeMode sizeMode)
{
    qDebug() << "Entering function: DFInstallErrorDialog::slotSizeModeChanged with sizeMode:" << sizeMode;
    Utils::clearImgCache();
    if (sizeMode == DGuiApplicationHelper::SizeMode::CompactMode) {
        qDebug() << "Switching to compact mode";
        this->setFixedSize(448, 259);
#if QT_VERSION_MAJOR > 5
        setIcon(Utils::renderSVG("://exception-logo.svg", QSize(25, 25)));
#else
        setIconPixmap(Utils::renderSVG("://exception-logo.svg", QSize(25, 25)));
#endif
    } else {
        qDebug() << "Switching to normal mode";
        this->setFixedSize(448, 302);
#if QT_VERSION_MAJOR > 5
        setIcon(Utils::renderSVG("://exception-logo.svg", QSize(32, 32)));
#else
        setIconPixmap(Utils::renderSVG("://exception-logo.svg", QSize(32, 32)));
#endif
    }
    qDebug() << "Exiting function: DFInstallErrorDialog::slotSizeModeChanged";
}
#endif

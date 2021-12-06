/*
 *
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:
*
* Maintainer:
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dfontbasedialog.h"
#include "globaldef.h"
#include "utils.h"

#include <QVBoxLayout>
#include <QApplication>

#include <DFontSizeManager>

DWIDGET_USE_NAMESPACE

DFontBaseDialog::DFontBaseDialog(QWidget *parent)
    : DAbstractDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    initUI();
    InitConnections();
}

DFontBaseDialog::~DFontBaseDialog()
{
    if (qApp->activeWindow() == this) {
        qApp->setActiveWindow(nullptr);
        hide();
    }
}

/*************************************************************************
 <Function>      initUI
 <Description>   初始化ui界面
 <Author>        null
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
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
//    m_closeButton->setFocusPolicy(Qt::NoFocus);//SP3--设置tab顺序--设置close按钮可聚焦(539)
    m_closeButton->setIconSize(QSize(50, 50));
    // waylandh环境下隐藏关闭按钮
    if (Utils::isWayland()) {
        m_closeButton->setVisible(false);
    }

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


/*************************************************************************
 <Function>      InitConnections
 <Description>   初始化信号与槽的链接
 <Input>
    <param1>     null            Description:null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontBaseDialog::InitConnections()
{
    connect(m_closeButton, &DWindowCloseButton::clicked, this, [this]() {
        Q_EMIT closeBtnClicked();
        this->close();
    });
}

/*************************************************************************
 <Function>      setLogoVisable
 <Description>   设置logo标签是否可见
 <Author>        null
 <Input>
    <param1>     visible            Description:表示logo标签要设置的状态
 <Return>        null
 <Note>          null
*************************************************************************/
void DFontBaseDialog::setLogoVisable(bool visible)
{
    if (nullptr != m_logoIcon) {
        m_logoIcon->setVisible(visible);
    }
}


/*************************************************************************
 <Function>      setTitle
 <Description>   设置title标签显示的内容
 <Author>        null
 <Input>
    <param1>     title            Description:需要标签显示的内容
 <Return>        null
 <Note>          null
*************************************************************************/
void DFontBaseDialog::setTitle(const QString &title)
{
    if (nullptr != m_tileText) {
        m_tileText->setText(title);
    }
}

/*************************************************************************
 <Function>      getContentLayout
 <Description>   获取窗口使用的布局
 <Author>        null
 <Input>
    <param1>     null
 <Return>        Qlayout *,窗口正在使用的布局;
 <Note>          null
*************************************************************************/
QLayout *DFontBaseDialog::getContentLayout()
{
    return m_contentLayout;
}

/*************************************************************************
 <Function>      addContent
 <Description>   向窗口中添加控件
 <Author>        null
 <Input>
    <param1>     Qwidget*        Description:需要添加的控件
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontBaseDialog::addContent(QWidget *content)
{
    if (nullptr != getContentLayout()) {
        getContentLayout()->addWidget(content);
    }
}


/*************************************************************************
 <Function>      setIconPixmap
 <Description>   设置Iconlabel显示的图片内容
 <Author>        null
 <Input>
    <param1>     QPixmap  &        Description:需要显示的内容
 <Return>        null              Description:null
 <Note>          null
*************************************************************************/
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

/*************************************************************************
 <Function>      getCloseButton
 <Description>   获取右上角的关闭按钮
 <Author>        null
 <Input>         null
 <Return>        null
 <Note>          null
*************************************************************************/
DWindowCloseButton *DFontBaseDialog::getCloseButton() const
{
    return m_closeButton;
}

///*************************************************************************
// <Function>      getContent
// <Description>   获取窗口中使用主布局的控件
// <Author>        null
// <Input>         null
// <Return>        null
// <Note>          null
//*************************************************************************/
//QWidget *DFontBaseDialog::getContent() const
//{
//    return m_content;
//}

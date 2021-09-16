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

#include "dfdeletedialog.h"
#include "utils.h"
#include "globaldef.h"
#include "dfontmgrmainwindow.h"

#include <DApplication>
#include <DApplicationHelper>
#include <DFontSizeManager>
#include <DCheckBox>

#include <QHBoxLayout>
#include <QKeyEvent>

struct NewStr {
    QStringList strList; //每行的字符串
    QString resultStr; //添加'\n'后的字符串
    int fontHeifht = 0; //当前字体的高度
};

/**
 * @brief autoCutText 根据字符串长度和pDesLbl宽度适当添加'\n'
 * @param text
 * @param pDesLbl
 * @return
 */
NewStr autoCutText(const QString &text, DLabel *pDesLbl)
{
    if (text.isNull() || nullptr == pDesLbl) {
        return NewStr();
    }

    QFont font; // 应用使用字体对象
    QFontMetrics font_label(font);
    QString strText = text;
    int titlewidth = font_label.width(strText);
    QString str;
    NewStr newstr;
    int width = pDesLbl->width();
    if (titlewidth < width) {
        newstr.strList.append(strText);
        newstr.resultStr += strText;
    } else {
        for (int i = 0; i < strText.count(); i++) {
            str += strText.at(i);

            if (font_label.width(str) > width) { //根据label宽度调整每行字符数
                str.remove(str.count() - 1, 1);
                newstr.strList.append(str);
                newstr.resultStr += str + "\n";
                str.clear();
                --i;
            }
        }
        newstr.strList.append(str);
        newstr.resultStr += str;
    }
    newstr.fontHeifht = font_label.height();
    return newstr;
}

/*************************************************************************
 <Function>      DFDeleteDialog
 <Description>   构造函数：此类为字体删除确认页面，主要功能为确认删除字体和取消删除字体
 <Author>
 <Input>
    <param1>     win               Description:主页面实例对象
    <param2>     deleteCnt         Description:待删除字体数量
    <param3>     systemCnt         Description:选中系统字体的数量
    <param3>     hasCurrent        Description:是否包含当前系统在用字体
    <param3>     parent            Description:父对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFDeleteDialog::DFDeleteDialog(DFontMgrMainWindow *win, int deleteCnt, int systemCnt, bool hasCurrent, QWidget *parent)
    : DFontBaseDialog(parent)
    , m_mainWindow(win)
    , m_deleteCnt(deleteCnt)
    , m_systemCnt(systemCnt)
    , m_deleting(false)
    , m_hasCurFont(hasCurrent)
{
    initUI();
    initConnections();
    setTheme();
}

/*************************************************************************
 <Function>      initUI
 <Description>   初始化字体删除待确认页面
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFDeleteDialog::initUI()
{
    setFixedWidth(DEFAULT_WINDOW_W);

    initMessageTitle();
    initMessageDetail();
    QLayout *buttonsLayout = initBottomButtons();

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(10, 0, 10, 10);
    mainLayout->addWidget(messageTitle, 0, Qt::AlignCenter);
    mainLayout->addSpacing(6);
    mainLayout->addWidget(messageDetail, 0, Qt::AlignCenter);
    mainLayout->addSpacing(16);
    mainLayout->addLayout(buttonsLayout);

    QWidget *mainFrame = new QWidget(this);
    mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    messageDetail->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainFrame->setLayout(mainLayout);

    //#000794 解决显示不全的问题
    messageTitle->setMinimumWidth(DEFAULT_WINDOW_W - 20);
    messageDetail->setMinimumWidth(DEFAULT_WINDOW_W - 20);

    addContent(mainFrame);
}

/*************************************************************************
 <Function>      initConnections
 <Description>   初始化用于判断删除或取消操作的信号和槽的链接函数
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFDeleteDialog::initConnections()
{
    connect(m_cancelBtn, &DPushButton::clicked, this, [ = ]() {
        reject();
        close();
        emit m_signalManager->cancelDel();
    });
    connect(m_confirmBtn, &DPushButton::clicked, this, [ = ]() {
        if (m_deleting)
            return;
        m_deleting = true;
        accept();
        close();
    });

    //关闭删除确认对话框并且没有点击"确认"按钮时,取消删除操作
    connect(this, &DFDeleteDialog::closed, this, [ = ]() {
        if (m_mainWindow != nullptr && !m_deleting) {
            qDebug() << "cancel delte";
            m_mainWindow->cancelDelete();
            emit m_signalManager->clearRecoverList();
        }
    });
    connect(qApp, &DApplication::fontChanged, this, &DFDeleteDialog::onFontChanged);

    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged, this, &DFDeleteDialog::setTheme);
}

/*************************************************************************
 <Function>      initMessageTitle
 <Description>   初始化页面提示信息标题
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFDeleteDialog::initMessageTitle()
{
    messageTitle = new DLabel(this);
    messageTitle->setText(DApplication::translate("DeleteConfirmDailog", "Are you sure you want to delete %1 font(s)?").arg(m_deleteCnt));

    /* Bug#21515 UT000591*/
    messageTitle->setFixedWidth(DEFAULT_WINDOW_W - 22);
    messageTitle->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    messageTitle->setWordWrap(true);
    messageTitle->setAlignment(Qt::AlignCenter);

    DFontSizeManager::instance()->bind(messageTitle, DFontSizeManager::T6, QFont::Medium);

    DPalette pa = DApplicationHelper::instance()->palette(messageTitle);
    QColor color = pa.toolTipText().color();
    color.setAlphaF(0.9);
    pa.setColor(DPalette::WindowText, color);
    DApplicationHelper::instance()->setPalette(messageTitle, pa);
}

/*************************************************************************
 <Function>      initMessageDetail
 <Description>   初始化页面提示信息内容
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFDeleteDialog::initMessageDetail()
{
    messageDetail = new DLabel(this);

    if (m_systemCnt == 0 && !m_hasCurFont) {
        if (m_deleteCnt == 1) {
            messageDetail->setText(DApplication::translate("DeleteConfirmDailog", "This font will not be available to applications"));
        } else {
            messageDetail->setText(DApplication::translate("DeleteConfirmDailog", "These fonts will not be available to applications"));
        }
    } else if (m_systemCnt > 0 && !m_hasCurFont) {
        messageDetail->setText(DApplication::translate("DeleteConfirmDailog",
                                                       "The other %1 system fonts cannot be deleted").arg(m_systemCnt));
    } else if (m_systemCnt == 0 && m_hasCurFont) {
        messageDetail->setText(DApplication::translate("DeleteConfirmDailog",
                                                       "The font \"%1\" in use cannot be deleted").arg(m_mainWindow->getPreviewListView()->getCurFontStrName()));
    } else {
        messageDetail->setText(DApplication::translate("DeleteConfirmDailog",
                                                       "The other %1 system fonts and the font \"%2\" in use cannot be deleted").arg(m_systemCnt)
                               .arg(m_mainWindow->getPreviewListView()->getCurFontStrName()));
    }

    /* Bug#21515 UT000591*/
    messageDetail->setFixedWidth(DEFAULT_WINDOW_W - 22);
    messageDetail->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    messageDetail->setWordWrap(true);
    messageDetail->setAlignment(Qt::AlignCenter);

    DFontSizeManager::instance()->bind(messageDetail, DFontSizeManager::T6, QFont::Normal);
    DPalette pa = DApplicationHelper::instance()->palette(messageDetail);
    QColor color = pa.toolTipText().color();
    color.setAlphaF(0.7);
    pa.setColor(DPalette::WindowText, color);
    DApplicationHelper::instance()->setPalette(messageDetail, pa);
}

/*************************************************************************
 <Function>      initBottomButtons
 <Description>   初始化页面按钮
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
QLayout *DFDeleteDialog::initBottomButtons()
{
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    m_cancelBtn = new DPushButton(this);
    m_cancelBtn->setFixedSize(170, 36);
    m_cancelBtn->setText(DApplication::translate("DFDeleteDialog", "Cancel", "button"));

    m_confirmBtn = new DWarningButton(this);
    m_confirmBtn->setFixedSize(170, 36);
    m_confirmBtn->setText(DApplication::translate("DeleteConfirmDailog", "Delete", "button"));

    DVerticalLine *verticalSplite = new DVerticalLine(this);
    DPalette pa = DApplicationHelper::instance()->palette(verticalSplite);
    QColor splitColor = pa.color(DPalette::ItemBackground);
    pa.setBrush(DPalette::Background, splitColor);
    verticalSplite->setPalette(pa);
    verticalSplite->setBackgroundRole(QPalette::Background);
    verticalSplite->setAutoFillBackground(true);
    verticalSplite->setFixedSize(3, 28);

    layout->addWidget(m_cancelBtn);
    layout->addSpacing(9);
    layout->addWidget(verticalSplite);
    layout->addSpacing(9);
    layout->addWidget(m_confirmBtn);

    return layout;
}

/*************************************************************************
 <Function>      onFontChanged
 <Description>   适应系统字体变化
 <Author>
 <Input>
    <param1>     font            Description:当前系统在用字体
    <param2>     null            Description:
    <param3>     null            Description:
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFDeleteDialog::onFontChanged(const QFont &font)
{
    Q_UNUSED(font);
    /* Bug#20953 #21069  UT000591 */
    messageDetail->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    resize(sizeHint());
}

/*************************************************************************
 <Function>      keyPressEvent
 <Description>   响应键盘press事件中的esc按键
 <Author>
 <Input>
    <param1>     event           Description:事件对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFDeleteDialog::keyPressEvent(QKeyEvent *event)
{
    bool received = false;
    if (event->key() == Qt::Key_Escape) {
        reject();
        close();
        received = true;
    }
    if (event->key() == Qt::Key_Return) {
        if (!getCloseButton()->hasFocus() && !m_cancelBtn->hasFocus()) {
            m_confirmBtn->click();
            received = true;
        }
    }
    if (!received)
        QDialog::keyPressEvent(event);
}

/*************************************************************************
 <Function>      setTheme
 <Description>   根据主题设置页面颜色
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFDeleteDialog::setTheme()
{
    if (DApplicationHelper::DarkType == DApplicationHelper::instance()->themeType()) {
        DPalette pa = DApplicationHelper::instance()->palette(this);
        pa.setColor(DPalette::Background, QColor(25, 25, 25, 80));
        DApplicationHelper::instance()->setPalette(this, pa);
    } else {
        DPalette pa = DApplicationHelper::instance()->palette(this);
        pa.setColor(DPalette::Background, QColor(247, 247, 247, 80));
        DApplicationHelper::instance()->setPalette(this, pa);
    }
}

DFHandleTTCDialog::DFHandleTTCDialog(DFontMgrMainWindow *win, QString &file, QWidget *parent)
    : DFontBaseDialog(parent)
    , m_mainWindow(win)
    , fontset(file)
{

}

bool DFHandleTTCDialog::getDeleting()
{
    return m_confirm;
}

bool DFHandleTTCDialog::getAapplyToAll()
{
    return m_bAapplyToAll;
}

bool DFHandleTTCDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == applyAllCkb) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
                applyAllCkb->setChecked(!applyAllCkb->isChecked()); // 应用全部
                m_bAapplyToAll = applyAllCkb->isChecked();
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    } else {
        return DFontBaseDialog::eventFilter(watched, event);
    }
}

void DFHandleTTCDialog::keyPressEvent(QKeyEvent *event)
{
    bool received = false;
    if (event->key() == Qt::Key_Escape) {
        reject();
        close();
        received = true;
    }
    if (!received)
        DFontBaseDialog::keyPressEvent(event);
}

void DFHandleTTCDialog::initUI()
{
    setFixedWidth(DEFAULT_WINDOW_W);

    initMessageTitle();
    initMessageDetail();
    setIconPixmap(Utils::renderSVG("://exception-logo.svg", QSize(32, 32)));
    QLayout *buttonsLayout = initBottomButtons();

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(10, 0, 10, 10);
    mainLayout->addWidget(messageTitle, 0, Qt::AlignCenter);
    mainLayout->addSpacing(6);
    mainLayout->addWidget(applyAllCkb, 0, Qt::AlignCenter);
    mainLayout->addSpacing(16);
    mainLayout->addLayout(buttonsLayout);

    QWidget *mainFrame = new QWidget(this);
    mainFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainFrame->setLayout(mainLayout);

    //#000794 解决显示不全的问题
    messageTitle->setMinimumWidth(DEFAULT_WINDOW_W - 40); //删除/禁用/启用字体集提示窗口中文字边距设为20

    addContent(mainFrame);
    applyAllCkb->installEventFilter(this);

    moveToCenter();
}

void DFHandleTTCDialog::initConnections()
{
    connect(m_cancelBtn, &DPushButton::clicked, this, [ = ]() {
        reject();
        close();
    });
    connect(m_confirmBtn, &DPushButton::clicked, this, [ = ]() {
        m_confirm = true;
        accept();
        close();
    });

    connect(applyAllCkb, &DCheckBox::clicked, this, [ = ]() {
        m_bAapplyToAll = applyAllCkb->isChecked();
    });
}

void DFHandleTTCDialog::initMessageTitle()
{
    messageTitle = new DLabel(this);
    messageTitle->setObjectName("messageTitle");
    setMessageTitleText();

    /* Bug#21515 UT000591*/
    messageTitle->setFixedWidth(DEFAULT_WINDOW_W - 40); //删除/禁用/启用字体集提示窗口中文字边距设为20
    messageTitle->setAlignment(Qt::AlignCenter);

    DFontSizeManager::instance()->bind(messageTitle, DFontSizeManager::T6, QFont::Medium);
    messageTitle->setForegroundRole(DPalette::WindowText);

    autoFeed(messageTitle);
}

void DFHandleTTCDialog::initMessageDetail()
{
    applyAllCkb = new DCheckBox(tr("Apply to all selected font families"), this);
    applyAllCkb->setAccessibleName("Applyall_btn");
    DFontSizeManager::instance()->bind(applyAllCkb, DFontSizeManager::T6, QFont::Medium);
}

QLayout *DFHandleTTCDialog::initBottomButtons()
{
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    m_cancelBtn = new DPushButton(this);
    m_cancelBtn->setFixedSize(170, 36);
    m_cancelBtn->setText(tr("Cancel"));

    m_confirmBtn = new DWarningButton(this);
    m_confirmBtn->setFixedSize(170, 36);
    setConfirmBtnText();

    DVerticalLine *verticalSplite = new DVerticalLine(this);
    DPalette pa = DApplicationHelper::instance()->palette(verticalSplite);
    QColor splitColor = pa.color(DPalette::ItemBackground);
    pa.setBrush(DPalette::Background, splitColor);
    verticalSplite->setPalette(pa);
    verticalSplite->setBackgroundRole(QPalette::Background);
    verticalSplite->setAutoFillBackground(true);
    verticalSplite->setFixedSize(3, 28);

    layout->addWidget(m_cancelBtn);
    layout->addSpacing(9);
    layout->addWidget(verticalSplite);
    layout->addSpacing(9);
    layout->addWidget(m_confirmBtn);

    return layout;
}

void DFHandleTTCDialog::autoFeed(DLabel *label)
{
    NewStr newstr = autoCutText(m_messageTitleText, label);
    label->setText(newstr.resultStr);
    int height_lable = newstr.strList.size() * newstr.fontHeifht;
    label->setFixedHeight(height_lable);
    if (0 == m_iLabelOldHeight) { // 第一次exec自动调整
        adjustSize();
    } else {
        m_iDialogOldHeight = height();
        setFixedHeight(m_iDialogOldHeight - m_iLabelOldHeight + height_lable); //字号变化后自适应调整
    }
    m_iLabelOldHeight = height_lable;
}

void DFHandleTTCDialog::changeEvent(QEvent *event)
{
    if (QEvent::FontChange == event->type()) {
        Dtk::Widget::DLabel *p = findChild<Dtk::Widget::DLabel *>("messageTitle");
        if (nullptr != p) {
            autoFeed(p);
        }
    }
    DFontBaseDialog::changeEvent(event);
}

DFDeleteTTCDialog::DFDeleteTTCDialog(DFontMgrMainWindow *win, QString &file, QWidget *parent)
    : DFHandleTTCDialog(win, file, parent)
{
    initUI();
    initConnections();
}

void DFDeleteTTCDialog::setConfirmBtnText()
{
    if (m_confirmBtn) {
        m_confirmBtn->setText(tr("Delete", "button"));
    }
}

void DFDeleteTTCDialog::setMessageTitleText()
{
    m_messageTitleText = (tr("%1 is a font family, if you proceed, all fonts in it will be deleted").arg(fontset));
}

DFDisableTTCDialog::DFDisableTTCDialog(DFontMgrMainWindow *win, QString &file, bool &isEnable, QWidget *parent)
    : DFHandleTTCDialog(win, file, parent)
    , m_isEnable(isEnable)
{
    initUI();
    initConnections();
}
void DFDisableTTCDialog::setMessageTitleText()
{
    if (m_isEnable) {
        m_messageTitleText = (tr("%1 is a font family, if you proceed, all fonts in it will be enabled").arg(fontset));
    } else {
        m_messageTitleText = (tr("%1 is a font family, if you proceed, all fonts in it will be disabled").arg(fontset));
    }
}

void DFDisableTTCDialog::setConfirmBtnText()
{
    if (!m_confirmBtn) {
        return;
    }

    if (m_isEnable) {
        m_confirmBtn->setText(tr("Enable", "button"));
    } else {
        m_confirmBtn->setText(tr("Disable", "button"));
    }
}

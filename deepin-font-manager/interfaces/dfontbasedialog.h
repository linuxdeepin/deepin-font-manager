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

#ifndef DFONTBASEDIALOG_H
#define DFONTBASEDIALOG_H

#include <dabstractdialog.h>

#include <DLabel>
#include <DWindowCloseButton>

DWIDGET_USE_NAMESPACE

/*************************************************************************
 <Class>         DFMXmlWrapper
 <Description>   项目中弹出对话框的基类
 <Author>
 <Note>          null
*************************************************************************/
//class DWindowCloseButton;
//class DLabel;
class QVBoxLayout;
class DFontBaseDialog : public DAbstractDialog
{
    Q_OBJECT
public:
    explicit DFontBaseDialog(QWidget *parent = nullptr);
    ~DFontBaseDialog()Q_DECL_OVERRIDE;

    //窗口增加包含控件的函数
    void addContent(QWidget *content);

    //设置IconLabel的显示内容
    void setIconPixmap(const QPixmap &iconPixmap);

    //获取窗口中使用主布局的控件的函数
    QWidget *getContent() const;

    //获取右上角关闭按钮的函数
    DWindowCloseButton *getCloseButton() const;

protected:
    //初始化ui界面
    void initUI();
    //初始化信号与槽的链接
    void InitConnections();

    //设置IconLabel是否可见
    void setLogoVisable(bool visible = true);

    //设置titlelabel显示内容
    void setTitle(const QString &title);
    //获取窗口使用的布局
    QLayout *getContentLayout();

    //Overrides
    void closeEvent(QCloseEvent *event) override;
signals:
    //窗口关闭信号
    void closed();

    //点击右上角关闭按钮信号
    void closeBtnClicked();

public slots:
private:
    QWidget *m_titleBar {nullptr};
    DLabel  *m_logoIcon {nullptr};
    DLabel  *m_tileText {nullptr};
    DWindowCloseButton *m_closeButton {nullptr};

    QWidget *m_content {nullptr};
    QVBoxLayout *m_contentLayout {nullptr};
};

#endif // DFONTBASEDIALOG_H

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

#ifndef DFONTINFODIALOG_H
#define DFONTINFODIALOG_H
#include "dfontbasedialog.h"
#include "dfontinfoscrollarea.h"

DWIDGET_USE_NAMESPACE

//类声明
struct DFontPreviewItemData;
class FontIconText;

/*************************************************************************
 <Class>         DFontInfoDialog
 <Description>  字体管理器信息页面，在本页面查看字体图标、名称、样式、类型、版本和描述、字形、类别等信息。
 <Author>
 <Note>          null
*************************************************************************/
class DFontInfoDialog : public DFontBaseDialog
{
    Q_OBJECT
public:
    explicit DFontInfoDialog(DFontPreviewItemData *fontInfo, QWidget *parent = nullptr);
    //静态变量-长宽属性
    static constexpr int DEFAULT_WINDOW_W = 300;
    static constexpr int DEFAULT_WINDOW_H = 640;

protected:
    //初始化信息页面
    void initUI();
    //初始化信号与槽连接处理函数-主题变化信号和槽
    void initConnections();
    //用于nametitle的第三行判断，由AutoFeed调用
    QString adaptiveLengthForNameTitle(QFontMetrics fm, QString thirdLineText, int lineWidth);
    //重新实现键盘press事件-信息页面打开快捷键关闭本窗口
    void keyPressEvent(QKeyEvent *ev) override;
    //自适应变化信息详情页面高度
    void autoHeight(int height);
signals:

public slots:

private:
    QWidget *m_mainFrame {nullptr};
    QString m_FileName;
    QPoint m_faCenter;
    SignalManager *m_signalManager = SignalManager::instance();

    FontIconText *m_fontLogo {nullptr};
    DLabel *m_fontFileName {nullptr};

    QFrame *m_basicInfoFrame {nullptr};
    QString AutoFeed(QString text);
    QVBoxLayout *m_baseicInfoLayout;

    DFontPreviewItemData *m_fontInfo;

    dfontinfoscrollarea *m_fontinfoArea;
    DScrollArea *m_scrollArea;
};

#endif  // DFONTINFODIALOG_H

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

#ifndef DFONTPREVIEWER_H
#define DFONTPREVIEWER_H

#include <DFrame>

DWIDGET_USE_NAMESPACE

/*************************************************************************
 <Class>         DFontPreviewer
 <Description>   负责控制quickinstallwindow中预览字体显示内容
 <Author>
 <Note>          null
*************************************************************************/
class DFontPreviewer : public QWidget
{
    Q_OBJECT
public:
    explicit DFontPreviewer(QWidget *parent = nullptr);
    //绘制预览字体
    void paintEvent(QPaintEvent *event) override;

    //初始化数据
    void InitData();

    //初始化链接
    void InitConnections();

    //设置当前字体路径
    void setPreviewFontPath(const QString font);

signals:
    //预览字体变化信号
    void previewFontChanged();

public slots:
    //预览字体变化后的处理
    void onPreviewFontChanged();

private:
    QString m_fontPath;
    QStringList m_previewTexts;
};

#endif // DFONTPREVIEWER_H

/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
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

#ifndef DFONTWIDGET_H
#define DFONTWIDGET_H

#include <QWidget>
#include <DScrollArea>
#include <QStackedLayout>
#include "dspinner.h"
#include "dfontpreview.h"
#include "dfontloadthread.h"
#include <DApplication>
#include <QScreen>
#include <QGuiApplication>
#include <QScrollBar>
#include <QTranslator>

DWIDGET_USE_NAMESPACE

class DFontWidget : public QWidget
{
    Q_OBJECT

public:
    DFontWidget(QWidget *parent = nullptr);
    ~DFontWidget() override;

    void setFileUrl(const QString &url);

protected:

private:
    void handleFinished(const QByteArray &data);

private:
    QStackedLayout *m_layout;
    DFontPreview *m_preview;
    DScrollArea *m_area;
    DFontLoadThread *m_thread;
    DSpinner *m_spinner;
    QString m_filePath;
    QLabel *m_lab;
    QTranslator m_translator;
};

#endif

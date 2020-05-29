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

#include "dfontwidget.h"
#include <QTimer>

DFontWidget::DFontWidget(QWidget *parent)
    : QWidget(parent),
      m_layout(new QStackedLayout(this)),
      m_preview(new DFontPreview(this)),
      m_thread(new DFontLoadThread(this)),
      m_spinner(new DSpinner(this)),
      m_lab(new QLabel(this))/*UT000539 暂时固定文本，确认文案后修改*/
{
    QWidget *spinnerPage = new QWidget;
    QVBoxLayout *spinnerLayout = new QVBoxLayout(spinnerPage);
    m_spinner->setFixedSize(50, 50);
    spinnerLayout->addWidget(m_spinner, 0, Qt::AlignCenter);

    m_layout->addWidget(spinnerPage);

    /*增加滚动条功能 UT000539*/
    m_area = new DScrollArea(this);
    m_area->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    m_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_area->setWidgetResizable(true);
    m_area->setWidget(m_preview);

    QPalette pal = palette();
    pal.setColor(QPalette::Background, QColor(0x00, 0xff, 0x00, 0x00));
    m_area->setPalette(pal);
    setPalette(pal);

    m_area->setFrameShape(QFrame::Shape::NoFrame);

    m_layout->addWidget(m_area);

    connect(m_thread, &DFontLoadThread::loadFinished, this, &DFontWidget::handleFinished);

    connect(qApp, &DApplication::fontChanged, this, [ = ]() {
        m_lab->setFont(DApplication::font());
    });

    m_area->setFixedSize(qApp->primaryScreen()->geometry().width() / 1.5,
                         qApp->primaryScreen()->geometry().height() / 1.5 + 20);
}

DFontWidget::~DFontWidget()
{
}

void DFontWidget::setFileUrl(const QString &url)
{
    m_filePath = url;

    m_layout->setCurrentIndex(0);
    m_spinner->start();

    m_preview->fontDatabase.removeAllApplicationFonts();
    m_thread->quit();
    m_thread->open(url);
    m_thread->start();
}

void DFontWidget::handleFinished(const QByteArray &data)
{
    /*UT000539 字体损坏弹出提示隐藏view*/
    if (m_preview->fontDatabase.addApplicationFontFromData(data) == -1) {
        m_spinner->stop();
        m_spinner->hide();
        m_preview->hide();
        m_lab->setText(DApplication::translate("fontpreview", "Broken file"));
        m_lab->setFont(DApplication::font());
        m_lab->move(this->geometry().center() - m_lab->rect().center());
        m_lab->show();
        return;
    }
    if (m_lab->isVisible())
        m_lab->hide();
    m_preview->setFileUrl(m_filePath);
    m_layout->setCurrentIndex(1);
    m_spinner->stop();
    m_preview->show();
    m_area->horizontalScrollBar()->setSliderPosition(0);

}

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

#include "dfontloadthread.h"
#include <QFile>

DFontLoadThread::DFontLoadThread(QObject *parent)
    : QThread(parent)
{

}

DFontLoadThread::~DFontLoadThread()
{
}

void DFontLoadThread::open(const QString &filepath)
{
    m_filePath = filepath;
}

void DFontLoadThread::run()
{
    QFile file(m_filePath);

    if (file.open(QIODevice::ReadOnly)) {
        QByteArray fileContent = file.readAll();

        emit loadFinished(fileContent);
    } else {
        emit loadFinished(QByteArray());
    }

    file.close();
}

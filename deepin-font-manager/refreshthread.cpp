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

#include "refreshthread.h"
#include <QProcess>
#include <QDebug>

RefreshThread::RefreshThread(QObject *parent)
    : QThread(parent)
{

}

RefreshThread::~RefreshThread()
{
}

void RefreshThread::run()
{
    qDebug() << "Refreshing font cache.";

    QProcess *process = new QProcess;
    process->start("fc-cache", QStringList() << "-v");
    process->waitForFinished(-1);
    process->deleteLater();

    qDebug() << "Refresh font cache finished.";

    emit refreshFinished();
}

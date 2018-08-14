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

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QDir>
#include <QDebug>
#include <QThread>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.process(app);

    const QStringList fileList = parser.positionalArguments();
    std::cout << 0 << std::endl;

    bool isFaild = false;
    QProcess process;
    process.start("cp", QStringList() << "-f" << fileList.first() << fileList.last());
    process.waitForFinished();
    isFaild |= process.exitCode();

    // the file is readable by the owner of the file.
    QFile::setPermissions(fileList.last(), QFileDevice::ReadOwner | QFileDevice::ReadGroup | QFileDevice::ReadOther);


    if (!isFaild) {
        process.start("fc-cache");
        process.waitForFinished();
    }

    QThread::msleep(300);
    std::cout << 1 << std::endl;

    return 0;
}

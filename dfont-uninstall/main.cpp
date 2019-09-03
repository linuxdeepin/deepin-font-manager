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

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QThread>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.process(app);

    const QStringList fileList = parser.positionalArguments();
    std::cout << 0 << std::endl;

    QDir userFontDir("/usr/share/fonts/");
    QDir systemFontDir("/usr/share/fonts/deepin-font-install");

    for (QString file : fileList) {
        QFileInfo openFile(file);

        QDir fileDir(openFile.path());

        // For security, check the font dir is valid
        if (userFontDir == fileDir || systemFontDir == fileDir) {
#ifdef QT_DEBUG
            qDebug() << "Invalid dir:" << fileDir.path();
#endif
            continue;
        }

        if (fileDir.removeRecursively()) {
#ifdef QT_DEBUG
            qDebug() << "Delete font ok:" << fileDir.path() << " " << openFile.completeSuffix();
#endif
            QProcess process;
            process.start("fc-cache");
            process.waitForFinished();
        }
    }

    QThread::msleep(300);
    std::cout << 1 << std::endl;

    return 0;
}

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
#include <QJsonObject>
#include <QJsonDocument>
#include <QThread>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QDir>
#include <QDebug>
#include <iostream>

inline void checkDirectory()
{
    const QString path(QDir::homePath() + "/.local/share/fonts");
    if (!QDir(path).exists()) {
        QDir dir(path);
        dir.mkpath(".");
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.process(app);
    checkDirectory();

    const QString sysDir = QDir::homePath() + "/.local/share/fonts";
    const QStringList fileList = parser.positionalArguments();

    QProcess *process = new QProcess;
    QString target = "";
    QString targetDir = "";

    for ( QString file : fileList) {
        QStringList fileParamList = file.split("|");
        QString filePathOrig = fileParamList.at(0);
        QString familyName = fileParamList.at(1);

        const QFileInfo info(filePathOrig);
        QString dirName = familyName;

        if (dirName.isEmpty()) {
            dirName = info.baseName();
        }

        target = QObject::tr("%1/%2/%3").arg(sysDir).arg(dirName).arg(info.fileName());
        targetDir = QObject::tr("%1/%2").arg(sysDir).arg(dirName);

        QDir dir(targetDir);
        dir.mkpath(".");
        QFile::copy(filePathOrig, target);

        // set permission.
        process->start("chmod", QStringList() << "644" << target);
        process->waitForFinished(-1);

        const int currentIndex = fileList.indexOf(file);
        const int count = fileList.count() - 1;

        if (fileList.count() == 1) {
            std::cout << target.toUtf8().data() << std::endl;
            QThread::msleep(50);
        } else {
            QJsonObject object;
            object.insert("FilePath", filePathOrig);
            object.insert("Percent", currentIndex / double(count) * 100);

            QJsonDocument document;
            document.setObject(object);
            QByteArray array = document.toJson(QJsonDocument::Compact);

            std::cout << array.data() << std::endl;

            // output too fast will crash.
            QThread::msleep(10);
        }
    }

    process->start("fc-cache");
    process->waitForFinished();
    process->deleteLater();

    return 0;
}

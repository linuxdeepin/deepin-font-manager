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

#include "dfontmanager.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QFileInfo>
#include <QDir>

static DFontManager *INSTANCE = nullptr;
const QString sysDir = QDir::homePath() + "/.local/share/fonts";

DFontManager *DFontManager::instance()
{
    if (!INSTANCE) {
        INSTANCE = new DFontManager;
    }

    return INSTANCE;
}

DFontManager::DFontManager(QObject *parent)
    : QThread(parent)
{
}

DFontManager::~DFontManager() {}

void DFontManager::setType(Type type)
{
    qDebug() << type << endl;
    m_type = type;
}

void DFontManager::setInstallFileList(const QStringList &list)
{
    if (!m_instFileList.isEmpty()) {
        m_instFileList.clear();
    }

    m_instFileList << list;
}

void DFontManager::setReInstallFile(const QString &reinstFile, const QString &sysFile)
{
    m_reinstFile = reinstFile;
    m_sysFile = sysFile;
}

void DFontManager::setUnInstallFile(const QStringList &filePath)
{
    m_uninstFile = filePath;
}

void DFontManager::handleInstallOutput()
{
    QProcess *process = dynamic_cast<QProcess *>(sender());
    qDebug() << process->processId();
    QString output = process->readAllStandardOutput();

    // single file installation.
    if (m_instFileList.count() == 1) {
        emit installPositionChanged(output);
    } else {
        // FIXME(Rekols): this operation is required under the Loongson platform.
        for (const QString &line : output.split("\n")) {
            QJsonDocument document = QJsonDocument::fromJson(line.toUtf8());
            QJsonObject object = document.object();

            emit batchInstall(object.value("FilePath").toString(),
                              object.value("Percent").toDouble());
        }
    }
}

void DFontManager::handleReInstallOutput()
{
    QProcess *process = dynamic_cast<QProcess *>(sender());
    qDebug() << process->processId();
    QString output = process->readAllStandardOutput();
    // 0 is installing.
    if (output.toInt() == 0) {
        emit reinstalling();
    } else {
        emit reinstallFinished();
    }
}

void DFontManager::handleUnInstallOutput()
{
    QProcess *process = dynamic_cast<QProcess *>(sender());
    qDebug() << process->processId();
    QString output = process->readAllStandardOutput();
    if (output.toInt() == 0) {
        emit uninstalling();
    } else {
        emit uninstallFinished();
    }
}

void DFontManager::run()
{
    qDebug() << m_type << endl;
    switch (m_type) {
    case Install:
        handleInstall();
        break;
    case ReInstall:
        handleReInstall();
        break;
    case UnInstall:
        handleUnInstall();
        break;
    default:
        break;
    }
}

bool DFontManager::doCmd(const QString &program, const QStringList &arguments)
{
    int failed = false;
    qDebug() << "QProcess start";
    qDebug() << m_type << endl;
    switch (m_type) {
    case Install:
        doInstall(arguments);
        break;

    case ReInstall:
        doInstall(arguments, true);
        break;

    case UnInstall:
        doUninstall(arguments);
        break;
    default:
        break;
    }

    return !failed;
}

void DFontManager::handleInstall()
{
    //if (doCmd("pkexec", QStringList() << "dfont-install" << m_instFileList)) {

    if (doCmd("dfont-install", QStringList() << m_instFileList)) {
        if (m_instFileList.count() == 1) {
            // emit installFinished();
        }

        Q_EMIT installFinished(0, m_installOutList);
    } else {
        // For:unathorized exit

        Q_EMIT installFinished(127, QStringList());
    }
}

void DFontManager::handleUnInstall()
{
    qDebug() << "waitForFinished";
    if (doCmd("dfont-uninstall", QStringList() << m_uninstFile)) {
        emit uninstallFinished();

        emit uninstallFontFinished(m_uninstFile);
    }

}

void DFontManager::handleReInstall()
{

    if (doCmd("dfont-install", QStringList() << m_instFileList)) {
        if (m_instFileList.count() == 1) {
            // emit installFinished();
        }

        emit reInstallFinished(0, m_installOutList);
//        qDebug() << "ASDASDasd" << endl;
    } else {
        // For:unathorized exit

        Q_EMIT reInstallFinished(127, QStringList());
    }
}

void DFontManager::setSystemFontCount(int systemFontCount)
{
    m_systemFontCount = systemFontCount;
}

// install fileList fonts
void DFontManager::doInstall(const QStringList &fileList, bool reinstall)
{
    QString target = "";
    QString targetDir = "";

    m_installOutList.clear();
    for (QString file : fileList) {
        QStringList fileParamList = file.split("|");
        QString filePathOrig = fileParamList.at(0);
        QString familyName = fileParamList.at(1);
        const QFileInfo info(filePathOrig);
        QString dirName = familyName;

        if (dirName.isEmpty()) {
            dirName = info.baseName();
        }

        target = QString("%1/%2/%3").arg(sysDir).arg(dirName).arg(info.fileName());
        targetDir = QString("%1/%2").arg(sysDir).arg(dirName);

        QDir dir(targetDir);
        dir.mkpath(".");
        QFile::copy(filePathOrig, target);
        m_installOutList << target;

        const int currentIndex = fileList.indexOf(file);
        const int count = fileList.count() - 1;

        if (fileList.count() == 1) {
//            qDebug() << __FUNCTION__ << target.toUtf8().data();
            if (!reinstall)
                Q_EMIT installPositionChanged(target.toUtf8().data());
//            QThread::msleep(50);
        } else {
            QString filePath = filePathOrig;
            double percent = currentIndex / double(count) * 100;

//            qDebug() << __FUNCTION__ << filePath << ", " << percent;
            if (!reinstall) {
                Q_EMIT batchInstall(filePath, percent);
            } else {

            }

            // output too fast will crash.
//            QThread::msleep(10);

        }
    }

    QProcess process;
    process.start("fc-cache");
    process.waitForFinished();
}

void DFontManager::doUninstall(const QStringList &fileList)
{
    Q_EMIT uninstalling();
    for (QString file : fileList) {
        QFileInfo openFile(file);

        QDir fileDir(openFile.path());

        QDir userFontDir("/usr/share/fonts/");
        QDir systemFontDir(QDir::homePath() + "/.local/share/fonts");
        // For security, check the font dir is valid
        if (userFontDir == fileDir || systemFontDir == fileDir) {
#ifdef QT_DEBUG
            qDebug() << "Invalid dir:" << fileDir.path();
#endif
            continue;
        }

        QFile::remove(file);

        //Fonts with same family name, different style may be
        //installed in same dir, so only delete dir when it's
        //empty
        if (fileDir.isEmpty()) {
            fileDir.removeRecursively();
        }

#ifdef QT_DEBUG
//        qDebug() << "Delete font ok:" << fileDir.path() << " " << openFile.completeSuffix();
#endif
    }

    QProcess process;
    process.start("fc-cache");
    process.waitForFinished();

    QThread::msleep(30);
    Q_EMIT uninstallFinished();
}

bool DFontManager::getIsWaiting() const
{
    return isWaiting;
}

void DFontManager::setIsWaiting(bool value)
{
    isWaiting = value;
}

void DFontManager::handleProcessFinished(int exitCode)
{
    QProcess *process = dynamic_cast<QProcess *>(sender());
    qDebug() << process->processId();
    qDebug() << exitCode << endl;
    process->deleteLater();
}

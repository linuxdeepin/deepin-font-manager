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
//    connect(this, &QThread::finished, [ = ] {
//        qDebug() << QThread::currentThreadId() << endl;
//        qDebug() << "########## finished";
//    });
//    connect(this, &QThread::started, [ = ] {
//        qDebug() << "########## started";
//    });
}

DFontManager::~DFontManager() {}

void DFontManager::setType(Type type)
{
    qDebug() << type << endl;
    m_type = type;
}

void DFontManager::setInstallFileList(const QStringList &list)
{
    qDebug() << __FUNCTION__ << "start" << endl;
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
    /*    QProcess *process = dynamic_cast<QProcess *>(sender());
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
                m_installOutList << object.value("TargetPath").toString();

                emit batchInstall(object.value("FilePath").toString(),
                                  object.value("Percent").toDouble());
            }
        }*/
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
    qDebug() << __FUNCTION__ << "start" << m_type << endl;
    switch (m_type) {
    case Install:
        handleInstall();
        break;
    case HalfwayInstall:
        handleInstall(true);
        break;
    case ReInstall:
        handleReInstall();
        break;
    case UnInstall:
        handleUnInstall();
        break;
    }
}

bool DFontManager::doCmd(const QStringList &arguments)
{
//    QProcess *process = new QProcess;
    int failed = false;
    qDebug() << "QProcess start";
    qDebug() << m_type << endl;
    switch (m_type) {
    case Install:
        /*connect(process, SIGNAL(readyReadStandardOutput()), this,
                SLOT(handleInstallOutput()));*/
        doInstall(arguments);
        break;

    case ReInstall:
        /*connect(process, SIGNAL(readyReadStandardOutput()), this,
                SLOT(handleReInstallOutput()));*/
        doInstall(arguments, true);
        break;
    case HalfwayInstall:
        doInstall(arguments);
        break;
    case UnInstall:
        /*connect(process, SIGNAL(readyReadStandardOutput()), this,
                SLOT(handleUnInstallOutput()));*/
        doUninstall(arguments);
        break;
    }

    /*    connect(process, SIGNAL(finished(int)), this, SLOT(handleProcessFinished(int)));

        process->start(program, arguments);
        process->waitForFinished(-1);

        failed |= process->exitCode();
    */
    return !failed;
}

void DFontManager::handleInstall(bool isHalfwayInstall)
{
    //if (doCmd("pkexec", QStringList() << "dfont-install" << m_instFileList)) {

    if (doCmd(QStringList() << m_instFileList)) {
        if (m_instFileList.count() == 1) {
            // emit installFinished();
        }
        if (!isHalfwayInstall) {
            Q_EMIT installFinished(InstallStatus::InstallSuccess, m_installOutList);
        } else {
            Q_EMIT installFinished(InstallStatus::HalfwayInstallSuccess, m_installOutList);
        }
    } else {
        // For:unathorized exit

        Q_EMIT installFinished(127, QStringList());
    }
    //clear
    m_instFileList.clear();
    m_installOutList.clear();
}

void DFontManager::handleUnInstall()
{
    qDebug() << "waitForFinished";
    if (doCmd(QStringList() << m_uninstFile)) {
        emit uninstallFinished();

        emit uninstallFontFinished(m_uninstFile);
    }
    //clear
    m_uninstFile.clear();
}

void DFontManager::handleReInstall()
{

    if (doCmd(QStringList() << m_instFileList)) {
        if (m_instFileList.count() == 1) {
            // emit installFinished();
        }

        emit reInstallFinished(0, m_installOutList);
//        qDebug() << "ASDASDasd" << endl;
    } else {
        // For:unathorized exit

        Q_EMIT reInstallFinished(127, QStringList());
    }
    //clear
    m_instFileList.clear();
    m_installOutList.clear();
}

void DFontManager::setSystemFontCount(int systemFontCount)
{
    m_systemFontCount = systemFontCount;
}

// install fileList fonts
void DFontManager::doInstall(const QStringList &fileList, bool reinstall)
{
    Q_UNUSED(reinstall);
    qDebug() << __func__ << "s" << endl;

    QString target = "";
    QString targetDir = "";

    m_installOutList.clear();
    for (const QString &file : fileList) {
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
//            if (!reinstall)
//                Q_EMIT installPositionChanged(target.toUtf8().data());
//            QThread::msleep(50);
            //  ut000442 之前安装一个字体时没有安装进度条,在此添加
            Q_EMIT batchInstall(familyName, 100);

        } else {
            QString filePath = filePathOrig;
            double percent = currentIndex / double(count) * 100;

//            qDebug() << __FUNCTION__ << filePath << ", " << percent;
//            if (!reinstall) {
            Q_EMIT batchInstall(familyName, percent);
//            } else {

//            }

            // output too fast will crash.
//            QThread::msleep(10);

        }
    }

//    QProcess process;

//    process.start("fc-cache");
//    process.waitForFinished(50);

    //之前使用了waitforfinished这个函数，这个函数造成了没有必要的堵塞，导致后面流程受阻，现在换成下面这个函数，暂时没有发现问题。
    QProcess::startDetached("fc-cache");

//    if (!reinstall) {
//        QString filename;
//        if (!fileList.isEmpty())
//            filename = fileList.last();
////        Q_EMIT batchInstall(filename, 96);
//    }
}

void DFontManager::doUninstall(const QStringList &fileList)
{
    Q_EMIT uninstalling();
    for (const QString &file : fileList) {
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

//    QProcess process;
//    process.start("fc-cache");
//    process.waitForFinished();

//发现开机后先删除字体再安装字体时，偶现安装进程无法启动，修改这里后现象消失
    QProcess::startDetached("fc-cache");


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

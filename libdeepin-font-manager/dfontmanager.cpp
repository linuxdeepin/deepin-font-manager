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

static DFontManager *INSTANCE = 0;

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

void DFontManager::setUnInstallFile(const QString &filePath)
{
    m_uninstFile = filePath;
}

void DFontManager::handleInstallOutput(const QString &output)
{
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

void DFontManager::handleReInstallOutput(const QString &output)
{
    // 0 is installing.
    if (output.toInt() == 0) {
        emit reinstalling();
    } else {
        emit reinstallFinished();
    }
}

void DFontManager::handleUnInstallOutput(const QString &output)
{
    if (output.toInt() == 0) {
        emit uninstalling();
    } else {
        emit uninstallFinished();
    }
}

void DFontManager::run()
{
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
    QProcess *process = new QProcess;
    int failed = false;

    switch (m_type) {
    case Install:
        connect(process, &QProcess::readyReadStandardOutput, this,
                [=] { handleInstallOutput(process->readAllStandardOutput()); });
        break;

    case ReInstall:
        connect(process, &QProcess::readyReadStandardOutput, this,
                [=] { handleReInstallOutput(process->readAllStandardOutput()); });
        break;

    case UnInstall:
        connect(process, &QProcess::readyReadStandardOutput, this,
                [=] { handleUnInstallOutput(process->readAllStandardOutput()); });
        break;
    }

    process->start(program, arguments);
    process->waitForFinished(-1);
    failed |= process->exitCode();
    process->deleteLater();

    return !failed;
}

void DFontManager::handleInstall()
{
    if (doCmd("pkexec", QStringList() << "dfont-install" << m_instFileList)) {
        if (m_instFileList.count() == 1) {
            // emit installFinished();
        }
        Q_EMIT installFinished(0);
    } else {
        // For:unathorized exit
        Q_EMIT installFinished(127);
    }
}

void DFontManager::handleUnInstall()
{
    if (doCmd("pkexec", QStringList() << "dfont-uninstall" << m_uninstFile)) {
        emit uninstallFinished();
    }
}

void DFontManager::handleReInstall()
{
    if (doCmd("pkexec", QStringList() << "dfont-install" << m_reinstFile)) {
        emit reinstallFinished();
    }
}

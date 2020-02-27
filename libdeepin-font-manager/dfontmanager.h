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

#ifndef DFONTMANAGER_H
#define DFONTMANAGER_H

#include <QThread>
#include <QModelIndex>

class DFontManager : public QThread
{
    Q_OBJECT

public:
    enum Type { Install, ReInstall, UnInstall };

    static DFontManager *instance();
    DFontManager(QObject *parent = nullptr);
    ~DFontManager();

    void setType(Type type);
    void setInstallFileList(const QStringList &list);
    void setReInstallFile(const QString &reinstFile, const QString &sysFile);
    void setUnInstallFile(const QStringList &filePath);

private slots:
    void handleInstallOutput();
    void handleReInstallOutput();
    void handleUnInstallOutput();
    void handleProcessFinished(int exitCode);

signals:
    void batchInstall(const QString &filePath, const double &percent);
    void installPositionChanged(const QString &instPath);
    void reinstalling();
    void uninstalling();
    void installFinished(int state);
    void reinstallFinished();
    void uninstallFinished();
    void uninstallFontFinished(const QStringList &uninstallIndex);

protected:
    void run();

private:
    bool doCmd(const QString &program, const QStringList &arguments);
    void handleInstall();
    void handleUnInstall();
    void handleReInstall();

private:
    QStringList m_instFileList;
    QStringList m_uninstFile;
    QString m_reinstFile;
    QString m_sysFile;
    Type m_type;
};

#endif

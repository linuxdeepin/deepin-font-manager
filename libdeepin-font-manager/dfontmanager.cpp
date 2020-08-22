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

/*************************************************************************
 <Function>      instance
 <Description>   获取字体管理线程类的单例对象
 <Author>
 <Input>         Null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFontManager *DFontManager::instance()
{
    if (!INSTANCE) {
        INSTANCE = new DFontManager;
    }

    return INSTANCE;
}

/*************************************************************************
 <Function>      DFontManager
 <Description>   构造函数-字体管理线程类
 <Author>
 <Input>
    <param1>     parent          Description:父类对象
 <Return>        DFontManager    Description:返回字体管理器线程类对象
 <Note>          null
*************************************************************************/
DFontManager::DFontManager(QObject *parent)
    : QThread(parent)
{

}

/*************************************************************************
 <Function>      ~DFontManager
 <Description>   析构函数-析构字体管理线程类对象
 <Author>
 <Input>
    <param1>     parent          Description:父类对象
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
DFontManager::~DFontManager() {}

/*************************************************************************
 <Function>      setType
 <Description>   设置线程执行类型
 <Author>
 <Input>
    <param1>     type            Description:类型参数
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontManager::setType(Type type)
{
    qDebug() << type << endl;
    m_type = type;
}

/*************************************************************************
 <Function>      setInstallFileList
 <Description>   传入待安装字体列表
 <Author>
 <Input>
    <param1>     list            Description:待安装字体列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontManager::setInstallFileList(const QStringList &list)
{
    qDebug() << __FUNCTION__ << "start" << endl;
    if (!m_instFileList.isEmpty()) {
        m_instFileList.clear();
    }

    m_instFileList << list;
}

/*************************************************************************
 <Function>      setUnInstallFile
 <Description>   传入待删除字体列表
 <Author>
 <Input>
    <param1>     filePath        Description:待删除字体列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontManager::setUnInstallFile(const QStringList &filePath)
{
    m_uninstFile = filePath;
}

/*************************************************************************
 <Function>      run
 <Description>   线程执行入口函数-安装、中途安装、重装与卸载
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontManager::run()
{
    qDebug() << __FUNCTION__ << "start" << m_type << endl;
    switch (m_type) {
    case Install:
    case HalfwayInstall:
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

/*************************************************************************
 <Function>      doCmd
 <Description>   线程执行函数
 <Author>
 <Input>
    <param1>     arguments       Description:动作执行参数列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontManager::doCmd(const QStringList &arguments)
{
    qDebug() << "QProcess start";
    qDebug() << m_type << endl;
    switch (m_type) {
    case Install:
    case ReInstall:
    case HalfwayInstall:
        doInstall(arguments);
        break;
    case UnInstall:
        doUninstall(arguments);
        break;
    default:
        break;
    }
}

/*************************************************************************
 <Function>      handleInstall
 <Description>   字体安装-函数入口
 <Author>
 <Input>
    <param1>     null
 <Return>        null             Description:null
 <Note>          null
*************************************************************************/
void DFontManager::handleInstall()
{
    doCmd(QStringList() << m_instFileList);
    if (m_instFileList.count() == 1) {
        // emit installFinished();
    }

    Q_EMIT installFinished(InstallStatus::InstallSuccess, m_installOutList);

    if (m_CacheStatus == CacheNow && !m_installCanceled) {
        doCache();
    }

    //clear
    m_instFileList.clear();
    m_installOutList.clear();
}

/*************************************************************************
 <Function>      handleUnInstall
 <Description>   字体卸载-函数入口
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontManager::handleUnInstall()
{
    qDebug() << "waitForFinished";
    doCmd(QStringList() << m_uninstFile);
//        qDebug() << __FUNCTION__ << m_uninstFile.size();
//        emit uninstallFontFinished(m_uninstFile);
    //clear
    m_uninstFile.clear();
}

/*************************************************************************
 <Function>      handleReInstall
 <Description>   字体重装-函数入口
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontManager::handleReInstall()
{
    doCmd(QStringList() << m_instFileList);
    if (m_instFileList.count() == 1) {
        // emit installFinished();
    }

    emit reInstallFinished(0, m_installOutList);

    if (m_CacheStatus != NoNewFonts || !m_instFileList.isEmpty())
        doCache();
    //clear
    m_instFileList.clear();
    m_installOutList.clear();
}

/*************************************************************************
 <Function>      doInstall
 <Description>   字体安装-具体执行函数
 <Author>
 <Input>
    <param1>     fileList        Description:待安装字体列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontManager::doInstall(const QStringList &fileList)
{
    qDebug() << __func__ << "s" << endl;

    QString target = "";
    QString targetDir = "";

    m_installOutList.clear();
    m_installCanceled = false;

    for (const QString &file : fileList) {
        if (m_installCanceled) {
            break;
        }

        QStringList fileParamList = file.split("|");
        QString filePathOrig = fileParamList.at(0);
        QString familyName = fileParamList.at(1);

        //这里有过familyname中带有 /  的话，创建的目录会多一层，导致与其他不统一，也会造成删除时删除不完全的问题
        if (familyName.contains("/")) {
            familyName.replace("/", "-");
        }
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
            //  ut000442 之前安装一个字体时没有安装进度条,在此添加
            Q_EMIT batchInstall(familyName, 100);

        } else {
            QString filePath = filePathOrig;
            double percent = currentIndex / double(count) * 100;

            Q_EMIT batchInstall(familyName, percent);
        }
    }

    //delete installed fonts to prevent next time install take long time
    if (!m_installCanceled) {
        return;
    }

    for (const QString &file : fileList) {
        QStringList fileParamList = file.split("|");
        QString filePathOrig = fileParamList.at(0);
        QString familyName = fileParamList.at(1);
        //这里有过familyname中带有 /  的话，创建的目录会多一层，导致与其他不统一，也会造成删除时删除不完全的问题
        if (familyName.contains("/")) {
            familyName.replace("/", " ");
        }
        const QFileInfo info(filePathOrig);
        QString dirName = familyName;
        target = QString("%1/%2/%3").arg(sysDir).arg(dirName).arg(info.fileName());
        QFile::remove(target);
        QDir fileDir(QFileInfo(target).path());
        if (fileDir.isEmpty()) {
            fileDir.removeRecursively();
        }
    }
    Q_EMIT requestCancelInstall();
}

/*************************************************************************
 <Function>      doUninstall
 <Description>   字体卸载-具体执行函数
 <Author>
 <Input>
    <param1>     fileList        Description:待卸载字体列表
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontManager::doUninstall(const QStringList &fileList)
{
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

    //修复删除时间有时候长的问题, 35479
    Q_EMIT uninstallFontFinished(m_uninstFile);
    qDebug() << __FUNCTION__ << m_uninstFile.size();

//    QProcess process;
//    process.start("fc-cache");
//    process.waitForFinished();

//发现开机后先删除字体再安装字体时，偶现安装进程无法启动，修改这里后现象消失
    bool ret = QProcess::startDetached("fc-cache");
    Q_EMIT uninstallFcCacheFinish();
    qDebug() << __FUNCTION__ << ret;
}

/*************************************************************************
 <Function>      setCacheStatus
 <Description>   设置fc-cache命令执行的状态
 <Author>
 <Input>
    <param1>     CacheStatus     Description:命令执行状态的枚举
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontManager::setCacheStatus(const CacheStatus &CacheStatus)
{
    m_CacheStatus = CacheStatus;
}

/*************************************************************************
 <Function>      stop
 <Description>   更新线程停止状态标志位
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontManager::cancelInstall()
{
    m_installCanceled = true;
}

/*************************************************************************
 <Function>      doCache
 <Description>   执行fc-cache命令
 <Author>
 <Input>         null
 <Return>        null            Description:null
 <Note>          null
*************************************************************************/
void DFontManager::doCache()
{
    qDebug() << __FUNCTION__;
    QProcess process;
    process.start("fc-cache");
    process.waitForFinished(-1);
    emit  cacheFinish();

}

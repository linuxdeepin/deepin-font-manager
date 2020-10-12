/*****************************************************************************
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     Xiao Zhiguo <xiaozhiguo@uniontech.com>
* Date:       2020-09-15
*
* Maintainer: Xiao Zhiguo <xiaozhiguo@uniontech.com>
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
*
*****************************************************************************/

#include <QTime>
#include <QDebug>
#include "performancemonitor.h"

const QString LOG_FLAG = "[PerformanceMonitor]";

qint64 PerformanceMonitor::initializeAppStartMs = 0;
qint64 PerformanceMonitor::initializeAppFinishMs = 0;
qint64 PerformanceMonitor::loadFontStartMs = 0;
qint64 PerformanceMonitor::loadFontFinishMs = 0;
qint64 PerformanceMonitor::installFontStartMs = 0;
qint64 PerformanceMonitor::installFontFinishMs = 0;
qint64 PerformanceMonitor::exportFontStartMs = 0;
qint64 PerformanceMonitor::exportFontFinishMs = 0;
qint64 PerformanceMonitor::deleteFontStartMs = 0;
qint64 PerformanceMonitor::deleteFontFinishMs = 0;
qint64 PerformanceMonitor::favoriteFontStartMs = 0;
qint64 PerformanceMonitor::favoriteFontFinishMs = 0;

PerformanceMonitor::PerformanceMonitor(QObject *parent) : QObject(parent)
{
}

/*************************************************************************
 <Function>     initializeMainDialogStart
 <Description>  记录程序初始化开始时间
 <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
 <Maintainer>
 <Input>
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::initializeAppStart()
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << "start to initialize app";
    initializeAppStartMs = current.toMSecsSinceEpoch();
}

/*************************************************************************
 <Function>     initializeAppFinish
 <Description>  记录程序初始化结束时间
 <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
 <Maintainer>
 <Input>
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::initializeAppFinish()
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " finish to initialize app";

    initializeAppFinishMs = current.toMSecsSinceEpoch();
    qint64 time = initializeAppFinishMs - initializeAppStartMs;
    qDebug() << QString("%1 %2-%3 %4 #(Init app time)").arg(GRAB_POINT).arg(APP_NAME).arg(INIT_APP_TIME).arg(time);
}

/*************************************************************************
 <Function>     loadFontStart
 <Description>  记录程序启动时，开始加载字体时间
 <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
 <Maintainer>
 <Input>
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::loadFontStart()
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << "start to load fonts";
    loadFontStartMs = current.toMSecsSinceEpoch();
}

/*************************************************************************
 <Function>     loadFontFinish
 <Description>  记录程序启动时，完成加载字体时间
 <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
 <Maintainer>
 <Input>
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::loadFontFinish()
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " finish to load fonts";

    loadFontFinishMs = current.toMSecsSinceEpoch();
    qint64 time1 = loadFontFinishMs - loadFontStartMs;
    qDebug() << QString("%1 %2-%3 %4 #(Load fonts time)").arg(GRAB_POINT).arg(APP_NAME).arg(LOAD_FONTS_TIME).arg(time1);
    qint64 time2 = loadFontFinishMs - initializeAppStartMs;
    qDebug() << QString("%1 %2-%3 %4 #(Init and load time)").arg(GRAB_POINT).arg(APP_NAME).arg(INIT_AND_LOAD_TIME).arg(time2);
}

/*************************************************************************
 <Function>     installFontStart
 <Description>  记录安装字体的开始时间
 <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
 <Maintainer>
 <Input>
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::installFontStart()
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " start to install fonts";
    installFontStartMs = current.toMSecsSinceEpoch();
}

/*************************************************************************
 <Function>     installFontFinish
 <Description>  记录安装字体的结束时间
 <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
 <Maintainer>   fontCount 安装字体个数
 <Input>
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::installFontFinish(int fontCount)
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " finish to install fonts";

    installFontFinishMs = current.toMSecsSinceEpoch();
    qint64 time = installFontFinishMs - installFontStartMs;
    qDebug() << QString("%1 %2-%3 %4 %5 #(Install fonts time)").arg(GRAB_POINT).arg(APP_NAME).arg(INSTALL_FONTS_TIME).arg(fontCount).arg(time);
}

/*************************************************************************
 <Function>     exportFontStart
 <Description>  记录字体导出的开始时间
 <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
 <Maintainer>
 <Input>
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::exportFontStart()
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " start to export fonts";
    exportFontStartMs = current.toMSecsSinceEpoch();
}

/*************************************************************************
 <Function>     exportFontFinish
 <Description>  记录字体导出的结束时间
 <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
 <Maintainer>   fontCount 导出字体个数
 <Input>
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::exportFontFinish(int fontCount)
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " finish to export fonts";

    exportFontFinishMs = current.toMSecsSinceEpoch();
    qint64 time = exportFontFinishMs - exportFontStartMs;
    qDebug() << QString("%1 %2-%3 %4 %5 #(Export fonts time)").arg(GRAB_POINT).arg(APP_NAME).arg(EXPORT_FONTS_TIME).arg(fontCount).arg(time);
}

/*************************************************************************
 <Function>     deleteFontStart
 <Description>  记录删除字体的开始时间
 <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
 <Maintainer>
 <Input>
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::deleteFontStart()
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " start to delete fonts";
    deleteFontStartMs = current.toMSecsSinceEpoch();
}

/*************************************************************************
 <Function>     deleteFontFinish
 <Description>  记录删除字体的结束时间
 <Author>       Xiao Zhiguo <xiaozhiguo@uniontech.com>
 <Maintainer>
 <Input>        fontCount 删除字体个数
 <Return>
 <Note>
*************************************************************************/
void PerformanceMonitor::deleteFontFinish(int fontCount)
{
    QDateTime current = QDateTime::currentDateTime();
    qDebug() << LOG_FLAG
             << QDateTime::currentDateTime().toString(Qt::ISODateWithMs)
             << " finish to delete fonts";

    deleteFontFinishMs = current.toMSecsSinceEpoch();
    qint64 time = deleteFontFinishMs - deleteFontStartMs;
    qDebug() << QString("%1 %2-%3 %4 %5 #(delete fonts time)").arg(GRAB_POINT).arg(APP_NAME).arg(DELETE_FONTS_TIME).arg(fontCount).arg(time);
}

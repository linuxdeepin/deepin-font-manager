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

#include "globaldef.h"
#include "utils.h"
#include "views/dfontmgrmainwindow.h"
#include "singlefontapplication.h"

#include <QCommandLineParser>
#include <QDebug>

#include <DApplication>
#include <DLog>
#include <DWidgetUtil>
#include <DApplicationSettings>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE


bool checkNewprogress()
{
    QSharedMemory sharedMemory(QString("deepinfontmanagersingle"));
    //if (sharedMemory.isAttached()) {
    //   sharedMemory.detach();
    // }
    //1.申请QBuffer
    QBuffer buffer;
    //2.将buffer写入data流中
    QDataStream out(&buffer);
    //3.buffer读写操作  利用QBuffer将图片数据转化为char * 格式
    buffer.open(QBuffer::ReadWrite);
    //4.将时间写入QDataStream
    QDateTime wstime = QDateTime::currentDateTime();
    QString teststr = wstime.toString("yyyy-MM-dd hh:mm:ss");
    out << teststr;
    //5.定义size  = buffer.size()
    int size = buffer.size();
    bool newflag = true;

    // 创建共享内存段
    if (!sharedMemory.create(size)) {
        // 从共享内存中读取数据
        if (!sharedMemory.isAttached()) //检测程序当前是否关联共享内存
            sharedMemory.attach();
        QBuffer sbuffer;
        QDataStream in(&sbuffer);
        //读取时间
        QDateTime rstime;
        QString tstr;
        sharedMemory.lock();
        sbuffer.setData((char *)sharedMemory.constData(), sharedMemory.size());
        sbuffer.open(QBuffer::ReadOnly);
        in >> tstr;
        rstime = QDateTime::fromString(tstr, "yyyy-MM-dd hh:mm:ss");
        //sharedMemory.unlock();
        //sharedMemory.detach();
        qint64 temptime = rstime.secsTo(wstime);
        if (!rstime.isValid()) return  0;
        if (temptime >= 0 && temptime <= 2 && rstime.isValid()) {
            newflag = false;
        }
        //if (sharedMemory.isAttached()) //检测程序当前是否关联共享内存
        //sharedMemory.attach();
        // sharedMemory.lock();
        char *to = (char *)sharedMemory.data();
        const char *from = buffer.data().data();
        memcpy(to, from, qMin(sharedMemory.size(), size));
        sharedMemory.unlock();
        sharedMemory.detach();
        qDebug() << teststr << "   " << tstr << " error " <<  newflag ;

    } else {
        sharedMemory.lock();
        char *to = (char *)sharedMemory.data();
        const char *from = buffer.data().data();
        memcpy(to, from, qMin(sharedMemory.size(), size));
        sharedMemory.unlock();
    }
    return newflag;
}

int main(int argc, char *argv[])
{
    // load dtk xcb plugin.
    DApplication::loadDXcbPlugin();

    bool isNew = checkNewprogress();
    // init Dtk application's attrubites.
    SingleFontApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.loadTranslator();
    app.setApplicationName("deepin-font-manager");
    app.setOrganizationName("deepin");
    app.setApplicationVersion(DApplication::buildVersion("1.0"));
    app.setApplicationAcknowledgementPage("https://www.deepin.org/original/deepin-font-installer/");
    app.setProductIcon(QIcon::fromTheme(DEEPIN_FONT_MANAGER));
    app.setProductName(DApplication::translate("Main", "Font Manager"));
    app.setApplicationDescription(
        DApplication::translate("Main",
                                "Font Manager helps users install and manage fonts."));

    qputenv("DTK_USE_SEMAPHORE_SINGLEINSTANCE", "1");
    if (!DGuiApplicationHelper::instance()->setSingleInstance(app.applicationName(), DGuiApplicationHelper::UserScope)) {
        return 0;
    }

    DApplicationSettings settings;

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    if (!app.parseCmdLine()) {
        return 0;
    }

    if (isNew == true) {
        app.activateWindow();

        return app.exec();
    } else {

        return 0;
    }

}



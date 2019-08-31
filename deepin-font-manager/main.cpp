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

#include <QCommandLineParser>
#include <QDebug>

#include <DApplication>
#include <DLog>
#include <DWidgetUtil>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    // load dtk xcb plugin.
    DApplication::loadDXcbPlugin();

    // init Dtk application's attrubites.
    DApplication app(argc, argv);
    app.setTheme("light");
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.loadTranslator();
    app.setOrganizationName("deepin");
    app.setApplicationVersion(DApplication::buildVersion("1.0"));
    app.setApplicationAcknowledgementPage("https://www.deepin.org/original/deepin-font-installer/");
    app.setProductIcon(QIcon(":/images/deepin-font-manager.svg"));
    app.setProductName(DApplication::translate("Main", "Deepin Font Manager"));
    // app.setStyleSheet(Utils::getQssContent(":/qss/style.qss"));
    app.setApplicationDescription(
        DApplication::translate("Main",
                                "Deepin Font Manager is used to install and uninstall font file "
                                "for users with bulk install function."));

    // temp skin change
    app.setStyle("chameleon");

    if (!app.setSingleInstance("deepin-font-manager")) {
        qDebug() << "Deepin Font Manager is Single instance only now!";
        return 0;
    }

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    // add command line parser to app.
    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin Font Manager.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("filename", "Font file path.", "file [file..]");
    parser.process(app);

    const QStringList fileList = parser.positionalArguments();

    // init modules.
    QScopedPointer<DFontMgrMainWindow> pqsMainWnd(new DFontMgrMainWindow());

    // handle command line parser.
    // Double click font file start quick install UI

    pqsMainWnd->setMinimumSize(DEFAULT_WINDOWS_WIDTH, DEFAULT_WINDOWS_HEIGHT);
    pqsMainWnd->setWindowIcon(QIcon(":/images/deepin-font-manager.svg"));
    pqsMainWnd->show();

    Dtk::Widget::moveToCenter(pqsMainWnd.get());

    if (fileList.size() > 0) {
        qDebug() << "File:" << fileList << " to quick install.";

        pqsMainWnd->setVisible(false);
        pqsMainWnd->setQuickInstallMode(true);
        QMetaObject::invokeMethod(pqsMainWnd.get(), "quickModeInstall", Qt::QueuedConnection,
                                  Q_ARG(QStringList, fileList));
    }

    return app.exec();
}

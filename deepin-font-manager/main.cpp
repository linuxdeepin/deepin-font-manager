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

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    // load dtk xcb plugin.
    DApplication::loadDXcbPlugin();

    // init Dtk application's attrubites.
    SingleFontApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.loadTranslator();
    app.setApplicationName("deepin-font-manager");
    app.setOrganizationName("deepin");
    app.setApplicationVersion(DApplication::buildVersion("1.0"));
    app.setApplicationAcknowledgementPage("https://www.deepin.org/original/deepin-font-installer/");
    app.setProductIcon(QIcon(":/images/deepin-font-manager.svg"));
    app.setProductName(DApplication::translate("Main", "Deepin Font Manager"));
    app.setApplicationDescription(
        DApplication::translate("Main",
                                "Deepin Font Manager is used to install and uninstall font file "
                                "for users with bulk install function."));

    DApplicationSettings settings;

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    if(!app.isRunning()) {
        app.setMainWindow(new DFontMgrMainWindow());

        app.activateWindow();

        return app.exec();
    }

    return 0;
}
